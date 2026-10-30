#pragma once

#using <System.dll>
#using <System.Data.dll>

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class DatabaseConnection abstract sealed
    {
    private:
        static String^ ConnectionString()
        {
            return
                "Data Source=(localdb)\\MSSQLLocalDB;"
                "Initial Catalog=LoanManagementDB;"
                "Integrated Security=True;"
                "TrustServerCertificate=True;";
        }

    public:
        static SqlConnection^ CreateConnection()
        {
            return gcnew SqlConnection(ConnectionString());
        }

        static bool TestConnection()
        {
            SqlConnection^ connection = CreateConnection();

            try
            {
                connection->Open();
                return connection->State == ConnectionState::Open;
            }
            finally
            {
                delete connection;
            }
        }

        static int GetTableCount()
        {
            SqlConnection^ connection = CreateConnection();

            try
            {
                connection->Open();

                SqlCommand^ command = gcnew SqlCommand(
                    "SELECT COUNT(*) FROM sys.tables "
                    "WHERE schema_id = SCHEMA_ID('dbo')",
                    connection
                );

                try
                {
                    return Convert::ToInt32(
                        command->ExecuteScalar()
                    );
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
