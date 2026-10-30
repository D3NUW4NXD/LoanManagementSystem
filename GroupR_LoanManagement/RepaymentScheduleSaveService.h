#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"
#include "RepaymentPlanCalculator.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Collections::Generic;

namespace GroupR_LoanManagement
{
    public ref class RepaymentScheduleSaveService abstract sealed
    {
    private:
        static void AddMoney(
            SqlCommand^ cmd,
            String^ name,
            Decimal amount,
            Byte precision,
            Byte scale)
        {
            SqlParameter^ p =
                cmd->Parameters->Add(
                    name, SqlDbType::Decimal);

            p->Precision = precision;
            p->Scale = scale;
            p->Value = amount;
        }

        static bool IsComputed(
            SqlConnection^ con,
            SqlTransaction^ tx,
            String^ columnName)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT is_computed "
                "FROM sys.columns "
                "WHERE object_id = "
                "OBJECT_ID(N'dbo.RepaymentSchedule') "
                "AND name = @column",
                con, tx);

            cmd->Parameters->Add(
                "@column", SqlDbType::NVarChar, 128
            )->Value = columnName;

            Object^ value = cmd->ExecuteScalar();

            if (value == nullptr ||
                value == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Missing database column: " + columnName);

            return Convert::ToBoolean(value);
        }

    public:
        static int GenerateAndSave(
            AuthenticatedUser^ actor,
            int loanID,
            DateTime firstDueDate)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            if (loanID <= 0)
                throw gcnew ArgumentException(
                    "Invalid loan ID.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();

                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                // Only active administrators can generate
                // persistent repayment schedules.
                SqlCommand^ security = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@user "
                    "AND Username=@username "
                    "AND IsActive=1 "
                    "AND RoleID IN (1,2)",
                    con, tx);

                security->Parameters->Add(
                    "@user", SqlDbType::Int
                )->Value = actor->UserID;

                security->Parameters->Add(
                    "@username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                if (Convert::ToInt32(
                    security->ExecuteScalar()) != 1)
                    throw gcnew UnauthorizedAccessException(
                        "Only active administrators can "
                        "generate repayment schedules.");

                // Lock the loan while reading its approved
                // terms and creating the schedule.
                SqlCommand^ loanQuery = gcnew SqlCommand(
                    "SELECT LoanStatus, ApprovedAmount, "
                    "AnnualInterestRate, TermMonths, "
                    "DisbursementDate "
                    "FROM dbo.Loans WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE LoanID=@loan",
                    con, tx);

                loanQuery->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                SqlDataReader^ reader =
                    loanQuery->ExecuteReader();

                bool found = false;
                String^ status = "";
                Decimal amount = Decimal::Zero;
                Decimal rate = Decimal::Zero;
                int months = 0;
                DateTime disbursementDate =
                    DateTime::MinValue;

                try
                {
                    if (reader->Read())
                    {
                        found = true;

                        status = Convert::ToString(
                            reader["LoanStatus"])->Trim();

                        if (reader["ApprovedAmount"] ==
                            DBNull::Value)
                            throw gcnew InvalidOperationException(
                                "The loan does not have "
                                "an approved amount.");

                        if (reader["DisbursementDate"] ==
                            DBNull::Value)
                            throw gcnew InvalidOperationException(
                                "Record the disbursement date "
                                "before generating a schedule.");

                        amount = Convert::ToDecimal(
                            reader["ApprovedAmount"]);

                        rate = Convert::ToDecimal(
                            reader["AnnualInterestRate"]);

                        months = Convert::ToInt32(
                            reader["TermMonths"]);

                        disbursementDate =
                            Convert::ToDateTime(
                                reader["DisbursementDate"]);
                    }
                }
                finally
                {
                    reader->Close();
                }

                if (!found)
                    throw gcnew InvalidOperationException(
                        "Loan not found.");

                bool approved =
                    String::Equals(
                        status, "Approved",
                        StringComparison::OrdinalIgnoreCase);

                bool disbursed =
                    String::Equals(
                        status, "Disbursed",
                        StringComparison::OrdinalIgnoreCase);

                if (!approved && !disbursed)
                    throw gcnew InvalidOperationException(
                        "Loan must be Approved or Disbursed. "
                        "Current status: " + status);

                if (amount <= Decimal::Zero)
                    throw gcnew InvalidOperationException(
                        "Approved amount must be positive.");

                if (firstDueDate.Date <=
                    disbursementDate.Date)
                    throw gcnew InvalidOperationException(
                        "First installment must be due "
                        "after the disbursement date.");

                // Do not create a duplicate schedule.
                SqlCommand^ existing = gcnew SqlCommand(
                    "SELECT COUNT(*) "
                    "FROM dbo.RepaymentSchedule "
                    "WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE LoanID=@loan",
                    con, tx);

                existing->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                if (Convert::ToInt32(
                    existing->ExecuteScalar()) > 0)
                    throw gcnew InvalidOperationException(
                        "This loan already has a repayment "
                        "schedule. Existing installments "
                        "have not been modified.");

                List<RepaymentScheduleRow^>^ rows =
                    RepaymentPlanCalculator::Generate(
                        amount, rate, months,
                        firstDueDate.Date);

                // Computed SQL columns must not appear
                // in INSERT statements.
                bool writeTotalDue =
                    !IsComputed(con, tx, "TotalDue");

                bool writeTotalPaid =
                    !IsComputed(con, tx, "TotalPaid");

                bool writeBalance =
                    !IsComputed(con, tx, "BalanceRemaining");

                String^ columns =
                    "LoanID, InstallmentNumber, DueDate, "
                    "PrincipalDue, InterestDue";

                String^ parameters =
                    "@loan, @number, @due, "
                    "@principal, @interest";

                if (writeTotalDue)
                {
                    columns += ", TotalDue";
                    parameters += ", @total";
                }

                if (writeTotalPaid)
                {
                    columns += ", TotalPaid";
                    parameters += ", @paid";
                }

                if (writeBalance)
                {
                    columns += ", BalanceRemaining";
                    parameters += ", @balance";
                }

                String^ insertSql =
                    "INSERT INTO dbo.RepaymentSchedule (" +
                    columns + ") VALUES (" +
                    parameters + ")";

                int inserted = 0;

                for each (RepaymentScheduleRow^ row
                    in rows)
                {
                    SqlCommand^ insert =
                        gcnew SqlCommand(
                            insertSql, con, tx);

                    insert->Parameters->Add(
                        "@loan", SqlDbType::Int
                    )->Value = loanID;

                    insert->Parameters->Add(
                        "@number", SqlDbType::Int
                    )->Value = row->InstallmentNumber;

                    insert->Parameters->Add(
                        "@due", SqlDbType::Date
                    )->Value = row->DueDate.Date;

                    AddMoney(
                        insert, "@principal",
                        row->PrincipalAmount, 18, 2);

                    AddMoney(
                        insert, "@interest",
                        row->InterestAmount, 18, 2);

                    if (writeTotalDue)
                        AddMoney(
                            insert, "@total",
                            row->MonthlyPayment, 19, 2);

                    if (writeTotalPaid)
                        AddMoney(
                            insert, "@paid",
                            Decimal::Zero, 19, 2);

                    if (writeBalance)
                        AddMoney(
                            insert, "@balance",
                            row->ClosingBalance, 21, 2);

                    // PrincipalPaid and InterestPaid
                    // default to zero.
                    // PaymentStatus defaults to Scheduled.
                    // CreatedAt defaults to UTC now.
                    inserted += insert->ExecuteNonQuery();
                }

                if (inserted != rows->Count)
                    throw gcnew InvalidOperationException(
                        "Repayment schedule insertion "
                        "was incomplete.");

                tx->Commit();
                tx = nullptr;

                return inserted;
            }
            catch (Exception^)
            {
                if (tx != nullptr)
                {
                    try
                    {
                        tx->Rollback();
                    }
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
