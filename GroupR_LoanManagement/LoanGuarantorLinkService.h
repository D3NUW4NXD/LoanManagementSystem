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
    public ref class LoanGuarantorLinkService abstract sealed
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

        static DataTable^ ReadTable(SqlCommand^ cmd)
        {
            DataTable^ result = gcnew DataTable();
            SqlDataAdapter^ adapter =
                gcnew SqlDataAdapter(cmd);

            try
            {
                adapter->Fill(result);
            }
            finally
            {
                delete adapter;
            }

            return result;
        }

    public:
        static DataTable^ GetLoans(AuthenticatedUser^ actor)
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT l.LoanID, l.ApplicantID, "
                    "a.FullName AS ApplicantName, "
                    "CONVERT(NVARCHAR(20), l.LoanID) "
                    "+ N' - ' + a.FullName AS DisplayName "
                    "FROM dbo.Loans l "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID = l.ApplicantID "
                    "ORDER BY l.LoanID DESC",
                    con);

                return ReadTable(cmd);
            }
            finally
            {
                delete con;
            }
        }

        static DataTable^ GetEligibleGuarantors(
            AuthenticatedUser^ actor, int loanID)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid loan.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT g.GuarantorID, "
                    "g.GuarantorCode, g.FullName, "
                    "g.GuarantorCode + N' - ' + g.FullName "
                    "AS DisplayName "
                    "FROM dbo.Guarantors g "
                    "INNER JOIN dbo.Loans l "
                    "ON l.ApplicantID = g.ApplicantID "
                    "WHERE l.LoanID = @loan "
                    "AND NOT EXISTS ("
                    " SELECT 1 FROM dbo.LoanGuarantors lg "
                    " WHERE lg.LoanID = l.LoanID "
                    " AND lg.GuarantorID = g.GuarantorID"
                    ") ORDER BY g.FullName",
                    con);

                cmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                return ReadTable(cmd);
            }
            finally
            {
                delete con;
            }
        }

        static DataTable^ GetLinkedGuarantors(
            AuthenticatedUser^ actor, int loanID)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid loan.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();
                RequireUser(con, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT g.GuarantorID, "
                    "g.GuarantorCode, g.FullName, "
                    "g.RelationshipToApplicant, g.Phone, "
                    "lg.IsPrimaryGuarantor, "
                    "lg.GuaranteeAmount "
                    "FROM dbo.LoanGuarantors lg "
                    "INNER JOIN dbo.Guarantors g "
                    "ON lg.GuarantorID = g.GuarantorID "
                    "WHERE lg.LoanID = @loan "
                    "ORDER BY g.FullName",
                    con);

                cmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                return ReadTable(cmd);
            }
            finally
            {
                delete con;
            }
        }

        static void Assign(
            AuthenticatedUser^ actor,
            int loanID,
            int guarantorID)
        {
            if (loanID <= 0 || guarantorID <= 0)
                throw gcnew ArgumentException(
                    "Select a loan and guarantor.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();
                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                RequireUser(con, tx, actor);

                SqlCommand^ loanCmd = gcnew SqlCommand(
                    "SELECT ApplicantID FROM dbo.Loans "
                    "WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE LoanID = @loan",
                    con, tx);

                loanCmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                Object^ loanApplicant =
                    loanCmd->ExecuteScalar();

                if (loanApplicant == nullptr ||
                    loanApplicant == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "The selected loan does not exist.");

                SqlCommand^ guarantorCmd =
                    gcnew SqlCommand(
                        "SELECT ApplicantID "
                        "FROM dbo.Guarantors "
                        "WITH (UPDLOCK, HOLDLOCK) "
                        "WHERE GuarantorID = @guarantor",
                        con, tx);

                guarantorCmd->Parameters->Add(
                    "@guarantor", SqlDbType::Int
                )->Value = guarantorID;

                Object^ guarantorApplicant =
                    guarantorCmd->ExecuteScalar();

                if (guarantorApplicant == nullptr ||
                    guarantorApplicant == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "The selected guarantor does not exist.");

                if (Convert::ToInt32(loanApplicant) !=
                    Convert::ToInt32(guarantorApplicant))
                    throw gcnew InvalidOperationException(
                        "The guarantor belongs to a different applicant.");

                SqlCommand^ duplicate =
                    gcnew SqlCommand(
                        "SELECT COUNT(*) "
                        "FROM dbo.LoanGuarantors "
                        "WITH (UPDLOCK, HOLDLOCK) "
                        "WHERE LoanID = @loan "
                        "AND GuarantorID = @guarantor",
                        con, tx);

                duplicate->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                duplicate->Parameters->Add(
                    "@guarantor", SqlDbType::Int
                )->Value = guarantorID;

                if (Convert::ToInt32(
                    duplicate->ExecuteScalar()) > 0)
                    throw gcnew InvalidOperationException(
                        "This guarantor is already linked to the loan.");

                // CreatedByUserID is REQUIRED in your database.
                SqlCommand^ insertCmd = gcnew SqlCommand(
                    "INSERT INTO dbo.LoanGuarantors "
                    "(LoanID, GuarantorID, CreatedByUserID) "
                    "VALUES (@loan, @guarantor, @creator)",
                    con, tx);

                insertCmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                insertCmd->Parameters->Add(
                    "@guarantor", SqlDbType::Int
                )->Value = guarantorID;

                insertCmd->Parameters->Add(
                    "@creator", SqlDbType::Int
                )->Value = actor->UserID;

                if (insertCmd->ExecuteNonQuery() != 1)
                    throw gcnew InvalidOperationException(
                        "Could not link the guarantor.");

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
