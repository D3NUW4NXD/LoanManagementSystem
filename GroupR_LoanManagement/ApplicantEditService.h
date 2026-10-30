#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class ApplicantEditRecord
    {
    public:
        property int ApplicantID;
        property String^ ApplicantCode;
        property String^ FullName;
        property String^ NIC;
        property Nullable<DateTime> DateOfBirth;
        property String^ Phone;
        property String^ Email;
        property String^ Address;
        property String^ Occupation;
        property String^ Employer;
        property Decimal MonthlyIncome;
        property Decimal ExistingMonthlyDebt;
        property Nullable<int> CreditScore;
    };

    public ref class ApplicantEditService abstract sealed
    {
    private:
        static void Authorize(
            SqlConnection^ connection,
            SqlTransaction^ transaction,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr || actor->UserID <= 0)
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            SqlCommand^ command = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@UserID AND Username=@Username "
                "AND IsActive=1 AND RoleID IN (1,2,3)",
                connection, transaction);

            command->Parameters->Add(
                "@UserID", SqlDbType::Int)->Value =
                actor->UserID;

            command->Parameters->Add(
                "@Username", SqlDbType::NVarChar, 256
            )->Value = actor->Username;

            bool allowed =
                Convert::ToInt32(command->ExecuteScalar()) == 1;

            delete command;

            if (!allowed)
                throw gcnew UnauthorizedAccessException(
                    "You are not authorized to edit applicants.");
        }

        static String^ ReadText(
            SqlDataReader^ reader, String^ column)
        {
            Object^ value = reader[column];

            return value == DBNull::Value
                ? String::Empty
                : Convert::ToString(value);
        }

        static void AddOptional(
            SqlCommand^ command,
            String^ parameter,
            String^ value,
            int maximum)
        {
            if (value != nullptr)
                value = value->Trim();

            if (value != nullptr && value->Length > maximum)
                throw gcnew ArgumentException(
                    parameter + " is too long.");

            SqlParameter^ p =
                command->Parameters->Add(
                    parameter, SqlDbType::NVarChar, maximum);

            p->Value = String::IsNullOrWhiteSpace(value)
                ? (Object^)DBNull::Value
                : (Object^)value;
        }

        static void Validate(ApplicantEditRecord^ record)
        {
            if (record == nullptr || record->ApplicantID <= 0)
                throw gcnew ArgumentException(
                    "Invalid applicant record.");

            if (String::IsNullOrWhiteSpace(record->FullName) ||
                record->FullName->Trim()->Length > 150)
                throw gcnew ArgumentException(
                    "Full name is required (max 150 characters).");

            if (String::IsNullOrWhiteSpace(record->Phone) ||
                record->Phone->Trim()->Length > 25)
                throw gcnew ArgumentException(
                    "Phone is required (max 25 characters).");

            int digits = 0;
            for each (Char ch in record->Phone)
            {
                if (Char::IsDigit(ch))
                    ++digits;
                else if (!(ch == '+' || ch == '-' ||
                    ch == '(' || ch == ')' || ch == ' '))
                    throw gcnew ArgumentException(
                        "Invalid phone number characters.");
            }

            if (digits < 9 || digits > 15)
                throw gcnew ArgumentException(
                    "Phone must contain 9 to 15 digits.");

            if (record->MonthlyIncome <= Decimal::Zero)
                throw gcnew ArgumentException(
                    "Monthly income must be greater than zero.");

            if (record->ExistingMonthlyDebt < Decimal::Zero)
                throw gcnew ArgumentException(
                    "Existing monthly debt cannot be negative.");

            if (record->CreditScore.HasValue &&
                record->CreditScore.Value < 0)
                throw gcnew ArgumentException(
                    "Credit score cannot be negative.");

            if (record->DateOfBirth.HasValue &&
                record->DateOfBirth.Value.Date > DateTime::Today)
                throw gcnew ArgumentException(
                    "Date of birth cannot be in the future.");

            if (!String::IsNullOrWhiteSpace(record->Email))
            {
                String^ email = record->Email->Trim();
                try
                {
                    System::Net::Mail::MailAddress^ address =
                        gcnew System::Net::Mail::MailAddress(email);

                    if (!String::Equals(address->Address, email,
                        StringComparison::OrdinalIgnoreCase))
                        throw gcnew FormatException();
                }
                catch (FormatException^)
                {
                    throw gcnew ArgumentException(
                        "Enter a valid email address.");
                }
            }
        }

    public:
        static ApplicantEditRecord^ GetApplicant(
            AuthenticatedUser^ actor,
            int applicantID)
        {
            if (applicantID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid applicant.");

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();
                Authorize(connection, nullptr, actor);

                SqlCommand^ command = gcnew SqlCommand(
                    "SELECT ApplicantID, ApplicantCode, "
                    "FullName, NIC, DateOfBirth, Phone, "
                    "Email, Address, Occupation, Employer, "
                    "MonthlyIncome, ExistingMonthlyDebt, "
                    "CreditScore FROM dbo.Applicants "
                    "WHERE ApplicantID=@ApplicantID",
                    connection);

                command->Parameters->Add(
                    "@ApplicantID", SqlDbType::Int
                )->Value = applicantID;

                SqlDataReader^ reader =
                    command->ExecuteReader();

                try
                {
                    if (!reader->Read())
                        throw gcnew InvalidOperationException(
                            "Applicant record not found.");

                    ApplicantEditRecord^ record =
                        gcnew ApplicantEditRecord();

                    record->ApplicantID =
                        Convert::ToInt32(reader["ApplicantID"]);

                    record->ApplicantCode =
                        ReadText(reader, "ApplicantCode");

                    record->FullName =
                        ReadText(reader, "FullName");

                    record->NIC = ReadText(reader, "NIC");
                    record->Phone = ReadText(reader, "Phone");
                    record->Email = ReadText(reader, "Email");
                    record->Address = ReadText(reader, "Address");
                    record->Occupation =
                        ReadText(reader, "Occupation");
                    record->Employer =
                        ReadText(reader, "Employer");

                    record->MonthlyIncome =
                        Convert::ToDecimal(reader["MonthlyIncome"]);

                    record->ExistingMonthlyDebt =
                        Convert::ToDecimal(
                            reader["ExistingMonthlyDebt"]);

                    if (reader["DateOfBirth"] != DBNull::Value)
                        record->DateOfBirth = Nullable<DateTime>(
                            Convert::ToDateTime(reader["DateOfBirth"]));

                    if (reader["CreditScore"] != DBNull::Value)
                        record->CreditScore = Nullable<int>(
                            Convert::ToInt32(reader["CreditScore"]));

                    return record;
                }
                finally
                {
                    delete reader;
                    delete command;
                }
            }
            finally
            {
                delete connection;
            }
        }

        static void UpdateApplicant(
            AuthenticatedUser^ actor,
            ApplicantEditRecord^ record)
        {
            Validate(record);

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ transaction = nullptr;

            try
            {
                connection->Open();
                transaction = connection->BeginTransaction();

                Authorize(connection, transaction, actor);

                SqlCommand^ command = gcnew SqlCommand(
                    "UPDATE dbo.Applicants SET "
                    "FullName=@FullName, NIC=@NIC, "
                    "DateOfBirth=@DOB, Phone=@Phone, "
                    "Email=@Email, Address=@Address, "
                    "Occupation=@Occupation, Employer=@Employer, "
                    "MonthlyIncome=@Income, "
                    "ExistingMonthlyDebt=@Debt, "
                    "CreditScore=@Score "
                    "WHERE ApplicantID=@ApplicantID",
                    connection, transaction);

                command->Parameters->Add(
                    "@ApplicantID", SqlDbType::Int
                )->Value = record->ApplicantID;

                command->Parameters->Add(
                    "@FullName", SqlDbType::NVarChar, 150
                )->Value = record->FullName->Trim();

                command->Parameters->Add(
                    "@Phone", SqlDbType::NVarChar, 25
                )->Value = record->Phone->Trim();

                AddOptional(command, "@NIC", record->NIC, 20);
                AddOptional(command, "@Email", record->Email, 150);
                AddOptional(command, "@Address", record->Address, 300);
                AddOptional(command, "@Occupation",
                    record->Occupation, 100);
                AddOptional(command, "@Employer",
                    record->Employer, 150);

                SqlParameter^ dob = command->Parameters->Add(
                    "@DOB", SqlDbType::Date);

                dob->Value = record->DateOfBirth.HasValue
                    ? (Object^)record->DateOfBirth.Value.Date
                    : (Object^)DBNull::Value;

                SqlParameter^ income = command->Parameters->Add(
                    "@Income", SqlDbType::Decimal);
                income->Precision = 18;
                income->Scale = 2;
                income->Value = record->MonthlyIncome;

                SqlParameter^ debt = command->Parameters->Add(
                    "@Debt", SqlDbType::Decimal);
                debt->Precision = 18;
                debt->Scale = 2;
                debt->Value = record->ExistingMonthlyDebt;

                SqlParameter^ score = command->Parameters->Add(
                    "@Score", SqlDbType::Int);

                score->Value = record->CreditScore.HasValue
                    ? (Object^)record->CreditScore.Value
                    : (Object^)DBNull::Value;

                int affected = command->ExecuteNonQuery();
                delete command;

                if (affected != 1)
                    throw gcnew InvalidOperationException(
                        "Applicant no longer exists. Nothing was saved.");

                transaction->Commit();
            }
            catch (Exception^)
            {
                if (transaction != nullptr)
                {
                    try { transaction->Rollback(); }
                    catch (Exception^) {}
                }
                throw;
            }
            finally
            {
                delete connection;
            }
        }
    };
}
