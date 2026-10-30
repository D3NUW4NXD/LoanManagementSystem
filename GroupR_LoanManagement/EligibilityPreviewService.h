#pragma once

#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"
#include "LoanEligibilityCalculator.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class EligibilityPreviewService abstract sealed
    {
    public:
        static DataTable^ GetLoanData(
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
                    "A valid loan must be selected.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ security = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@user "
                    "AND Username=@username "
                    "AND IsActive=1 "
                    "AND RoleID IN (1,2,3)",
                    con);

                security->Parameters->Add(
                    "@user", SqlDbType::Int
                )->Value = actor->UserID;

                security->Parameters->Add(
                    "@username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                if (Convert::ToInt32(
                    security->ExecuteScalar()) != 1)
                    throw gcnew UnauthorizedAccessException(
                        "Your account does not have access.");

                String^ sql =
                    "SELECT "
                    "l.LoanID, l.LoanCode, l.LoanType, "
                    "l.LoanStatus, l.RequestedAmount, "
                    "l.AnnualInterestRate, l.TermMonths, "
                    "a.ApplicantID, a.ApplicantCode, "
                    "a.FullName AS ApplicantName, "
                    "a.MonthlyIncome, "
                    "a.ExistingMonthlyDebt, "
                    "a.CreditScore "
                    "FROM dbo.Loans l "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID=l.ApplicantID "
                    "WHERE l.LoanID=@loan";

                SqlCommand^ command =
                    gcnew SqlCommand(sql, con);

                command->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                DataTable^ result = gcnew DataTable();
                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(command);

                try
                {
                    adapter->Fill(result);
                }
                finally
                {
                    delete adapter;
                }

                if (result->Rows->Count != 1)
                    throw gcnew InvalidOperationException(
                        "Loan application was not found.");

                return result;
            }
            finally
            {
                delete con;
            }
        }
    };
}
