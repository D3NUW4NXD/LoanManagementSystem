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
    public ref class UserEditRecord sealed
    {
    public:
        property int UserID;
        property String^ Username;
        property String^ FullName;
        property String^ Email;
        property String^ RoleName;
        property bool IsActive;
    };

    public ref class UserAdminEditService abstract sealed
    {
    private:
        static void RequireSuperAdmin(
            SqlConnection^ connection,
            SqlTransaction^ transaction,
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr || actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");
            }

            SqlCommand^ cmd = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@ID AND Username=@Name "
                "AND RoleID=1 AND IsActive=1",
                connection, transaction);

            try
            {
                cmd->Parameters->Add(
                    "@ID", SqlDbType::Int)->Value = actor->UserID;

                cmd->Parameters->Add(
                    "@Name", SqlDbType::NVarChar, 100
                )->Value = actor->Username;

                if (Convert::ToInt32(cmd->ExecuteScalar()) != 1)
                    throw gcnew UnauthorizedAccessException(
                        "Super Administrator access required.");
            }
            finally
            {
                delete cmd;
            }
        }

    public:
        static UserEditRecord^ GetUser(
            AuthenticatedUser^ actor, int userID)
        {
            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();
                RequireSuperAdmin(connection, nullptr, actor);

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT u.UserID,u.Username,u.FullName,"
                    "u.Email,r.RoleName,u.IsActive "
                    "FROM dbo.Users u "
                    "INNER JOIN dbo.Roles r ON u.RoleID=r.RoleID "
                    "WHERE u.UserID=@ID AND u.RoleID IN (2,3)",
                    connection);

                try
                {
                    cmd->Parameters->Add(
                        "@ID", SqlDbType::Int)->Value = userID;

                    SqlDataReader^ reader = cmd->ExecuteReader();

                    try
                    {
                        if (!reader->Read())
                            throw gcnew InvalidOperationException(
                                "Only Administrator and Staff accounts "
                                "can be edited.");

                        UserEditRecord^ result =
                            gcnew UserEditRecord();

                        result->UserID =
                            Convert::ToInt32(reader["UserID"]);
                        result->Username =
                            Convert::ToString(reader["Username"]);
                        result->FullName =
                            Convert::ToString(reader["FullName"]);
                        result->Email =
                            reader->IsDBNull(reader->GetOrdinal("Email"))
                            ? String::Empty
                            : Convert::ToString(reader["Email"]);
                        result->RoleName =
                            Convert::ToString(reader["RoleName"]);
                        result->IsActive =
                            Convert::ToBoolean(reader["IsActive"]);

                        return result;
                    }
                    finally
                    {
                        delete reader;
                    }
                }
                finally
                {
                    delete cmd;
                }
            }
            finally
            {
                delete connection;
            }
        }

        static void UpdateDetails(
            AuthenticatedUser^ actor,
            int userID,
            String^ fullName,
            String^ email)
        {
            if (userID <= 0 ||
                String::IsNullOrWhiteSpace(fullName))
                throw gcnew ArgumentException(
                    "A valid user and full name are required.");

            fullName = fullName->Trim();
            email = email == nullptr ? String::Empty : email->Trim();

            if (fullName->Length > 150 || email->Length > 254)
                throw gcnew ArgumentException(
                    "Full name or email is too long.");

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                SqlTransaction^ tx =
                    connection->BeginTransaction(
                        IsolationLevel::Serializable);

                try
                {
                    RequireSuperAdmin(connection, tx, actor);

                    SqlCommand^ cmd = gcnew SqlCommand(
                        "UPDATE dbo.Users "
                        "SET FullName=@FullName,Email=@Email "
                        "WHERE UserID=@ID AND RoleID IN (2,3)",
                        connection, tx);

                    try
                    {
                        cmd->Parameters->Add(
                            "@ID", SqlDbType::Int)->Value = userID;

                        cmd->Parameters->Add(
                            "@FullName", SqlDbType::NVarChar, 150
                        )->Value = fullName;

                        cmd->Parameters->Add(
                            "@Email", SqlDbType::NVarChar, 254
                        )->Value =
                            String::IsNullOrWhiteSpace(email)
                            ? (Object^)DBNull::Value
                            : (Object^)email;

                        if (cmd->ExecuteNonQuery() != 1)
                            throw gcnew InvalidOperationException(
                                "Account not found or protected.");
                    }
                    finally
                    {
                        delete cmd;
                    }

                    tx->Commit();
                }
                catch (Exception^)
                {
                    try { tx->Rollback(); }
                    catch (Exception^) {}
                    throw;
                }
                finally
                {
                    delete tx;
                }
            }
            finally
            {
                delete connection;
            }
        }

        static void SetActive(
            AuthenticatedUser^ actor,
            int userID,
            bool active)
        {
            if (userID <= 0)
                throw gcnew ArgumentException("Invalid user ID.");

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                SqlTransaction^ tx =
                    connection->BeginTransaction(
                        IsolationLevel::Serializable);

                try
                {
                    RequireSuperAdmin(connection, tx, actor);

                    SqlCommand^ cmd = gcnew SqlCommand(
                        "UPDATE dbo.Users SET IsActive=@Active "
                        "WHERE UserID=@ID AND RoleID IN (2,3)",
                        connection, tx);

                    try
                    {
                        cmd->Parameters->Add(
                            "@Active", SqlDbType::Bit)->Value = active;

                        cmd->Parameters->Add(
                            "@ID", SqlDbType::Int)->Value = userID;

                        if (cmd->ExecuteNonQuery() != 1)
                            throw gcnew InvalidOperationException(
                                "Account not found or protected.");
                    }
                    finally
                    {
                        delete cmd;
                    }

                    tx->Commit();
                }
                catch (Exception^)
                {
                    try { tx->Rollback(); }
                    catch (Exception^) {}
                    throw;
                }
                finally
                {
                    delete tx;
                }
            }
            finally
            {
                delete connection;
            }
        }
    };
}
