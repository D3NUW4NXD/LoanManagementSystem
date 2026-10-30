#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"
#include "PasswordHasher.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class PasswordResetService abstract sealed
    {
    public:
        static void ResetPassword(
            AuthenticatedUser^ actor,
            int targetUserID,
            String^ newPassword)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Authentication is required.");
            }

            if (targetUserID <= 0)
                throw gcnew ArgumentException(
                    "Invalid target account.");

            if (String::IsNullOrEmpty(newPassword) ||
                newPassword->Length < 16 ||
                newPassword->Length > 128)
            {
                throw gcnew ArgumentException(
                    "Password must contain 16 to 128 characters.");
            }

            array<Byte>^ salt = nullptr;
            array<Byte>^ hash = nullptr;

            try
            {
                salt = PasswordHasher::GenerateSalt();

                hash = PasswordHasher::HashPassword(
                    newPassword,
                    salt,
                    PasswordHasher::Iterations);

                SqlConnection^ connection =
                    DatabaseConnection::CreateConnection();

                try
                {
                    connection->Open();

                    SqlTransaction^ transaction =
                        connection->BeginTransaction(
                            IsolationLevel::Serializable);

                    try
                    {
                        // Verify Super Administrator access in SQL.
                        SqlCommand^ authorize = gcnew SqlCommand(
                            "SELECT COUNT(*) FROM dbo.Users "
                            "WHERE UserID=@ActorID "
                            "AND Username=@ActorName "
                            "AND RoleID=1 AND IsActive=1",
                            connection,
                            transaction);

                        try
                        {
                            authorize->Parameters->Add(
                                "@ActorID",
                                SqlDbType::Int
                            )->Value = actor->UserID;

                            authorize->Parameters->Add(
                                "@ActorName",
                                SqlDbType::NVarChar, 100
                            )->Value = actor->Username;

                            if (Convert::ToInt32(
                                authorize->ExecuteScalar()) != 1)
                            {
                                throw gcnew UnauthorizedAccessException(
                                    "Only an active Super Administrator "
                                    "may reset user passwords.");
                            }
                        }
                        finally
                        {
                            delete authorize;
                        }

                        // Protect the Super Administrator account.
                        SqlCommand^ update = gcnew SqlCommand(
                            "UPDATE dbo.Users "
                            "SET PasswordHash=@Hash, "
                            "PasswordSalt=@Salt, "
                            "PasswordIterations=@Iterations "
                            "WHERE UserID=@TargetID "
                            "AND RoleID IN (2,3)",
                            connection,
                            transaction);

                        try
                        {
                            update->Parameters->Add(
                                "@Hash",
                                SqlDbType::VarBinary, 64
                            )->Value = hash;

                            update->Parameters->Add(
                                "@Salt",
                                SqlDbType::VarBinary, 32
                            )->Value = salt;

                            update->Parameters->Add(
                                "@Iterations",
                                SqlDbType::Int
                            )->Value =
                                PasswordHasher::Iterations;

                            update->Parameters->Add(
                                "@TargetID",
                                SqlDbType::Int
                            )->Value = targetUserID;

                            if (update->ExecuteNonQuery() != 1)
                            {
                                throw gcnew InvalidOperationException(
                                    "Account not found or protected.");
                            }
                        }
                        finally
                        {
                            delete update;
                        }

                        transaction->Commit();
                    }
                    catch (Exception^)
                    {
                        try { transaction->Rollback(); }
                        catch (Exception^) {}
                        throw;
                    }
                    finally
                    {
                        delete transaction;
                    }
                }
                finally
                {
                    delete connection;
                }
            }
            finally
            {
                if (hash != nullptr)
                    Array::Clear(hash, 0, hash->Length);

                if (salt != nullptr)
                    Array::Clear(salt, 0, salt->Length);
            }
        }
    };
}
