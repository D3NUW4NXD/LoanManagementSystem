#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class PaymentRecordingService abstract sealed
    {
    private:
        static void MoneyParameter(
            SqlCommand^ cmd,
            String^ name,
            Decimal value,
            Byte precision)
        {
            SqlParameter^ p =
                cmd->Parameters->Add(
                    name, SqlDbType::Decimal);

            p->Precision = precision;
            p->Scale = 2;
            p->Value = value;
        }

        static bool ComputedColumn(
            SqlConnection^ con,
            SqlTransaction^ tx,
            String^ tableName,
            String^ columnName)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT c.is_computed "
                "FROM sys.columns c "
                "WHERE c.object_id = "
                "OBJECT_ID(@tableName) "
                "AND c.name = @columnName",
                con, tx);

            cmd->Parameters->Add(
                "@tableName", SqlDbType::NVarChar, 128
            )->Value = tableName;

            cmd->Parameters->Add(
                "@columnName", SqlDbType::NVarChar, 128
            )->Value = columnName;

            Object^ result = cmd->ExecuteScalar();

            if (result == nullptr ||
                result == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Required database column missing: " +
                    tableName + "." + columnName);

            return Convert::ToBoolean(result);
        }

        static int TextCapacity(
            SqlConnection^ con,
            SqlTransaction^ tx,
            String^ tableName,
            String^ columnName)
        {
            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT max_length "
                "FROM sys.columns "
                "WHERE object_id = OBJECT_ID(@tableName) "
                "AND name = @columnName",
                con, tx);

            cmd->Parameters->Add(
                "@tableName", SqlDbType::NVarChar, 128
            )->Value = tableName;

            cmd->Parameters->Add(
                "@columnName", SqlDbType::NVarChar, 128
            )->Value = columnName;

            Object^ result = cmd->ExecuteScalar();

            if (result == nullptr ||
                result == DBNull::Value)
                throw gcnew InvalidOperationException(
                    "Missing column: " + columnName);

            int bytes = Convert::ToInt32(result);

            // nvarchar uses two bytes per UTF-16 code unit.
            return bytes < 0 ? 4000 : bytes / 2;
        }

    public:
        static String^ RecordPayment(
            AuthenticatedUser^ actor,
            int repaymentScheduleID,
            Decimal amount,
            String^ paymentMethod,
            String^ transactionReference,
            String^ notes)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            if (repaymentScheduleID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid repayment installment.");

            if (amount <= Decimal::Zero)
                throw gcnew ArgumentOutOfRangeException(
                    "amount",
                    "Payment amount must be positive.");

            if (amount != Decimal::Round(
                    amount, 2,
                    MidpointRounding::AwayFromZero))
                throw gcnew ArgumentException(
                    "Use no more than two decimal places.");

            paymentMethod = String::IsNullOrWhiteSpace(
                paymentMethod)
                ? ""
                : paymentMethod->Trim();

            if (paymentMethod != "Cash" &&
                paymentMethod != "Bank Transfer" &&
                paymentMethod != "Card")
                throw gcnew ArgumentException(
                    "Payment method must be Cash, "
                    "Bank Transfer or Card.");

            transactionReference =
                String::IsNullOrWhiteSpace(
                    transactionReference)
                    ? nullptr
                    : transactionReference->Trim();

            notes = String::IsNullOrWhiteSpace(notes)
                ? nullptr
                : notes->Trim();

            if (paymentMethod != "Cash" &&
                transactionReference == nullptr)
                throw gcnew ArgumentException(
                    "A transaction reference is required "
                    "for non-cash payments.");

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            SqlTransaction^ tx = nullptr;

            try
            {
                con->Open();

                tx = con->BeginTransaction(
                    IsolationLevel::Serializable);

                // Verify the authenticated account
                // still exists and remains active.
                SqlCommand^ security = gcnew SqlCommand(
                    "SELECT COUNT(*) "
                    "FROM dbo.Users "
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
                        "Your account is not authorized "
                        "to record payments.");

                int methodMax = TextCapacity(
                    con, tx, "dbo.Payments",
                    "PaymentMethod");

                int referenceMax = TextCapacity(
                    con, tx, "dbo.Payments",
                    "TransactionReference");

                int notesMax = TextCapacity(
                    con, tx, "dbo.Payments",
                    "Notes");

                if (paymentMethod->Length > methodMax)
                    throw gcnew ArgumentException(
                        "Payment method exceeds database length.");

                if (transactionReference != nullptr &&
                    transactionReference->Length > referenceMax)
                    throw gcnew ArgumentException(
                        "Transaction reference is too long.");

                if (notes != nullptr &&
                    notes->Length > notesMax)
                    throw gcnew ArgumentException(
                        "Payment notes are too long.");

                // Lock the installment until the payment
                // and paid balances have been saved.
                SqlCommand^ rowQuery = gcnew SqlCommand(
                    "SELECT PrincipalDue, InterestDue, "
                    "PrincipalPaid, InterestPaid "
                    "FROM dbo.RepaymentSchedule "
                    "WITH (UPDLOCK, HOLDLOCK) "
                    "WHERE RepaymentScheduleID=@id",
                    con, tx);

                rowQuery->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = repaymentScheduleID;

                SqlDataReader^ reader =
                    rowQuery->ExecuteReader();

                Decimal principalDue = Decimal::Zero;
                Decimal interestDue = Decimal::Zero;
                Decimal principalPaid = Decimal::Zero;
                Decimal interestPaid = Decimal::Zero;
                bool found = false;

                try
                {
                    if (reader->Read())
                    {
                        found = true;

                        principalDue = Convert::ToDecimal(
                            reader["PrincipalDue"]);

                        interestDue = Convert::ToDecimal(
                            reader["InterestDue"]);

                        principalPaid = Convert::ToDecimal(
                            reader["PrincipalPaid"]);

                        interestPaid = Convert::ToDecimal(
                            reader["InterestPaid"]);
                    }
                }
                finally
                {
                    reader->Close();
                }

                if (!found)
                    throw gcnew InvalidOperationException(
                        "Repayment installment not found.");

                Decimal remainingPrincipal =
                    principalDue - principalPaid;

                Decimal remainingInterest =
                    interestDue - interestPaid;

                if (remainingPrincipal < Decimal::Zero ||
                    remainingInterest < Decimal::Zero)
                    throw gcnew InvalidOperationException(
                        "Stored repayment amounts "
                        "are inconsistent.");

                Decimal outstanding =
                    remainingPrincipal + remainingInterest;

                if (outstanding <= Decimal::Zero)
                    throw gcnew InvalidOperationException(
                        "This installment is already paid.");

                if (amount > outstanding)
                    throw gcnew InvalidOperationException(
                        "Payment exceeds remaining amount. "
                        "Outstanding: Rs. " +
                        outstanding.ToString("N2"));

                // Interest-first allocation.
                Decimal payInterest =
                    amount < remainingInterest
                        ? amount
                        : remainingInterest;

                Decimal payPrincipal =
                    amount - payInterest;

                Decimal newPrincipalPaid =
                    principalPaid + payPrincipal;

                Decimal newInterestPaid =
                    interestPaid + payInterest;

                Decimal newTotalPaid =
                    newPrincipalPaid + newInterestPaid;

                bool fullyPaid =
                    newPrincipalPaid == principalDue &&
                    newInterestPaid == interestDue;

                String^ newStatus = fullyPaid
                    ? "Paid"
                    : "Partially Paid";

                // A bank/card transaction reference must
                // not already be recorded for another payment.
                if (transactionReference != nullptr)
                {
                    SqlCommand^ duplicate =
                        gcnew SqlCommand(
                            "SELECT COUNT(*) "
                            "FROM dbo.Payments "
                            "WITH (UPDLOCK, HOLDLOCK) "
                            "WHERE PaymentMethod=@method "
                            "AND TransactionReference=@reference",
                            con, tx);

                    duplicate->Parameters->Add(
                        "@method", SqlDbType::NVarChar, 100
                    )->Value = paymentMethod;

                    duplicate->Parameters->Add(
                        "@reference", SqlDbType::NVarChar, 300
                    )->Value = transactionReference;

                    if (Convert::ToInt32(
                        duplicate->ExecuteScalar()) > 0)
                        throw gcnew InvalidOperationException(
                            "This transaction reference "
                            "has already been recorded.");
                }

                // Create a short payment reference that
                // fits the actual SQL nvarchar column.
                int paymentRefMax = TextCapacity(
                    con, tx, "dbo.Payments",
                    "PaymentReference");

                if (paymentRefMax < 9)
                    throw gcnew InvalidOperationException(
                        "PaymentReference column is too short.");

                String^ suffix =
                    Guid::NewGuid().ToString("N");

                int suffixLength = Math::Min(
                    16, paymentRefMax - 1);

                String^ paymentReference =
                    "P" + suffix->Substring(0, suffixLength);

                SqlCommand^ referenceCheck =
                    gcnew SqlCommand(
                        "SELECT COUNT(*) "
                        "FROM dbo.Payments "
                        "WITH (UPDLOCK, HOLDLOCK) "
                        "WHERE PaymentReference=@ref",
                        con, tx);

                referenceCheck->Parameters->Add(
                    "@ref", SqlDbType::NVarChar, 100
                )->Value = paymentReference;

                if (Convert::ToInt32(
                    referenceCheck->ExecuteScalar()) > 0)
                    throw gcnew InvalidOperationException(
                        "Payment reference collision. "
                        "Retry the operation.");

                bool amountComputed = ComputedColumn(
                    con, tx, "dbo.Payments",
                    "AmountPaid");

                bool totalComputed = ComputedColumn(
                    con, tx, "dbo.RepaymentSchedule",
                    "TotalPaid");

                DateTime now = DateTime::UtcNow;

                String^ insertColumns =
                    "PaymentReference, "
                    "RepaymentScheduleID, PaymentDate, "
                    "PrincipalAmount, InterestAmount, "
                    "PaymentMethod, TransactionReference, "
                    "RecordedByUserID, Notes, CreatedAt";

                String^ insertValues =
                    "@ref, @id, @date, "
                    "@principal, @interest, "
                    "@method, @transaction, "
                    "@user, @notes, @created";

                if (!amountComputed)
                {
                    insertColumns += ", AmountPaid";
                    insertValues += ", @amount";
                }

                SqlCommand^ insert = gcnew SqlCommand(
                    "INSERT INTO dbo.Payments (" +
                    insertColumns + ") VALUES (" +
                    insertValues + ")",
                    con, tx);

                insert->Parameters->Add(
                    "@ref", SqlDbType::NVarChar, 100
                )->Value = paymentReference;

                insert->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = repaymentScheduleID;

                insert->Parameters->Add(
                    "@date", SqlDbType::DateTime2
                )->Value = now;

                MoneyParameter(
                    insert, "@principal",
                    payPrincipal, 18);

                MoneyParameter(
                    insert, "@interest",
                    payInterest, 18);

                insert->Parameters->Add(
                    "@method", SqlDbType::NVarChar, 100
                )->Value = paymentMethod;

                insert->Parameters->Add(
                    "@transaction", SqlDbType::NVarChar, 300
                )->Value = transactionReference == nullptr
                    ? (Object^)DBNull::Value
                    : (Object^)transactionReference;

                insert->Parameters->Add(
                    "@user", SqlDbType::Int
                )->Value = actor->UserID;

                insert->Parameters->Add(
                    "@notes", SqlDbType::NVarChar, -1
                )->Value = notes == nullptr
                    ? (Object^)DBNull::Value
                    : (Object^)notes;

                insert->Parameters->Add(
                    "@created", SqlDbType::DateTime2
                )->Value = now;

                if (!amountComputed)
                    MoneyParameter(
                        insert, "@amount",
                        amount, 19);

                if (insert->ExecuteNonQuery() != 1)
                    throw gcnew InvalidOperationException(
                        "Payment insert failed.");

                String^ updateSql =
                    "UPDATE dbo.RepaymentSchedule SET "
                    "PrincipalPaid=@principalPaid, "
                    "InterestPaid=@interestPaid, "
                    "PaymentStatus=@status, "
                    "LastPaymentDate=@date";

                if (!totalComputed)
                    updateSql += ", TotalPaid=@totalPaid";

                updateSql +=
                    " WHERE RepaymentScheduleID=@id";

                SqlCommand^ update =
                    gcnew SqlCommand(updateSql, con, tx);

                MoneyParameter(
                    update, "@principalPaid",
                    newPrincipalPaid, 18);

                MoneyParameter(
                    update, "@interestPaid",
                    newInterestPaid, 18);

                if (!totalComputed)
                    MoneyParameter(
                        update, "@totalPaid",
                        newTotalPaid, 19);

                update->Parameters->Add(
                    "@status", SqlDbType::NVarChar, 40
                )->Value = newStatus;

                update->Parameters->Add(
                    "@date", SqlDbType::Date
                )->Value = now.Date;

                update->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = repaymentScheduleID;

                if (update->ExecuteNonQuery() != 1)
                    throw gcnew InvalidOperationException(
                        "Repayment balance update failed.");

                // Payment and installment updates
                // must both succeed or both roll back.
                tx->Commit();
                tx = nullptr;

                return paymentReference;
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
