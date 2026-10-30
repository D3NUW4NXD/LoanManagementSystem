#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "DatabaseConnection.h"
#include "AuthenticationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Collections::Generic;

namespace GroupR_LoanManagement
{
    public ref class UserSummary sealed
    {
    public:
        property int UserID;
        property String^ Username;
        property String^ FullName;
        property String^ RoleName;
        property bool IsActive;
    };

    public ref class UserManagementService abstract sealed
    {
    private:
        static bool CanManageUsers(
            SqlConnection^ connection,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrEmpty(actor->Username))
            {
                return false;
            }

            SqlCommand^ command = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID = @UserID "
                "AND Username = @Username "
                "AND RoleID = 1 "
                "AND IsActive = 1",
                connection
            );

            try
            {
                command->Parameters->Add(
                    "@UserID",
                    SqlDbType::Int
                )->Value = actor->UserID;

                command->Parameters->Add(
                    "@Username",
                    SqlDbType::NVarChar,
                    100
                )->Value = actor->Username;

                return Convert::ToInt32(
                    command->ExecuteScalar()
                ) == 1;
            }
            finally
            {
                delete command;
            }
        }

    public:
        static List<UserSummary^>^ GetAllUsers(
            AuthenticatedUser^ actor)
        {
            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                // Check authorization against the database.
                if (!CanManageUsers(connection, actor))
                {
                    throw gcnew UnauthorizedAccessException(
                        "Only an active Super Administrator "
                        "can manage users."
                    );
                }

                List<UserSummary^>^ users =
                    gcnew List<UserSummary^>();

                SqlCommand^ command = gcnew SqlCommand(
                    "SELECT u.UserID, u.Username, "
                    "u.FullName, r.RoleName, u.IsActive "
                    "FROM dbo.Users u "
                    "INNER JOIN dbo.Roles r "
                    "ON u.RoleID = r.RoleID "
                    "ORDER BY u.UserID",
                    connection
                );

                try
                {
                    SqlDataReader^ reader =
                        command->ExecuteReader();

                    try
                    {
                        while (reader->Read())
                        {
                            UserSummary^ user =
                                gcnew UserSummary();

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

                            user->RoleName =
                                Convert::ToString(
                                    reader["RoleName"]
                                );

                            user->IsActive =
                                Convert::ToBoolean(
                                    reader["IsActive"]
                                );

                            users->Add(user);
                        }
                    }
                    finally
                    {
                        delete reader;
                    }
                }
                finally
                {
                    delete command;
                }

                return users;
            }
            finally
            {
                delete connection;
            }
        }
    };
}
