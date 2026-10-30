#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class AuditLoggingService abstract sealed
    {
    private:
        static Object^ DbText(String^ value, int maximum)
        {
            if (String::IsNullOrEmpty(value))
                return DBNull::Value;

            if (maximum > 0 && value->Length > maximum)
                value = value->Substring(0, maximum);

            return value;
        }

        static void InsertAudit(
            SqlConnection^ connection,
            SqlTransaction^ transaction,
            Nullable<int> userId,
            String^ action,
            String^ entityType,
            String^ entityId,
            String^ description,
            String^ oldValues,
            String^ newValues,
            bool successful,
            String^ errorMessage)
        {
            if (connection == nullptr ||
                connection->State != ConnectionState::Open)
            {
                throw gcnew InvalidOperationException(
                    "An open SQL connection is required.");
            }

            if (String::IsNullOrWhiteSpace(action) ||
                String::IsNullOrWhiteSpace(entityType))
            {
                throw gcnew ArgumentException(
                    "Audit action and entity type are required.");
            }

            SqlCommand^ command = gcnew SqlCommand(
                "INSERT INTO dbo.AuditLogs "
                "(UserID, Action, EntityType, EntityID, "
                "Description, OldValues, NewValues, "
                "IsSuccessful, ErrorMessage, ClientMachine) "
                "VALUES "
                "(@userId, @action, @entityType, @entityId, "
                "@description, @oldValues, @newValues, "
                "@successful, @errorMessage, @clientMachine)",
                connection);

            if (transaction != nullptr)
                command->Transaction = transaction;

            command->Parameters->Add(
                "@userId", SqlDbType::Int)->Value =
                userId.HasValue ?
                safe_cast<Object^>(userId.Value) :
                DBNull::Value;

            command->Parameters->Add(
                "@action", SqlDbType::NVarChar, 60)->Value =
                action->Length > 60 ?
                action->Substring(0, 60) : action;

            command->Parameters->Add(
                "@entityType", SqlDbType::NVarChar, 80)->Value =
                entityType->Length > 80 ?
                entityType->Substring(0, 80) : entityType;

            command->Parameters->Add(
                "@entityId", SqlDbType::NVarChar, 100)->Value =
                DbText(entityId, 100);

            command->Parameters->Add(
                "@description", SqlDbType::NVarChar, 1000)->Value =
                DbText(description, 1000);

            command->Parameters->Add(
                "@oldValues", SqlDbType::NVarChar, -1)->Value =
                DbText(oldValues, -1);

            command->Parameters->Add(
                "@newValues", SqlDbType::NVarChar, -1)->Value =
                DbText(newValues, -1);

            command->Parameters->Add(
                "@successful", SqlDbType::Bit)->Value =
                successful;

            command->Parameters->Add(
                "@errorMessage", SqlDbType::NVarChar, 1000)->Value =
                DbText(errorMessage, 1000);

            command->Parameters->Add(
                "@clientMachine", SqlDbType::NVarChar, 128)->Value =
                DbText(Environment::MachineName, 128);

            command->ExecuteNonQuery();
        }

    public:
        static void Record(
            Nullable<int> userId,
            String^ action,
            String^ entityType,
            String^ entityId,
            String^ description,
            String^ oldValues,
            String^ newValues,
            bool successful,
            String^ errorMessage)
        {
            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                InsertAudit(
                    connection,
                    nullptr,
                    userId,
                    action,
                    entityType,
                    entityId,
                    description,
                    oldValues,
                    newValues,
                    successful,
                    errorMessage);
            }
            finally
            {
                delete connection;
            }
        }

        static void RecordInTransaction(
            SqlConnection^ connection,
            SqlTransaction^ transaction,
            Nullable<int> userId,
            String^ action,
            String^ entityType,
            String^ entityId,
            String^ description,
            String^ oldValues,
            String^ newValues,
            bool successful,
            String^ errorMessage)
        {
            if (transaction == nullptr)
                throw gcnew ArgumentNullException("transaction");

            InsertAudit(
                connection,
                transaction,
                userId,
                action,
                entityType,
                entityId,
                description,
                oldValues,
                newValues,
                successful,
                errorMessage);
        }
    };
}
