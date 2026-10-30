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
    public ref class LoanRegistrationService abstract sealed
    {
    private:
        static void ValidateUser(
            SqlConnection^ con,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please log in again.");

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id "
                "AND Username=@username "
                "AND IsActive=1 "
                "AND RoleID IN (1,2,3)",
                con);

            cmd->Parameters->Add(
                "@id", SqlDbType::Int
            )->Value = actor->UserID;

            cmd->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256
            )->Value = actor->Username;

            if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
                throw gcnew UnauthorizedAccessException(
                    "Your account is not authorized.");
        }

        static void CheckFieldLength(
            SqlConnection^ con,
            String^ column,
            String^ value)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT max_length "
                "FROM sys.columns "
                "WHERE object_id=OBJECT_ID('dbo.Loans') "
                "AND name=@name",
                con);

            cmd->Parameters->Add(
                "@name", SqlDbType::NVarChar, 128
            )->Value = column;

            Object^ result = cmd->ExecuteScalar();

            if (result == nullptr || result == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Missing database column: " + column);

            int bytes = Convert::ToInt32(result);

            // NVARCHAR stores UTF-16 code units, generally two bytes each.
            if (bytes != -1 && value->Length > bytes / 2)
                throw gcnew ArgumentException(
                    column + " exceeds the database field length.");
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
                ValidateUser(con, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT ApplicantID, ApplicantCode, "
                    "FullName, "
                    "ApplicantCode + N' - ' + FullName "
                    "AS DisplayName "
                    "FROM dbo.Applicants "
                    "ORDER BY FullName",
                    con);

                DataTable^ table = gcnew DataTable();

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                try
                {
                    adapter->Fill(table);
                }
                finally
                {
                    delete adapter;
                }

                return table;
            }
            finally
            {
                delete con;
            }
        }

        static int RegisterLoan(
            AuthenticatedUser^ actor,
            int applicantID,
            String^ loanType,
            String^ loanPurpose,
            Decimal requestedAmount,
            Decimal annualInterestRate,
            int termMonths)
        {
            if (applicantID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid applicant.");

            loanType = loanType == nullptr
                ? "" : loanType->Trim();

            loanPurpose = loanPurpose == nullptr
                ? "" : loanPurpose->Trim();

            if (String::IsNullOrWhiteSpace(loanType))
                throw gcnew ArgumentException(
                    "Loan type is required.");

            if (String::IsNullOrWhiteSpace(loanPurpose))
                throw gcnew ArgumentException(
                    "Loan purpose is required.");

            if (Decimal::Compare(requestedAmount, Decimal::Zero) <= 0)
                throw gcnew ArgumentException(
                    "Requested amount must be greater than zero.");

            if (Decimal::Compare(annualInterestRate, Decimal::Zero) < 0 ||
                Decimal::Compare(annualInterestRate, Decimal(100)) > 0)
                throw gcnew ArgumentException(
                    "Interest rate must be between 0 and 100.");

            if (termMonths < 1 || termMonths > 360)
                throw gcnew ArgumentException(
                    "Loan term must be between 1 and 360 months.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                ValidateUser(con, actor);

                SqlCommand^ applicantCheck = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Applicants "
                    "WHERE ApplicantID=@id",
                    con);

                applicantCheck->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = applicantID;

                if (Convert::ToInt32(
                    applicantCheck->ExecuteScalar()) != 1)
                    throw gcnew InvalidOperationException(
                        "The selected applicant no longer exists.");

                Guid token = Guid::NewGuid();
                String^ loanCode =
                    "LN-" + token.ToString("N")->Substring(0, 14);

                CheckFieldLength(con, "LoanCode", loanCode);
                CheckFieldLength(con, "LoanType", loanType);
                CheckFieldLength(con, "LoanPurpose", loanPurpose);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "INSERT INTO dbo.Loans "
                    "(LoanCode,ApplicantID,LoanType,LoanPurpose,"
                    "RequestedAmount,AnnualInterestRate,"
                    "TermMonths,CreatedByUserID) "
                    "OUTPUT INSERTED.LoanID "
                    "VALUES (@code,@applicant,@type,@purpose,"
                    "@amount,@rate,@term,@creator)",
                    con);

                cmd->Parameters->Add(
                    "@code", SqlDbType::NVarChar, 20
                )->Value = loanCode;

                cmd->Parameters->Add(
                    "@applicant", SqlDbType::Int
                )->Value = applicantID;

                cmd->Parameters->Add(
                    "@type", SqlDbType::NVarChar, -1
                )->Value = loanType;

                cmd->Parameters->Add(
                    "@purpose", SqlDbType::NVarChar, -1
                )->Value = loanPurpose;

                SqlParameter^ amountParam =
                    cmd->Parameters->Add(
                        "@amount", SqlDbType::Decimal);

                amountParam->Precision = 18;
                amountParam->Scale = 2;
                amountParam->Value = requestedAmount;

                SqlParameter^ rateParam =
                    cmd->Parameters->Add(
                        "@rate", SqlDbType::Decimal);

                rateParam->Precision = 10;
                rateParam->Scale = 2;
                rateParam->Value = annualInterestRate;

                cmd->Parameters->Add(
                    "@term", SqlDbType::Int
                )->Value = termMonths;

                cmd->Parameters->Add(
                    "@creator", SqlDbType::Int
                )->Value = actor->UserID;

                Object^ result = cmd->ExecuteScalar();

                if (result == nullptr || result == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Loan registration did not return a LoanID.");

                return Convert::ToInt32(result);
            }
            finally
            {
                delete con;
            }
        }
    };
}
