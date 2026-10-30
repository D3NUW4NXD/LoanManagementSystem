#pragma once
#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class ApplicantRegistrationService abstract sealed
    {
    private:
        static void OptionalText(SqlCommand^ cmd,
            String^ key, int size, String^ value)
        {
            SqlParameter^ p =
                cmd->Parameters->Add(key, SqlDbType::NVarChar, size);
            p->Value = String::IsNullOrWhiteSpace(value)
                ? (Object^)DBNull::Value : (Object^)value->Trim();
        }

    public:
        static int Register(
            AuthenticatedUser^ actor,
            String^ fullName, String^ nic,
            Nullable<DateTime> dateOfBirth,
            String^ phone, String^ email,
            String^ address, String^ occupation,
            String^ employer, Decimal income,
            Decimal debt, Nullable<int> creditScore)
        {
            if (actor == nullptr || actor->UserID <= 0)
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            fullName = fullName == nullptr ?
                "" : fullName->Trim();
            phone = phone == nullptr ?
                "" : phone->Trim();

            if (String::IsNullOrWhiteSpace(fullName) ||
                fullName->Length > 150)
                throw gcnew ArgumentException(
                    "Full name is required (maximum 150 characters).");

            if (String::IsNullOrWhiteSpace(phone) ||
                phone->Length > 25)
                throw gcnew ArgumentException(
                    "Phone is required (maximum 25 characters).");

            int digits = 0;
            for each (Char ch in phone)
            {
                if (Char::IsDigit(ch)) ++digits;
                else if (!(ch == '+' || ch == '-' ||
                    ch == '(' || ch == ')' || ch == ' '))
                    throw gcnew ArgumentException(
                        "Invalid phone number characters.");
            }

            if (digits < 9 || digits > 15)
                throw gcnew ArgumentException(
                    "Phone must contain 9 to 15 digits.");

            if (income <= Decimal::Zero)
                throw gcnew ArgumentException(
                    "Monthly income must be greater than zero.");

            if (debt < Decimal::Zero)
                throw gcnew ArgumentException(
                    "Existing monthly debt cannot be negative.");

            if (creditScore.HasValue &&
                creditScore.Value < 0)
                throw gcnew ArgumentException(
                    "Credit score cannot be negative.");

            if (dateOfBirth.HasValue &&
                dateOfBirth.Value.Date > DateTime::Today)
                throw gcnew ArgumentException(
                    "Date of birth cannot be in the future.");

            if (!String::IsNullOrWhiteSpace(email))
            {
                try {
                    System::Net::Mail::MailAddress^ parsed =
                        gcnew System::Net::Mail::MailAddress(email->Trim());
                    if (!String::Equals(parsed->Address,
                        email->Trim(),
                        StringComparison::OrdinalIgnoreCase))
                        throw gcnew FormatException();
                }
                catch (FormatException^) {
                    throw gcnew ArgumentException(
                        "Enter a valid email address.");
                }
            }

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ transaction = nullptr;

            try
            {
                connection->Open();
                transaction = connection->BeginTransaction();

                SqlCommand^ auth = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@ID AND Username=@Username "
                    "AND IsActive=1 AND RoleID IN (1,2,3)",
                    connection, transaction);

                auth->Parameters->Add(
                    "@ID", SqlDbType::Int)->Value = actor->UserID;
                auth->Parameters->Add(
                    "@Username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                bool authorized =
                    Convert::ToInt32(auth->ExecuteScalar()) == 1;
                delete auth;

                if (!authorized)
                    throw gcnew UnauthorizedAccessException(
                        "Your account cannot register applicants.");

                String^ code = "APP-" +
                    Guid::NewGuid().ToString("N")
                    ->Substring(0, 12)->ToUpperInvariant();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "INSERT INTO dbo.Applicants "
                    "(ApplicantCode, FullName, NIC, "
                    "DateOfBirth, Phone, Email, Address, "
                    "Occupation, Employer, MonthlyIncome, "
                    "ExistingMonthlyDebt, CreditScore, "
                    "CreatedByUserID) "
                    "OUTPUT INSERTED.ApplicantID "
                    "VALUES (@Code,@Name,@NIC,@DOB,@Phone,"
                    "@Email,@Address,@Occupation,@Employer,"
                    "@Income,@Debt,@Score,@UserID)",
                    connection, transaction);

                cmd->Parameters->Add(
                    "@Code", SqlDbType::NVarChar, 20)->Value = code;
                cmd->Parameters->Add(
                    "@Name", SqlDbType::NVarChar, 150)->Value = fullName;
                cmd->Parameters->Add(
                    "@Phone", SqlDbType::NVarChar, 25)->Value = phone;
                cmd->Parameters->Add(
                    "@UserID", SqlDbType::Int)->Value = actor->UserID;

                OptionalText(cmd, "@NIC", 20, nic);
                OptionalText(cmd, "@Email", 150, email);
                OptionalText(cmd, "@Address", 300, address);
                OptionalText(cmd, "@Occupation", 100, occupation);
                OptionalText(cmd, "@Employer", 150, employer);

                SqlParameter^ dob =
                    cmd->Parameters->Add("@DOB", SqlDbType::Date);
                dob->Value = dateOfBirth.HasValue
                    ? (Object^)dateOfBirth.Value.Date
                    : (Object^)DBNull::Value;

                SqlParameter^ amount =
                    cmd->Parameters->Add(
                        "@Income", SqlDbType::Decimal);
                amount->Precision = 18;
                amount->Scale = 2;
                amount->Value = income;

                SqlParameter^ monthlyDebt =
                    cmd->Parameters->Add(
                        "@Debt", SqlDbType::Decimal);
                monthlyDebt->Precision = 18;
                monthlyDebt->Scale = 2;
                monthlyDebt->Value = debt;

                SqlParameter^ score =
                    cmd->Parameters->Add(
                        "@Score", SqlDbType::Int);
                score->Value = creditScore.HasValue
                    ? (Object^)creditScore.Value
                    : (Object^)DBNull::Value;

                int id = Convert::ToInt32(
                    cmd->ExecuteScalar());

                delete cmd;
                transaction->Commit();
                return id;
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
