#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "DatabaseConnection.h"
#include "AuthenticationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class EligibilityHistoryService abstract sealed
    {
    private:
        static bool HasColumn(
            SqlConnection^ con,
            String^ column)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM sys.columns "
                "WHERE object_id=OBJECT_ID(N'dbo.EligibilityChecks') "
                "AND name=@column", con);

            cmd->Parameters->Add(
                "@column", SqlDbType::NVarChar, 128
            )->Value = column;

            return Convert::ToInt32(cmd->ExecuteScalar()) > 0;
        }

    public:
        static DataTable^ GetHistory(
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

            try
            {
                con->Open();

                SqlCommand^ security = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@id AND Username=@username "
                    "AND IsActive=1 AND RoleID IN (1,2,3)",
                    con);

                security->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = actor->UserID;

                security->Parameters->Add(
                    "@username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                if (Convert::ToInt32(
                    security->ExecuteScalar()) != 1)
                    throw gcnew UnauthorizedAccessException(
                        "Access denied for this account.");

                SqlCommand^ loanCheck = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Loans "
                    "WHERE LoanID=@id", con);

                loanCheck->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = loanID;

                if (Convert::ToInt32(
                    loanCheck->ExecuteScalar()) != 1)
                    throw gcnew InvalidOperationException(
                        "The selected loan does not exist.");

                SqlCommand^ tableCheck = gcnew SqlCommand(
                    "SELECT CASE WHEN "
                    "OBJECT_ID(N'dbo.EligibilityChecks', N'U') "
                    "IS NULL THEN 0 ELSE 1 END",
                    con);

                if (Convert::ToInt32(
                    tableCheck->ExecuteScalar()) != 1)
                    throw gcnew InvalidOperationException(
                        "The EligibilityChecks table was not found.");

                if (!HasColumn(con, "LoanID"))
                    throw gcnew InvalidOperationException(
                        "EligibilityChecks does not contain LoanID. "
                        "Its relationship to loans must be verified "
                        "before displaying history.");

                String^ orderBy = "";

                array<String^>^ candidates = {
                    "CheckedAt",
                    "AssessmentDate",
                    "CreatedAt",
                    "CheckDate",
                    "EligibilityCheckID"
                };

                for each (String^ candidate in candidates)
                {
                    if (HasColumn(con, candidate))
                    {
                        orderBy =
                            " ORDER BY [" + candidate + "] DESC";
                        break;
                    }
                }

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT TOP (200) * "
                    "FROM dbo.EligibilityChecks "
                    "WHERE LoanID=@loan" + orderBy,
                    con);

                cmd->Parameters->Add(
                    "@loan", SqlDbType::Int
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

                return table;
            }
            finally
            {
                delete con;
            }
        }
    };
}
