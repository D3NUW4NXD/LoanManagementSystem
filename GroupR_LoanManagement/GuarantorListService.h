#pragma once

#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class GuarantorListService abstract sealed
    {
    public:
        static DataTable^ GetAllGuarantors(
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
            {
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");
            }

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();

                SqlCommand^ auth = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM dbo.Users "
                    "WHERE UserID=@ID AND Username=@Username "
                    "AND IsActive=1 AND RoleID IN (1,2,3)",
                    connection);

                auth->Parameters->Add(
                    "@ID", SqlDbType::Int
                )->Value = actor->UserID;

                auth->Parameters->Add(
                    "@Username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                bool allowed =
                    Convert::ToInt32(auth->ExecuteScalar()) == 1;

                delete auth;

                if (!allowed)
                    throw gcnew UnauthorizedAccessException(
                        "Guarantor Management access denied.");

                DataTable^ result = gcnew DataTable();

                // Schema-independent until we inspect the
                // actual Guarantors table columns.
                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(
                        "SELECT * FROM dbo.Guarantors",
                        connection);

                try
                {
                    adapter->Fill(result);
                }
                finally
                {
                    delete adapter;
                }

                return result;
            }
            finally
            {
                delete connection;
            }
        }
    };
}
