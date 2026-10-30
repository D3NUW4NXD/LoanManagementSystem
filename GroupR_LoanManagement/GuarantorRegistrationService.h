#pragma once
#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class GuarantorRegistrationService abstract sealed
    {
    private:
        static void RequireActiveUser(
            SqlConnection^ con,
            SqlTransaction^ tx,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr || actor->UserID <= 0)
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND IsActive=1 "
                "AND RoleID IN (1,2,3)",
                con, tx);

            cmd->Parameters->Add("@id", SqlDbType::Int)->Value =
                actor->UserID;

            if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
                throw gcnew UnauthorizedAccessException(
                    "Your account is not authorized.");
        }

        static void CheckLength(
            String^ value, int limit, String^ field)
        {
            if (value != nullptr && value->Length > limit)
                throw gcnew ArgumentException(
                    field + " exceeds " + limit.ToString() +
                    " characters.");
        }

        static String^ Clean(String^ value)
        {
            return value == nullptr ? "" : value->Trim();
        }

        static Object^ OptionalText(String^ value)
        {
            return String::IsNullOrWhiteSpace(value)
                ? (Object^)DBNull::Value
                : (Object^)value;
        }

    public:
        static DataTable^ GetApplicants(
            AuthenticatedUser^ actor)
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireActiveUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT ApplicantID, "
                    "ApplicantCode + ' - ' + FullName AS DisplayName "
                    "FROM dbo.Applicants ORDER BY FullName",
                    con);

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                DataTable^ table = gcnew DataTable();
                adapter->Fill(table);
                return table;
            }
            finally
            {
                delete con;
            }
        }

        static int Register(
            AuthenticatedUser^ actor,
            int applicantID,
            String^ fullName,
            String^ nic,
            String^ phone,
            String^ email,
            String^ address,
            String^ relationship,
            String^ occupation,
            Decimal monthlyIncome)
        {
            fullName = Clean(fullName);
            nic = Clean(nic);
            phone = Clean(phone);
            email = Clean(email);
            address = Clean(address);
            relationship = Clean(relationship);
            occupation = Clean(occupation);

            if (applicantID <= 0)
                throw gcnew ArgumentException(
                    "Select an applicant.");

            if (String::IsNullOrWhiteSpace(fullName))
                throw gcnew ArgumentException(
                    "Guarantor full name is required.");

            if (String::IsNullOrWhiteSpace(phone))
                throw gcnew ArgumentException(
                    "Phone number is required.");

            if (String::IsNullOrWhiteSpace(relationship))
                throw gcnew ArgumentException(
                    "Relationship to applicant is required.");

            CheckLength(fullName, 150, "Full name");
            CheckLength(nic, 20, "NIC");
            CheckLength(phone, 25, "Phone");
            CheckLength(email, 150, "Email");
            CheckLength(address, 300, "Address");
            CheckLength(relationship, 100, "Relationship");
            CheckLength(occupation, 100, "Occupation");

            if (email->Length > 0 &&
                (email->IndexOf("@") <= 0 ||
                 email->LastIndexOf(".") <= email->IndexOf("@") + 1 ||
                 email->EndsWith(".")))
                throw gcnew ArgumentException(
                    "Enter a valid email address.");

            if (monthlyIncome < Decimal::Zero ||
                monthlyIncome > Decimal(9999999999999999LL))
                throw gcnew ArgumentException(
                    "Monthly income must be between 0 and "
                    "9,999,999,999,999,999.00.");

            monthlyIncome = Decimal::Round(
                monthlyIncome, 2,
                MidpointRounding::AwayFromZero);

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();
                tx = con->BeginTransaction();

                RequireActiveUser(con, tx, actor);

                SqlCommand^ check = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Applicants "
                    "WHERE ApplicantID=@applicant",
                    con, tx);

                check->Parameters->Add(
                    "@applicant", SqlDbType::Int
                )->Value = applicantID;

                if (Convert::ToInt32(check->ExecuteScalar()) != 1)
                    throw gcnew ArgumentException(
                        "The selected applicant no longer exists.");

                String^ code = "GUA-" +
                    Guid::NewGuid().ToString("N")->
                    Substring(0, 12)->ToUpperInvariant();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "INSERT INTO dbo.Guarantors "
                    "(GuarantorCode,ApplicantID,FullName,NIC,"
                    "Phone,Email,Address,RelationshipToApplicant,"
                    "Occupation,MonthlyIncome,CreatedByUserID) "
                    "OUTPUT INSERTED.GuarantorID "
                    "VALUES (@code,@applicant,@name,@nic,"
                    "@phone,@email,@address,@relationship,"
                    "@occupation,@income,@creator)",
                    con, tx);

                cmd->Parameters->Add(
                    "@code", SqlDbType::NVarChar, 20
                )->Value = code;

                cmd->Parameters->Add(
                    "@applicant", SqlDbType::Int
                )->Value = applicantID;

                cmd->Parameters->Add(
                    "@name", SqlDbType::NVarChar, 150
                )->Value = fullName;

                cmd->Parameters->Add(
                    "@nic", SqlDbType::NVarChar, 20
                )->Value = OptionalText(nic);

                cmd->Parameters->Add(
                    "@phone", SqlDbType::NVarChar, 25
                )->Value = phone;

                cmd->Parameters->Add(
                    "@email", SqlDbType::NVarChar, 150
                )->Value = OptionalText(email);

                cmd->Parameters->Add(
                    "@address", SqlDbType::NVarChar, 300
                )->Value = OptionalText(address);

                cmd->Parameters->Add(
                    "@relationship", SqlDbType::NVarChar, 100
                )->Value = relationship;

                cmd->Parameters->Add(
                    "@occupation", SqlDbType::NVarChar, 100
                )->Value = OptionalText(occupation);

                SqlParameter^ income =
                    cmd->Parameters->Add(
                        "@income", SqlDbType::Decimal);
                income->Precision = 18;
                income->Scale = 2;
                income->Value = monthlyIncome;

                cmd->Parameters->Add(
                    "@creator", SqlDbType::Int
                )->Value = actor->UserID;

                int id = Convert::ToInt32(
                    cmd->ExecuteScalar());

                tx->Commit();
                return id;
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
