#pragma once
#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "GuarantorRegistrationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class GuarantorEditRecord
    {
    public:
        property int GuarantorID;
        property String^ GuarantorCode;
        property int ApplicantID;
        property String^ FullName;
        property String^ NIC;
        property String^ Phone;
        property String^ Email;
        property String^ Address;
        property String^ RelationshipToApplicant;
        property String^ Occupation;
        property Decimal MonthlyIncome;
    };

    public ref class GuarantorEditService abstract sealed
    {
    private:
        static String^ Clean(String^ s)
        {
            return s == nullptr ? "" : s->Trim();
        }

        static String^ ReadText(
            SqlDataReader^ r, int index)
        {
            return r->IsDBNull(index)
                ? "" : r->GetString(index);
        }

        static Object^ OptionalText(String^ s)
        {
            return String::IsNullOrWhiteSpace(s)
                ? (Object^)DBNull::Value : (Object^)s;
        }

        static void RequireUser(
            SqlConnection^ con,
            SqlTransaction^ tx,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr || actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND Username=@username "
                "AND IsActive=1 AND RoleID IN (1,2,3)",
                con, tx);

            cmd->Parameters->Add(
                "@id", SqlDbType::Int)->Value = actor->UserID;

            cmd->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256
            )->Value = actor->Username;

            if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
                throw gcnew UnauthorizedAccessException(
                    "Access denied.");
        }

        static void CheckLength(
            String^ s, int limit, String^ name)
        {
            if (s->Length > limit)
                throw gcnew ArgumentException(
                    name + " exceeds " +
                    limit.ToString() + " characters.");
        }

    public:
        static GuarantorEditRecord^ GetGuarantor(
            AuthenticatedUser^ actor, int guarantorID)
        {
            if (guarantorID <= 0)
                throw gcnew ArgumentException(
                    "Invalid guarantor ID.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT GuarantorID,GuarantorCode,"
                    "ApplicantID,FullName,NIC,Phone,Email,"
                    "Address,RelationshipToApplicant,"
                    "Occupation,MonthlyIncome "
                    "FROM dbo.Guarantors "
                    "WHERE GuarantorID=@id", con);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = guarantorID;

                SqlDataReader^ reader = cmd->ExecuteReader();

                try
                {
                    if (!reader->Read())
                        throw gcnew InvalidOperationException(
                            "Guarantor record not found.");

                    GuarantorEditRecord^ r =
                        gcnew GuarantorEditRecord();

                    r->GuarantorID = reader->GetInt32(0);
                    r->GuarantorCode = ReadText(reader, 1);
                    r->ApplicantID = reader->GetInt32(2);
                    r->FullName = ReadText(reader, 3);
                    r->NIC = ReadText(reader, 4);
                    r->Phone = ReadText(reader, 5);
                    r->Email = ReadText(reader, 6);
                    r->Address = ReadText(reader, 7);
                    r->RelationshipToApplicant =
                        ReadText(reader, 8);
                    r->Occupation = ReadText(reader, 9);
                    r->MonthlyIncome = reader->GetDecimal(10);

                    return r;
                }
                finally
                {
                    delete reader;
                }
            }
            finally
            {
                delete con;
            }
        }

        static void Update(
            AuthenticatedUser^ actor,
            GuarantorEditRecord^ r)
        {
            if (r == nullptr || r->GuarantorID <= 0)
                throw gcnew ArgumentException(
                    "Invalid guarantor record.");

            r->FullName = Clean(r->FullName);
            r->NIC = Clean(r->NIC);
            r->Phone = Clean(r->Phone);
            r->Email = Clean(r->Email);
            r->Address = Clean(r->Address);
            r->RelationshipToApplicant =
                Clean(r->RelationshipToApplicant);
            r->Occupation = Clean(r->Occupation);

            if (r->ApplicantID <= 0)
                throw gcnew ArgumentException(
                    "Select an applicant.");

            if (String::IsNullOrWhiteSpace(r->FullName) ||
                String::IsNullOrWhiteSpace(r->Phone) ||
                String::IsNullOrWhiteSpace(
                    r->RelationshipToApplicant))
                throw gcnew ArgumentException(
                    "Name, phone and relationship are required.");

            CheckLength(r->FullName, 150, "Full name");
            CheckLength(r->NIC, 20, "NIC");
            CheckLength(r->Phone, 25, "Phone");
            CheckLength(r->Email, 150, "Email");
            CheckLength(r->Address, 300, "Address");
            CheckLength(r->RelationshipToApplicant,
                100, "Relationship");
            CheckLength(r->Occupation, 100, "Occupation");

            if (r->Email->Length > 0 &&
                (r->Email->IndexOf("@") <= 0 ||
                 r->Email->LastIndexOf(".") <=
                    r->Email->IndexOf("@") + 1 ||
                 r->Email->EndsWith(".")))
                throw gcnew ArgumentException(
                    "Enter a valid email address.");

            if (r->MonthlyIncome < Decimal::Zero ||
                r->MonthlyIncome >
                    Decimal(9999999999999999LL))
                throw gcnew ArgumentException(
                    "Monthly income is outside the allowed range.");

            r->MonthlyIncome = Decimal::Round(
                r->MonthlyIncome, 2,
                MidpointRounding::AwayFromZero);

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();
                tx = con->BeginTransaction();

                RequireUser(con, tx, actor);

                SqlCommand^ applicantCheck =
                    gcnew SqlCommand(
                        "SELECT COUNT(*) FROM dbo.Applicants "
                        "WHERE ApplicantID=@id",
                        con, tx);

                applicantCheck->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = r->ApplicantID;

                if (Convert::ToInt32(
                    applicantCheck->ExecuteScalar()) != 1)
                    throw gcnew ArgumentException(
                        "Selected applicant does not exist.");

                SqlCommand^ cmd = gcnew SqlCommand(
                    "UPDATE dbo.Guarantors SET "
                    "ApplicantID=@applicant,"
                    "FullName=@name,NIC=@nic,Phone=@phone,"
                    "Email=@email,Address=@address,"
                    "RelationshipToApplicant=@relationship,"
                    "Occupation=@occupation,"
                    "MonthlyIncome=@income "
                    "WHERE GuarantorID=@id",
                    con, tx);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = r->GuarantorID;

                cmd->Parameters->Add(
                    "@applicant", SqlDbType::Int
                )->Value = r->ApplicantID;

                cmd->Parameters->Add(
                    "@name", SqlDbType::NVarChar, 150
                )->Value = r->FullName;

                cmd->Parameters->Add(
                    "@nic", SqlDbType::NVarChar, 20
                )->Value = OptionalText(r->NIC);

                cmd->Parameters->Add(
                    "@phone", SqlDbType::NVarChar, 25
                )->Value = r->Phone;

                cmd->Parameters->Add(
                    "@email", SqlDbType::NVarChar, 150
                )->Value = OptionalText(r->Email);

                cmd->Parameters->Add(
                    "@address", SqlDbType::NVarChar, 300
                )->Value = OptionalText(r->Address);

                cmd->Parameters->Add(
                    "@relationship", SqlDbType::NVarChar, 100
                )->Value = r->RelationshipToApplicant;

                cmd->Parameters->Add(
                    "@occupation", SqlDbType::NVarChar, 100
                )->Value = OptionalText(r->Occupation);

                SqlParameter^ income = cmd->Parameters->Add(
                    "@income", SqlDbType::Decimal);
                income->Precision = 18;
                income->Scale = 2;
                income->Value = r->MonthlyIncome;

                int affected = cmd->ExecuteNonQuery();

                if (affected != 1)
                    throw gcnew InvalidOperationException(
                        "Guarantor was not found or was changed.");

                tx->Commit();
            }
            catch (Exception^)
            {
                if (tx != nullptr)
                {
                    try { tx->Rollback(); }
                    catch (Exception^) {}
                }
                throw;
            }
            finally
            {
                delete con;
            }
        }
    };
}
