#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "DatabaseConnection.h"
#include "PasswordHasher.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class AuthenticatedUser sealed
    {
    public:
        property int UserID;
        property String^ Username;
        property String^ FullName;
        property int RoleID;
        property String^ RoleName;
    };

    public ref class AuthenticationService abstract sealed
    {
    public:
        static AuthenticatedUser^ Login(
            String^ username,
            String^ password)
        {
            if (String::IsNullOrWhiteSpace(username) ||
                String::IsNullOrEmpty(password))
            {
                return nullptr;
            }

            username = username->Trim();

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                SqlCommand^ command = gcnew SqlCommand(
                    "SELECT u.UserID, u.Username, u.FullName, "
                    "u.PasswordHash, u.PasswordSalt, "
                    "u.PasswordIterations, u.RoleID, "
                    "u.IsActive, r.RoleName "
                    "FROM dbo.Users u "
                    "INNER JOIN dbo.Roles r "
                    "ON u.RoleID = r.RoleID "
                    "WHERE u.Username = @Username",
                    connection
                );

                try
                {
                    command->Parameters->Add(
                        "@Username",
                        SqlDbType::NVarChar,
                        100
                    )->Value = username;

                    SqlDataReader^ reader =
                        command->ExecuteReader();

                    AuthenticatedUser^ user = nullptr;
                    bool valid = false;

                    try
                    {
                        if (reader->Read())
                        {
                            bool active =
                                Convert::ToBoolean(
                                    reader["IsActive"]
                                );

                            array<Byte>^ storedHash =
                                safe_cast<array<Byte>^>(
                                    reader["PasswordHash"]
                                );

                            array<Byte>^ storedSalt =
                                safe_cast<array<Byte>^>(
                                    reader["PasswordSalt"]
                                );

                            int iterations =
                                Convert::ToInt32(
                                    reader["PasswordIterations"]
                                );

                            valid =
                                PasswordHasher::VerifyPassword(
                                    password,
                                    storedSalt,
                                    storedHash,
                                    iterations
                                );

                            if (valid && active)
                            {
                                user = gcnew AuthenticatedUser();

                                user->UserID =
                                    Convert::ToInt32(
                                        reader["UserID"]
                                    );

                                user->Username =
                                    Convert::ToString(
                                        reader["Username"]
                                    );

                                user->FullName =
                                    Convert::ToString(
                                        reader["FullName"]
                                    );

                                user->RoleID =
                                    Convert::ToInt32(
                                        reader["RoleID"]
                                    );

                                user->RoleName =
                                    Convert::ToString(
                                        reader["RoleName"]
                                    );
                            }
                        }
                    }
                    finally
                    {
                        delete reader;
                    }

                    if (user != nullptr)
                    {
                        SqlCommand^ update = gcnew SqlCommand(
                            "UPDATE dbo.Users "
                            "SET LastLoginAt = SYSUTCDATETIME() "
                            "WHERE UserID = @UserID AND IsActive = 1",
                            connection
                        );

                        try
                        {
                            update->Parameters->Add(
                                "@UserID",
                                SqlDbType::Int
                            )->Value = user->UserID;

                            if (update->ExecuteNonQuery() != 1)
                                return nullptr;
                        }
                        finally
                        {
                            delete update;
                        }
                    }

                    return user;
                }
                finally
                {
                    delete command;
                }
            }
            finally
            {
                delete connection;
            }
        }
    };
}
