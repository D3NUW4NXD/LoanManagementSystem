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
    public ref class LoanEditService abstract sealed
    {
    private:
        static void RequireUser(
            SqlConnection^ con,
            SqlTransaction^ tx,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please log in again.");

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND Username=@username "
                "AND IsActive=1 AND RoleID IN (1,2,3)",
                con, tx);

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

        static void CheckLength(
            SqlConnection^ con,
            SqlTransaction^ tx,
            String^ column,
            String^ value)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT max_length FROM sys.columns "
                "WHERE object_id=OBJECT_ID('dbo.Loans') "
                "AND name=@column",
                con, tx);

            cmd->Parameters->Add(
                "@column", SqlDbType::NVarChar, 128
            )->Value = column;

            Object^ result = cmd->ExecuteScalar();

            if (result == nullptr || result == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Missing Loans column: " + column);

            int bytes = Convert::ToInt32(result);

            if (bytes != -1 && value->Length > bytes / 2)
                throw gcnew ArgumentException(
                    column + " is longer than the database allows.");
        }

    public:
        static bool CanEditStatus(String^ status)
        {
            if (String::IsNullOrWhiteSpace(status))
                return false;

            status = status->Trim();

            return String::Equals(
                status, "Pending",
                StringComparison::OrdinalIgnoreCase)
                || String::Equals(
                    status, "Draft",
                    StringComparison::OrdinalIgnoreCase);
        }

        static DataTable^ GetLoan(
            AuthenticatedUser^ actor, int loanID)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException("Invalid Loan ID.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT l.LoanID, l.LoanCode, "
                    "l.ApplicantID, "
                    "a.ApplicantCode, "
                    "a.FullName AS ApplicantName, "
                    "l.LoanType, l.LoanPurpose, "
                    "l.RequestedAmount, "
                    "l.AnnualInterestRate, "
                    "l.TermMonths, l.LoanStatus "
                    "FROM dbo.Loans l "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID=l.ApplicantID "
                    "WHERE l.LoanID=@id",
                    con);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = loanID;

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

                if (table->Rows->Count != 1)
                    throw gcnew InvalidOperationException(
                        "Loan application not found.");

                return table;
            }
            finally
            {
                delete con;
            }
        }

        static void UpdateLoan(
            AuthenticatedUser^ actor,
            int loanID,
            String^ loanType,
            String^ purpose,
            Decimal amount,
            Decimal interestRate,
            int termMonths)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException("Invalid Loan ID.");

            loanType = loanType == nullptr
                ? "" : loanType->Trim();

            purpose = purpose == nullptr
                ? "" : purpose->Trim();

            if (String::IsNullOrWhiteSpace(loanType))
                throw gcnew ArgumentException(
                    "Loan type is required.");

            if (String::IsNullOrWhiteSpace(purpose))
                throw gcnew ArgumentException(
                    "Loan purpose is required.");

            if (Decimal::Compare(amount, Decimal::Zero) <= 0)
                throw gcnew ArgumentException(
                    "Requested amount must be greater than zero.");

            if (Decimal::Compare(interestRate, Decimal::Zero) < 0 ||
                Decimal::Compare(interestRate, Decimal(100)) > 0)
                throw gcnew ArgumentException(
                    "Interest rate must be between 0 and 100.");

            if (termMonths < 1 || termMonths > 360)
                throw gcnew ArgumentException(
                    "Term must be between 1 and 360 months.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();
                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                RequireUser(con, tx, actor);

                SqlCommand^ statusCmd = gcnew SqlCommand(
                    "SELECT LoanStatus FROM dbo.Loans "
                    "WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE LoanID=@id",
                    con, tx);

                statusCmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = loanID;

                Object^ statusValue =
                    statusCmd->ExecuteScalar();

                if (statusValue == nullptr ||
                    statusValue == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Loan application no longer exists.");

                String^ status =
                    Convert::ToString(statusValue);

                if (!CanEditStatus(status))
                    throw gcnew InvalidOperationException(
                        "This loan cannot be edited in its current "
                        "status: " + status);

                CheckLength(con, tx, "LoanType", loanType);
                CheckLength(con, tx, "LoanPurpose", purpose);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "UPDATE dbo.Loans SET "
                    "LoanType=@type, "
                    "LoanPurpose=@purpose, "
                    "RequestedAmount=@amount, "
                    "AnnualInterestRate=@rate, "
                    "TermMonths=@term "
                    "WHERE LoanID=@id "
                    "AND LoanStatus IN (N'Pending',N'Draft')",
                    con, tx);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = loanID;

                cmd->Parameters->Add(
                    "@type", SqlDbType::NVarChar, -1
                )->Value = loanType;

                cmd->Parameters->Add(
                    "@purpose", SqlDbType::NVarChar, -1
                )->Value = purpose;

                cmd->Parameters->Add(
                    "@amount", SqlDbType::Decimal
                )->Value = amount;

                cmd->Parameters->Add(
                    "@rate", SqlDbType::Decimal
                )->Value = interestRate;

                cmd->Parameters->Add(
                    "@term", SqlDbType::Int
                )->Value = termMonths;

                if (cmd->ExecuteNonQuery() != 1)
                    throw gcnew InvalidOperationException(
                        "The loan could not be updated. "
                        "Its status may have changed.");

                tx->Commit();
                tx = nullptr;
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
