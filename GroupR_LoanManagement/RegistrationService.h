#pragma once

#using <System.dll>
#using <System.Data.dll>

#include "DatabaseConnection.h"
#include "PasswordHasher.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Text::RegularExpressions;

namespace GroupR_LoanManagement
{
    public ref class RegistrationService abstract sealed
    {
    public:
        static void Register(
            String^ username,
            String^ fullName,
            String^ email,
            String^ password)
        {
            if (String::IsNullOrWhiteSpace(username) ||
                String::IsNullOrWhiteSpace(fullName) ||
                String::IsNullOrWhiteSpace(email) ||
                String::IsNullOrEmpty(password))
            {
                throw gcnew ArgumentException("Complete all required fields.");
            }

            username = username->Trim();
            fullName = fullName->Trim();
            email = email->Trim();

            if (!Regex::IsMatch(username, "^[A-Za-z0-9._-]{3,40}$"))
                throw gcnew ArgumentException(
                    "Username must have 3-40 letters, numbers, dots, hyphens or underscores.");

            if (fullName->Length > 150 ||
                email->Length > 254 ||
                !Regex::IsMatch(email, "^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$"))
                throw gcnew ArgumentException("Enter a valid name and email address.");

            if (password->Length < 16 || password->Length > 128)
                throw gcnew ArgumentException(
                    "Password must contain 16-128 characters.");

            array<Byte>^ salt = PasswordHasher::GenerateSalt();
            array<Byte>^ hash = nullptr;

            try
            {
                hash = PasswordHasher::HashPassword(
                    password, salt, PasswordHasher::Iterations);

                SqlConnection^ con = DatabaseConnection::CreateConnection();

                try
                {
                    con->Open();

                    SqlTransaction^ tx =
                        con->BeginTransaction(IsolationLevel::Serializable);

                    try
                    {
                        SqlCommand^ check = gcnew SqlCommand(
                            "SELECT COUNT(*) FROM dbo.Users WITH (UPDLOCK,HOLDLOCK) "
                            "WHERE Username=@Username OR Email=@Email",
                            con, tx);

                        check->Parameters->Add("@Username", SqlDbType::NVarChar, 100)->Value = username;
                        check->Parameters->Add("@Email", SqlDbType::NVarChar, 254)->Value = email;

                        if (Convert::ToInt32(check->ExecuteScalar()) > 0)
                            throw gcnew InvalidOperationException(
                                "Username or email is already registered.");

                        SqlCommand^ insert = gcnew SqlCommand(
                            "INSERT INTO dbo.Users "
                            "(Username,FullName,Email,PasswordHash,PasswordSalt,"
                            "PasswordIterations,RoleID,IsActive,CreatedAt) "
                            "VALUES (@Username,@FullName,@Email,@Hash,@Salt,"
                            "@Iterations,3,0,SYSUTCDATETIME())",
                            con, tx);

                        insert->Parameters->Add("@Username", SqlDbType::NVarChar, 100)->Value = username;
                        insert->Parameters->Add("@FullName", SqlDbType::NVarChar, 150)->Value = fullName;
                        insert->Parameters->Add("@Email", SqlDbType::NVarChar, 254)->Value = email;
                        insert->Parameters->Add("@Hash", SqlDbType::VarBinary, 64)->Value = hash;
                        insert->Parameters->Add("@Salt", SqlDbType::VarBinary, 32)->Value = salt;
                        insert->Parameters->Add("@Iterations", SqlDbType::Int)->Value = PasswordHasher::Iterations;

                        if (insert->ExecuteNonQuery() != 1)
                            throw gcnew InvalidOperationException("Registration could not be saved.");

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
                    delete con;
                }
            }
            finally
            {
                if (salt != nullptr) Array::Clear(salt, 0, salt->Length);
                if (hash != nullptr) Array::Clear(hash, 0, hash->Length);
            }
        }
    };
}
