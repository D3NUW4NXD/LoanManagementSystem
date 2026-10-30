#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"
#include "LoanEligibilityCalculator.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class EligibilitySaveService abstract sealed
    {
    private:
        static void AddDecimal(
            SqlCommand^ cmd,
            String^ name,
            Decimal value,
            Byte precision,
            Byte scale)
        {
            SqlParameter^ p =
                cmd->Parameters->Add(name, SqlDbType::Decimal);

            p->Precision = precision;
            p->Scale = scale;
            p->Value = value;
        }

    public:
        static int SaveAssessment(
            AuthenticatedUser^ actor,
            int loanID)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            if (loanID <= 0)
                throw gcnew ArgumentException(
                    "Invalid loan application.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();

                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                // Validate permissions against the database.
                SqlCommand^ security = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@user "
                    "AND Username=@username "
                    "AND IsActive=1 "
                    "AND RoleID IN (1,2,3)",
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
                        "You do not have permission to save "
                        "eligibility assessments.");

                // Read current loan and applicant values.
                // Lock the rows while creating the snapshot.
                SqlCommand^ lookup = gcnew SqlCommand(
                    "SELECT "
                    "l.LoanStatus, "
                    "l.RequestedAmount, "
                    "l.AnnualInterestRate, "
                    "l.TermMonths, "
                    "a.MonthlyIncome, "
                    "a.ExistingMonthlyDebt, "
                    "a.CreditScore "
                    "FROM dbo.Loans AS l WITH (UPDLOCK, HOLDLOCK) "
                    "INNER JOIN dbo.Applicants AS a WITH (HOLDLOCK) "
                    "ON a.ApplicantID=l.ApplicantID "
                    "WHERE l.LoanID=@loan",
                    con, tx);

                lookup->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                SqlDataReader^ reader =
                    lookup->ExecuteReader();

                bool found = false;
                String^ status = "";

                Decimal amount = Decimal::Zero;
                Decimal rate = Decimal::Zero;
                Decimal income = Decimal::Zero;
                Decimal debt = Decimal::Zero;
                int months = 0;
                Object^ creditScore = DBNull::Value;

                try
                {
                    if (reader->Read())
                    {
                        found = true;

                        status = Convert::ToString(
                            reader["LoanStatus"]);

                        amount = Convert::ToDecimal(
                            reader["RequestedAmount"]);

                        rate = Convert::ToDecimal(
                            reader["AnnualInterestRate"]);

                        months = Convert::ToInt32(
                            reader["TermMonths"]);

                        income = Convert::ToDecimal(
                            reader["MonthlyIncome"]);

                        debt = Convert::ToDecimal(
                            reader["ExistingMonthlyDebt"]);

                        creditScore = reader["CreditScore"];
                    }
                }
                finally
                {
                    reader->Close();
                }

                if (!found)
                    throw gcnew InvalidOperationException(
                        "Loan application not found.");

                if (!String::Equals(
                    status->Trim(), "Pending",
                    StringComparison::OrdinalIgnoreCase))
                    throw gcnew InvalidOperationException(
                        "Saving assessments is allowed only "
                        "for Pending loans. Current status: " +
                        status);

                // Recalculate using the current database values.
                LoanEligibilityResult^ result =
                    LoanEligibilityCalculator::Calculate(
                        income, debt, amount, rate, months);

                // Database column: decimal(9,4).
                Decimal dti = Decimal::Round(
                    result->DebtToIncomePercent,
                    4,
                    MidpointRounding::AwayFromZero);

                if (dti < Decimal::Zero ||
                    dti > Decimal(99999))
                    throw gcnew InvalidOperationException(
                        "DTI is outside the database's "
                        "supported numeric range.");

                // Not an approval or a policy-based verdict.
                String^ assessmentResult = "Needs Review";
                String^ criteriaVersion =
                    "PREVIEW-V1-NO-POLICY";

                String^ notes =
                    "Preliminary affordability calculation. "
                    "Estimated EMI assumes equal monthly "
                    "payments and a fixed annual interest rate. "
                    "No official credit-score, DTI or lending "
                    "eligibility thresholds have been applied. "
                    "Administrator review is required.";

                SqlCommand^ insert = gcnew SqlCommand(
                    "INSERT INTO dbo.EligibilityChecks "
                    "("
                    "LoanID, CheckedByUserID, CheckedAt, "
                    "MonthlyIncome, ExistingMonthlyDebt, "
                    "ProposedMonthlyEMI, DebtToIncomeRatio, "
                    "CreditScore, AssessmentResult, "
                    "AssessmentNotes, CriteriaVersion"
                    ") "
                    "OUTPUT INSERTED.EligibilityCheckID "
                    "VALUES "
                    "("
                    "@loan, @user, SYSUTCDATETIME(), "
                    "@income, @debt, @emi, @dti, "
                    "@score, @result, @notes, @version"
                    ")",
                    con, tx);

                insert->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                insert->Parameters->Add(
                    "@user", SqlDbType::Int
                )->Value = actor->UserID;

                AddDecimal(
                    insert, "@income",
                    result->MonthlyIncome, 18, 2);

                AddDecimal(
                    insert, "@debt",
                    result->ExistingMonthlyDebt, 18, 2);

                AddDecimal(
                    insert, "@emi",
                    result->EstimatedInstallment, 18, 2);

                AddDecimal(
                    insert, "@dti",
                    dti, 9, 4);

                insert->Parameters->Add(
                    "@score", SqlDbType::Int
                )->Value = creditScore;

                insert->Parameters->Add(
                    "@result", SqlDbType::NVarChar, 20
                )->Value = assessmentResult;

                insert->Parameters->Add(
                    "@notes", SqlDbType::NVarChar, 1000
                )->Value = notes;

                insert->Parameters->Add(
                    "@version", SqlDbType::NVarChar, 30
                )->Value = criteriaVersion;

                Object^ insertedID =
                    insert->ExecuteScalar();

                if (insertedID == nullptr ||
                    insertedID == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Assessment could not be saved.");

                int assessmentID =
                    Convert::ToInt32(insertedID);

                tx->Commit();
                tx = nullptr;

                return assessmentID;
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
