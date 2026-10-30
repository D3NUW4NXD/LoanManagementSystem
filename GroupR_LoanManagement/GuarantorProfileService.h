#pragma once

#using <System.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "GuarantorEditService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace GroupR_LoanManagement
{
    public ref class GuarantorProfileRecord
    {
    public:
        property GuarantorEditRecord^ Details;
        property String^ ApplicantName;
        property String^ ApplicantCode;
        property String^ CreatedBy;
        property DateTime CreatedAt;
    };

    public ref class GuarantorProfileService abstract sealed
    {
    public:
        static GuarantorProfileRecord^ GetProfile(
            AuthenticatedUser^ actor,
            int guarantorID)
        {
            if (actor == nullptr ||
                actor->UserID <= 0 ||
                String::IsNullOrWhiteSpace(actor->Username))
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            if (guarantorID <= 0)
                throw gcnew ArgumentException(
                    "Invalid guarantor ID.");

            // Reuse the existing authorization and record loader.
            GuarantorEditRecord^ details =
                GuarantorEditService::GetGuarantor(
                    actor, guarantorID);

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT g.GuarantorID, "
                    "a.FullName AS ApplicantName, "
                    "a.ApplicantCode, "
                    "u.Username AS CreatedBy, "
                    "g.CreatedAt "
                    "FROM dbo.Guarantors g "
                    "INNER JOIN dbo.Applicants a "
                    "ON a.ApplicantID = g.ApplicantID "
                    "LEFT JOIN dbo.Users u "
                    "ON u.UserID = g.CreatedByUserID "
                    "WHERE g.GuarantorID = @id "
                    "AND EXISTS ( "
                    " SELECT 1 FROM dbo.Users currentUser "
                    " WHERE currentUser.UserID = @userID "
                    " AND currentUser.Username = @username "
                    " AND currentUser.IsActive = 1 "
                    " AND currentUser.RoleID IN (1,2,3))",
                    con);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = guarantorID;

                cmd->Parameters->Add(
                    "@userID", SqlDbType::Int
                )->Value = actor->UserID;

                cmd->Parameters->Add(
                    "@username", SqlDbType::NVarChar, 256
                )->Value = actor->Username;

                SqlDataReader^ reader = cmd->ExecuteReader();

                try
                {
                    if (!reader->Read())
                        throw gcnew InvalidOperationException(
                            "Guarantor profile not available.");

                    GuarantorProfileRecord^ profile =
                        gcnew GuarantorProfileRecord();

                    profile->Details = details;

                    profile->ApplicantName =
                        reader->IsDBNull(1)
                        ? "Not available"
                        : reader->GetString(1);

                    profile->ApplicantCode =
                        reader->IsDBNull(2)
                        ? "Not available"
                        : reader->GetString(2);

                    profile->CreatedBy =
                        reader->IsDBNull(3)
                        ? "Unknown"
                        : reader->GetString(3);

                    profile->CreatedAt =
                        reader->GetDateTime(4);

                    return profile;
                }
                finally
                {
                    delete reader;
                }
            }
            finally
            {
                delete con;
            }
        }
    };
}
