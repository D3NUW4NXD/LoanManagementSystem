#pragma once

#using <System.dll>

using namespace System;
using namespace System::Security::Cryptography;

namespace GroupR_LoanManagement
{
    public ref class PasswordHasher abstract sealed
    {
    public:
        literal int SaltSize = 32;
        literal int HashSize = 64;
        literal int Iterations = 210000;

        static array<Byte>^ GenerateSalt()
        {
            array<Byte>^ salt =
                gcnew array<Byte>(SaltSize);

            RandomNumberGenerator^ rng =
                RandomNumberGenerator::Create();

            try
            {
                rng->GetBytes(salt);
            }
            finally
            {
                delete rng;
            }

            return salt;
        }

        static array<Byte>^ HashPassword(
            String^ password,
            array<Byte>^ salt,
            int iterations)
        {
            if (String::IsNullOrEmpty(password))
                throw gcnew ArgumentException(
                    "Password cannot be empty.");

            if (salt == nullptr || salt->Length != SaltSize)
                throw gcnew ArgumentException(
                    "Invalid password salt.");

            if (iterations < 100000)
                throw gcnew ArgumentException(
                    "Iteration count is too low.");

            Rfc2898DeriveBytes^ pbkdf2 =
                gcnew Rfc2898DeriveBytes(
                    password,
                    salt,
                    iterations,
                    HashAlgorithmName::SHA256
                );

            try
            {
                return pbkdf2->GetBytes(HashSize);
            }
            finally
            {
                delete pbkdf2;
            }
        }

        static bool VerifyPassword(
            String^ password,
            array<Byte>^ storedSalt,
            array<Byte>^ storedHash,
            int iterations)
        {
            if (String::IsNullOrEmpty(password) ||
                storedSalt == nullptr ||
                storedHash == nullptr ||
                storedSalt->Length != SaltSize ||
                storedHash->Length != HashSize ||
                iterations < 100000)
            {
                return false;
            }

            array<Byte>^ computedHash =
                HashPassword(
                    password,
                    storedSalt,
                    iterations
                );

            int difference = 0;

            for (int i = 0; i < HashSize; i++)
            {
                difference |=
                    computedHash[i] ^ storedHash[i];
            }

            return difference == 0;
        }
    };
}
