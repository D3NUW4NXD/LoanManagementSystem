#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "AuthenticationService.h"
#include "PasswordHasher.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class UserCreationService abstract sealed
    {
    public:
        static int CreateUser(
            AuthenticatedUser^ actor,
            String^ username,
            String^ fullName,
            String^ email,
            String^ password,
            int roleID)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Authentication required."
                );
            }

            if (roleID != 2 && roleID != 3)
            {
                throw gcnew ArgumentException(
                    "Only Administrator and Staff roles can be created."
                );
            }

            if (String::IsNullOrWhiteSpace(username) ||
                String::IsNullOrWhiteSpace(fullName))
            {
                throw gcnew ArgumentException(
                    "Username and full name are required."
                );
            }

            username = username->Trim();
            fullName = fullName->Trim();

            if (username->Length > 100 ||
                fullName->Length > 150)
            {
                throw gcnew ArgumentException(
                    "Username or full name is too long."
                );
            }

            if (String::IsNullOrEmpty(password) ||
                password->Length < 16)
            {
                throw gcnew ArgumentException(
                    "Password must have at least 16 characters."
                );
            }

            if (email == nullptr)
                email = String::Empty;

            email = email->Trim();

            if (email->Length > 254)
            {
                throw gcnew ArgumentException(
                    "Email address is too long."
                );
            }

            array<Byte>^ salt =
                PasswordHasher::GenerateSalt();

            array<Byte>^ hash =
                PasswordHasher::HashPassword(
                    password,
                    salt,
                    PasswordHasher::Iterations
                );

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                SqlTransaction^ transaction =
                    connection->BeginTransaction(
                        IsolationLevel::Serializable
                    );

                try
                {
                    // Recheck Super Administrator privileges
                    // inside the same database transaction.
                    SqlCommand^ authCommand =
                        gcnew SqlCommand(
                            "SELECT COUNT(*) FROM dbo.Users "
                            "WHERE UserID = @UserID "
                            "AND Username = @ActorUsername "
                            "AND RoleID = 1 AND IsActive = 1",
                            connection,
                            transaction
                        );

                    try
                    {
                        authCommand->Parameters->Add(
                            "@UserID", SqlDbType::Int
                        )->Value = actor->UserID;

                        authCommand->Parameters->Add(
                            "@ActorUsername",
                            SqlDbType::NVarChar, 100
                        )->Value = actor->Username;

                        if (Convert::ToInt32(
                            authCommand->ExecuteScalar()
                        ) != 1)
                        {
                            throw gcnew UnauthorizedAccessException(
                                "Only an active Super Administrator "
                                "can create accounts."
                            );
                        }
                    }
                    finally
                    {
                        delete authCommand;
                    }

                    SqlCommand^ checkCommand =
                        gcnew SqlCommand(
                            "SELECT COUNT(*) FROM dbo.Users "
                            "WITH (UPDLOCK, HOLDLOCK) "
                            "WHERE Username = @Username",
                            connection,
                            transaction
                        );

                    try
                    {
                        checkCommand->Parameters->Add(
                            "@Username",
                            SqlDbType::NVarChar, 100
                        )->Value = username;

                        if (Convert::ToInt32(
                            checkCommand->ExecuteScalar()
                        ) > 0)
                        {
                            throw gcnew InvalidOperationException(
                                "Username already exists."
                            );
                        }
                    }
                    finally
                    {
                        delete checkCommand;
                    }

                    SqlCommand^ insertCommand =
                        gcnew SqlCommand(
                            "INSERT INTO dbo.Users "
                            "(Username, FullName, Email, "
                            "PasswordHash, PasswordSalt, "
                            "PasswordIterations, RoleID, "
                            "IsActive, CreatedAt) "
                            "OUTPUT INSERTED.UserID "
                            "VALUES "
                            "(@Username, @FullName, @Email, "
                            "@Hash, @Salt, @Iterations, "
                            "@RoleID, 1, SYSUTCDATETIME())",
                            connection,
                            transaction
                        );

                    int newUserID;

                    try
                    {
                        insertCommand->Parameters->Add(
                            "@Username",
                            SqlDbType::NVarChar, 100
                        )->Value = username;

                        insertCommand->Parameters->Add(
                            "@FullName",
                            SqlDbType::NVarChar, 150
                        )->Value = fullName;

                        insertCommand->Parameters->Add(
                            "@Email",
                            SqlDbType::NVarChar, 254
                        )->Value =
                            String::IsNullOrWhiteSpace(email)
                            ? (Object^)DBNull::Value
                            : (Object^)email;

                        insertCommand->Parameters->Add(
                            "@Hash",
                            SqlDbType::VarBinary, 64
                        )->Value = hash;

                        insertCommand->Parameters->Add(
                            "@Salt",
                            SqlDbType::VarBinary, 32
                        )->Value = salt;

                        insertCommand->Parameters->Add(
                            "@Iterations",
                            SqlDbType::Int
                        )->Value = PasswordHasher::Iterations;

                        insertCommand->Parameters->Add(
                            "@RoleID",
                            SqlDbType::TinyInt
                        )->Value = (Byte)roleID;

                        newUserID = Convert::ToInt32(
                            insertCommand->ExecuteScalar()
                        );
                    }
                    finally
                    {
                        delete insertCommand;
                    }

                    transaction->Commit();

                    return newUserID;
                }
                catch (Exception^)
                {
                    try
                    {
                        transaction->Rollback();
                    }
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
                Array::Clear(salt, 0, salt->Length);
                Array::Clear(hash, 0, hash->Length);
            }
        }
    };
}
