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
    public ref class LoanDetailsService abstract sealed
    {
    private:
        static void RequireUser(
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
                "WHERE UserID=@id AND Username=@username "
                "AND IsActive=1 AND RoleID IN (1,2,3)",
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

        static DataTable^ ExecuteTable(
            SqlConnection^ con,
            String^ sql,
            int loanID)
        {
            SqlCommand^ cmd = gcnew SqlCommand(sql, con);

            cmd->Parameters->Add(
                "@loan", SqlDbType::Int
            )->Value = loanID;

            DataTable^ result = gcnew DataTable();
            SqlDataAdapter^ adapter = gcnew SqlDataAdapter(cmd);

            try {
                adapter->Fill(result);
            }
            finally {
                delete adapter;
            }

            return result;
        }

    public:
        static DataSet^ GetDetails(
            AuthenticatedUser^ actor,
            int loanID)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException("Invalid Loan ID.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, actor);

                String^ loanSql =
                    "SELECT l.LoanID, l.LoanCode, "
                    "l.ApplicantID, a.ApplicantCode, "
                    "a.FullName AS ApplicantName, "
                    "a.Phone AS ApplicantPhone, "
                    "a.Email AS ApplicantEmail, "
                    "l.LoanType, l.LoanPurpose, "
                    "l.RequestedAmount, "
                    "l.AnnualInterestRate, l.TermMonths, "
                    "l.LoanStatus, l.ApplicationDate, "
                    "l.ApprovedAmount, l.RejectionReason, "
                    "l.DecisionAt, l.DisbursementDate, "
                    "creator.FullName AS CreatedBy, "
                    "decisionUser.FullName AS DecisionBy "
                    "FROM dbo.Loans l "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID=l.ApplicantID "
                    "LEFT JOIN dbo.Users creator "
                    "ON creator.UserID=l.CreatedByUserID "
                    "LEFT JOIN dbo.Users decisionUser "
                    "ON decisionUser.UserID=l.DecisionByUserID "
                    "WHERE l.LoanID=@loan";

                String^ guarantorSql =
                    "SELECT g.GuarantorCode, g.FullName, "
                    "g.Phone, g.RelationshipToApplicant, "
                    "lg.IsPrimaryGuarantor, "
                    "lg.GuaranteeAmount "
                    "FROM dbo.LoanGuarantors lg "
                    "INNER JOIN dbo.Guarantors g "
                    "ON g.GuarantorID=lg.GuarantorID "
                    "WHERE lg.LoanID=@loan "
                    "ORDER BY lg.IsPrimaryGuarantor DESC, "
                    "g.FullName";

                DataTable^ loan =
                    ExecuteTable(con, loanSql, loanID);

                if (loan->Rows->Count != 1)
                    throw gcnew InvalidOperationException(
                        "Loan application not found.");

                DataTable^ guarantors =
                    ExecuteTable(con, guarantorSql, loanID);

                loan->TableName = "Loan";
                guarantors->TableName = "Guarantors";

                DataSet^ details = gcnew DataSet();
                details->Tables->Add(loan);
                details->Tables->Add(guarantors);

                return details;
            }
            finally
            {
                delete con;
            }
        }
    };
}
