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
    public ref class ApplicantService abstract sealed
    {
    public:
        static DataTable^ GetAllApplicants(
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");
            }

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                // Check the user's CURRENT database status.
                SqlCommand^ authorization = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@ID AND Username=@Username "
                    "AND IsActive=1 AND RoleID IN (1,2,3)",
                    connection);

                try
                {
                    authorization->Parameters->Add(
                        "@ID", SqlDbType::Int
                    )->Value = actor->UserID;

                    authorization->Parameters->Add(
                        "@Username", SqlDbType::NVarChar, 100
                    )->Value = actor->Username;

                    if (Convert::ToInt32(
                        authorization->ExecuteScalar()) != 1)
                    {
                        throw gcnew UnauthorizedAccessException(
                            "Applicant Management access denied.");
                    }
                }
                finally
                {
                    delete authorization;
                }

                // Initial read-only listing.
                // Uses the existing Applicants table without
                // assuming column names or changing its schema.
                DataTable^ applicants = gcnew DataTable();

                SqlDataAdapter^ adapter = gcnew SqlDataAdapter(
                    "SELECT * FROM dbo.Applicants",
                    connection);

                try
                {
                    adapter->Fill(applicants);
                }
                finally
                {
                    delete adapter;
                }

                return applicants;
            }
            finally
            {
                delete connection;
            }
        }
    };
}
