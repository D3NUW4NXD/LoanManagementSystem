#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Collections::Generic;

namespace GroupR_LoanManagement
{
    public ref class LoanDecisionService abstract sealed
    {
    private:
        static void RequireAdministrator(
            SqlConnection^ con,
            SqlTransaction^ tx,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr || actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Login is required.");

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND Username=@username "
                "AND IsActive=1 AND RoleID IN (1,2)",
                con, tx);

            cmd->Parameters->Add(
                "@id", SqlDbType::Int)->Value = actor->UserID;

            cmd->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256
            )->Value = actor->Username;

            if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
                throw gcnew UnauthorizedAccessException(
                    "Only active administrators may decide loans.");
        }

        static void ValidateTextLength(
            SqlConnection^ con,
            SqlTransaction^ tx,
            String^ column,
            String^ value)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT max_length FROM sys.columns "
                "WHERE object_id=OBJECT_ID('dbo.Loans') "
                "AND name=@name",
                con, tx);

            cmd->Parameters->Add(
                "@name", SqlDbType::NVarChar, 128
            )->Value = column;

            Object^ raw = cmd->ExecuteScalar();

            if (raw == nullptr || raw == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Missing Loans column: " + column);

            int maxBytes = Convert::ToInt32(raw);

            if (maxBytes != -1 && value->Length * 2 > maxBytes)
                throw gcnew ArgumentException(
                    column + " exceeds the database length.");
        }

        static bool NameIs(
            String^ name, String^ option)
        {
            return String::Equals(
                name, option,
                StringComparison::OrdinalIgnoreCase);
        }

        static void InsertAudit(
            SqlConnection^ con,
            SqlTransaction^ tx,
            AuthenticatedUser^ actor,
            int loanID,
            bool approving,
            Decimal requestedAmount,
            Decimal approvedAmount,
            String^ rejectionReason)
        {
            SqlCommand^ metadata = gcnew SqlCommand(
                "SELECT c.name, t.name AS TypeName, "
                "c.is_nullable, c.is_identity, c.is_computed, "
                "CASE WHEN c.default_object_id=0 THEN 0 ELSE 1 END "
                "AS HasDefault, c.max_length "
                "FROM sys.columns c "
                "JOIN sys.types t ON t.user_type_id=c.user_type_id "
                "WHERE c.object_id=OBJECT_ID('dbo.AuditLogs') "
                "ORDER BY c.column_id",
                con, tx);

            SqlDataReader^ reader = metadata->ExecuteReader();
            DataTable^ columns = gcnew DataTable();

            try
            {
                columns->Load(reader);
            }
            finally
            {
                reader->Close();
            }

            if (columns->Rows->Count == 0)
                throw gcnew InvalidOperationException(
                    "AuditLogs table is missing or inaccessible. "
                    "Loan decision cancelled.");

            String^ action = approving
                ? "Loan Approved" : "Loan Rejected";

            String^ status = approving
                ? "Approved" : "Rejected";

            String^ details = approving
                ? "LoanID=" + loanID.ToString() +
                  "; Requested=" + requestedAmount.ToString() +
                  "; Approved=" + approvedAmount.ToString()
                : "LoanID=" + loanID.ToString() +
                  "; Rejected; Reason=" + rejectionReason;

            List<String^>^ names = gcnew List<String^>();
            List<String^>^ parameters = gcnew List<String^>();
            List<Object^>^ values = gcnew List<Object^>();

            bool hasActor = false;
            bool hasAction = false;

            for each (DataRow^ row in columns->Rows)
            {
                String^ name = Convert::ToString(row["name"]);
                String^ key = name->ToLowerInvariant();

                bool nullable = Convert::ToBoolean(
                    row["is_nullable"]);
                bool identity = Convert::ToBoolean(
                    row["is_identity"]);
                bool computed = Convert::ToBoolean(
                    row["is_computed"]);
                bool hasDefault = Convert::ToInt32(
                    row["HasDefault"]) != 0;

                if (identity || computed)
                    continue;

                Object^ value = nullptr;
                bool mapped = true;

                if (key == "userid" ||
                    key == "actoruserid" ||
                    key == "performedbyuserid" ||
                    key == "createdbyuserid")
                {
                    value = actor->UserID;
                    hasActor = true;
                }
                else if (key == "username" ||
                         key == "actorusername")
                {
                    value = actor->Username;
                    hasActor = true;
                }
                else if (key == "roleid")
                {
                    value = actor->RoleID;
                }
                else if (key == "action" ||
                         key == "actiontype" ||
                         key == "eventtype" ||
                         key == "activity")
                {
                    value = action;
                    hasAction = true;
                }
                else if (key == "entitytype" ||
                         key == "entityname" ||
                         key == "tablename" ||
                         key == "modulename" ||
                         key == "module")
                {
                    value = "Loans";
                }
                else if (key == "entityid" ||
                         key == "recordid" ||
                         key == "targetid" ||
                         key == "loanid")
                {
                    value = loanID;
                }
                else if (key == "details" ||
                         key == "description" ||
                         key == "message" ||
                         key == "remarks")
                {
                    value = details;
                }
                else if (key == "oldvalue" ||
                         key == "oldvalues")
                {
                    value = "Pending";
                }
                else if (key == "newvalue" ||
                         key == "newvalues")
                {
                    value = status;
                }
                else if (key == "status")
                {
                    value = status;
                }
                else if (key == "createdat" ||
                         key == "timestamp" ||
                         key == "occurredat" ||
                         key == "actiondate" ||
                         key == "performedat" ||
                         key == "logdate")
                {
                    value = DateTime::UtcNow;
                }
                else
                {
                    mapped = false;
                }

                if (!mapped)
                {
                    if (!nullable && !hasDefault)
                        throw gcnew InvalidOperationException(
                            "AuditLogs requires unsupported column: " +
                            name + ". Decision cancelled.");

                    continue;
                }

                String^ typeName =
                    Convert::ToString(row["TypeName"])->ToLowerInvariant();

                if (typeName == "nvarchar" ||
                    typeName == "nchar" ||
                    typeName == "varchar" ||
                    typeName == "char")
                {
                    String^ textValue =
                        Convert::ToString(value);

                    int maxBytes =
                        Convert::ToInt32(row["max_length"]);

                    int allowed = maxBytes;

                    if (typeName == "nvarchar" ||
                        typeName == "nchar")
                        allowed = maxBytes / 2;

                    if (maxBytes != -1 &&
                        textValue->Length > allowed)
                        throw gcnew InvalidOperationException(
                            "AuditLogs column " + name +
                            " is too short. Decision cancelled.");
                }

                String^ parameter =
                    "@audit" + values->Count.ToString();

                names->Add("[" + name->Replace("]", "]]") + "]");
                parameters->Add(parameter);
                values->Add(value);
            }

            if (!hasActor || !hasAction)
                throw gcnew InvalidOperationException(
                    "AuditLogs needs a compatible actor and "
                    "action column. Decision cancelled.");

            String^ sql =
                "INSERT INTO dbo.AuditLogs (" +
                String::Join(",", names->ToArray()) +
                ") VALUES (" +
                String::Join(",", parameters->ToArray()) + ")";

            SqlCommand^ insert = gcnew SqlCommand(sql, con, tx);

            for (int i = 0; i < values->Count; i++)
            {
                insert->Parameters->AddWithValue(
                    parameters[i], values[i]);
            }

            if (insert->ExecuteNonQuery() != 1)
                throw gcnew InvalidOperationException(
                    "Audit entry could not be saved.");
        }

        static void Decide(
            AuthenticatedUser^ actor,
            int loanID,
            bool approving,
            Decimal approvedAmount,
            String^ rejectionReason)
        {
            if (loanID <= 0)
                throw gcnew ArgumentException("Invalid Loan ID.");

            rejectionReason = rejectionReason == nullptr
                ? "" : rejectionReason->Trim();

            if (!approving &&
                String::IsNullOrWhiteSpace(rejectionReason))
                throw gcnew ArgumentException(
                    "A rejection reason is required.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();
                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                RequireAdministrator(con, tx, actor);

                SqlCommand^ lookup = gcnew SqlCommand(
                    "SELECT RequestedAmount, LoanStatus, DecisionAt "
                    "FROM dbo.Loans WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE LoanID=@id",
                    con, tx);

                lookup->Parameters->Add(
                    "@id", SqlDbType::Int)->Value = loanID;

                SqlDataReader^ reader = lookup->ExecuteReader();

                Decimal requested = Decimal::Zero;
                String^ currentStatus = "";
                bool previouslyDecided = false;
                bool found = false;

                try
                {
                    if (reader->Read())
                    {
                        found = true;
                        requested = Convert::ToDecimal(
                            reader["RequestedAmount"]);
                        currentStatus = Convert::ToString(
                            reader["LoanStatus"]);
                        previouslyDecided =
                            reader["DecisionAt"] != DBNull::Value;
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
                    currentStatus->Trim(), "Pending",
                    StringComparison::OrdinalIgnoreCase) ||
                    previouslyDecided)
                    throw gcnew InvalidOperationException(
                        "Only undecided Pending loans can "
                        "be approved or rejected.");

                if (approving &&
                    (Decimal::Compare(approvedAmount, Decimal::Zero) <= 0 ||
                     Decimal::Compare(approvedAmount, requested) > 0))
                    throw gcnew ArgumentException(
                        "Approved amount must be greater than zero "
                        "and cannot exceed the requested amount.");

                ValidateTextLength(
                    con, tx, "LoanStatus",
                    approving ? "Approved" : "Rejected");

                if (!approving)
                    ValidateTextLength(
                        con, tx, "RejectionReason", rejectionReason);

                String^ updateSQL = approving
                    ? "UPDATE dbo.Loans SET "
                      "LoanStatus=N'Approved', "
                      "ApprovedAmount=@amount, "
                      "RejectionReason=NULL, "
                      "DecisionByUserID=@user, "
                      "DecisionAt=SYSUTCDATETIME() "
                      "WHERE LoanID=@id AND LoanStatus=@status "
                      "AND DecisionAt IS NULL"
                    : "UPDATE dbo.Loans SET "
                      "LoanStatus=N'Rejected', "
                      "ApprovedAmount=NULL, "
                      "RejectionReason=@reason, "
                      "DecisionByUserID=@user, "
                      "DecisionAt=SYSUTCDATETIME() "
                      "WHERE LoanID=@id AND LoanStatus=@status "
                      "AND DecisionAt IS NULL";

                SqlCommand^ update =
                    gcnew SqlCommand(updateSQL, con, tx);

                update->Parameters->Add(
                    "@id", SqlDbType::Int)->Value = loanID;

                update->Parameters->Add(
                    "@user", SqlDbType::Int)->Value = actor->UserID;

                update->Parameters->Add(
                    "@status", SqlDbType::NVarChar, 100
                )->Value = currentStatus;

                if (approving)
                {
                    SqlParameter^ amountParameter =
                        update->Parameters->Add(
                            "@amount", SqlDbType::Decimal);

                    amountParameter->Precision = 28;
                    amountParameter->Scale = 2;
                    amountParameter->Value = approvedAmount;
                }
                else
                {
                    update->Parameters->Add(
                        "@reason", SqlDbType::NVarChar, -1
                    )->Value = rejectionReason;
                }

                if (update->ExecuteNonQuery() != 1)
                    throw gcnew InvalidOperationException(
                        "Loan status changed before the decision. "
                        "No changes were saved.");

                InsertAudit(
                    con, tx, actor, loanID, approving,
                    requested, approvedAmount, rejectionReason);

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

    public:
        static void Approve(
            AuthenticatedUser^ actor,
            int loanID,
            Decimal amount)
        {
            Decide(actor, loanID, true, amount, "");
        }

        static void Reject(
            AuthenticatedUser^ actor,
            int loanID,
            String^ reason)
        {
            Decide(
                actor, loanID, false,
                Decimal::Zero, reason);
        }
    };
}
