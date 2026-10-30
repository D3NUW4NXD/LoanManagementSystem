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
    public ref class LoanListService abstract sealed
    {
    private:
        static void ValidateUser(
            SqlConnection^ con,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Please log in again.");
            }

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID = @id "
                "AND Username = @username "
                "AND IsActive = 1 "
                "AND RoleID IN (1,2,3)",
                con);

            cmd->Parameters->Add(
                "@id", SqlDbType::Int
            )->Value = actor->UserID;

            cmd->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256
            )->Value = actor->Username;

            if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
            {
                throw gcnew UnauthorizedAccessException(
                    "Your account is not authorized.");
            }
        }

    public:
        static DataTable^ GetAllLoans(
            AuthenticatedUser^ actor,
            String^ search)
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                ValidateUser(con, actor);

                if (search == nullptr)
                    search = "";

                search = search->Trim();

                if (search->Length > 100)
                    search = search->Substring(0, 100);

                String^ sql =
                    "SELECT "
                    "l.LoanID, "
                    "l.LoanCode, "
                    "a.ApplicantCode, "
                    "a.FullName AS ApplicantName, "
                    "l.LoanType, "
                    "l.LoanPurpose, "
                    "l.RequestedAmount, "
                    "l.AnnualInterestRate, "
                    "l.TermMonths, "
                    "l.LoanStatus, "
                    "l.ApplicationDate "
                    "FROM dbo.Loans l "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID = l.ApplicantID "
                    "WHERE ("
                    "@search = N'' "
                    "OR l.LoanCode LIKE N'%' + @search + N'%' "
                    "OR a.ApplicantCode LIKE N'%' + @search + N'%' "
                    "OR a.FullName LIKE N'%' + @search + N'%' "
                    "OR l.LoanType LIKE N'%' + @search + N'%' "
                    "OR l.LoanStatus LIKE N'%' + @search + N'%' "
                    ") "
                    "ORDER BY l.ApplicationDate DESC, l.LoanID DESC";

                SqlCommand^ cmd =
                    gcnew SqlCommand(sql, con);

                cmd->Parameters->Add(
                    "@search", SqlDbType::NVarChar, 100
                )->Value = search;

                DataTable^ results = gcnew DataTable();

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                try
                {
                    adapter->Fill(results);
                }
                finally
                {
                    delete adapter;
                }

                return results;
            }
            finally
            {
                delete con;
            }
        }

        static int GetTotalLoans(AuthenticatedUser^ actor)
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                ValidateUser(con, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Loans", con);

                return Convert::ToInt32(
                    cmd->ExecuteScalar());
            }
            finally
            {
                delete con;
            }
        }
    };
}
