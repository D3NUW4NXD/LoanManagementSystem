#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "GuarantorProfileService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class GuarantorProfileForm : public Form
    {
    // GROUPR_PHASE94_V2
protected:
    virtual void OnShown(System::EventArgs^ e) override
    {
        System::Windows::Forms::Form::OnShown(e);
        GroupRModernTheme::Apply(this);
    }

    private:
        GuarantorProfileRecord^ profile;

        void Section(String^ title, int y)
        {
            Label^ label = gcnew Label();
            label->Text = title;
            label->Location = Point(35, y);
            label->Size = Drawing::Size(820, 30);
            label->ForeColor =
                Color::FromArgb(74, 222, 128);
            label->Font = gcnew Drawing::Font(
                "Segoe UI", 13.0f, FontStyle::Bold);

            this->Controls->Add(label);
        }

        void Detail(
            String^ title,
            String^ value,
            int x,
            int y,
            int width)
        {
            Label^ caption = gcnew Label();
            caption->Text = title;
            caption->Location = Point(x, y);
            caption->Size = Drawing::Size(width, 23);
            caption->ForeColor =
                Color::FromArgb(155, 181, 162);
            caption->Font = gcnew Drawing::Font(
                "Segoe UI", 9.0f);

            this->Controls->Add(caption);

            TextBox^ field = gcnew TextBox();
            field->Text = String::IsNullOrWhiteSpace(value)
                ? "Not provided" : value;
            field->Location = Point(x, y + 25);
            field->Size = Drawing::Size(width, 29);
            field->Font = gcnew Drawing::Font(
                "Segoe UI", 10.5f, FontStyle::Bold);
            field->BackColor =
                Color::FromArgb(30, 45, 37);
            field->ForeColor = Color::White;
            field->BorderStyle = BorderStyle::None;
            field->ReadOnly = true;
            field->TabStop = false;

            this->Controls->Add(field);
        }

    public:
        GuarantorProfileForm(
            AuthenticatedUser^ actor,
            int guarantorID)
        {
            profile = GuarantorProfileService::GetProfile(
                actor, guarantorID);

            GuarantorEditRecord^ g = profile->Details;

            this->Text = "Group R - Guarantor Profile";
            this->ClientSize = Drawing::Size(900, 740);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor =
                Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;
            this->AutoScroll = true;
            this->AutoScrollMinSize =
                Drawing::Size(875, 1050);
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;

            Label^ heading = gcnew Label();
            heading->Text = "GUARANTOR PROFILE";
            heading->Location = Point(35, 20);
            heading->Size = Drawing::Size(820, 46);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 23.0f, FontStyle::Bold);

            this->Controls->Add(heading);

            Detail(
                "GUARANTOR CODE",
                g->GuarantorCode,
                35, 85, 400);

            Detail(
                "GUARANTOR ID",
                g->GuarantorID.ToString(),
                470, 85, 390);

            Section("PERSONAL INFORMATION", 175);

            Detail("FULL NAME", g->FullName, 35, 220, 400);
            Detail("NIC", g->NIC, 470, 220, 390);
            Detail("PHONE", g->Phone, 35, 290, 400);
            Detail("EMAIL", g->Email, 470, 290, 390);
            Detail("ADDRESS", g->Address, 35, 360, 825);

            Section("APPLICANT ASSOCIATION", 450);

            Detail(
                "LINKED APPLICANT",
                profile->ApplicantName,
                35, 495, 400);

            Detail(
                "APPLICANT CODE",
                profile->ApplicantCode,
                470, 495, 390);

            Detail(
                "RELATIONSHIP TO APPLICANT",
                g->RelationshipToApplicant,
                35, 565, 825);

            Section("EMPLOYMENT AND FINANCIAL DETAILS", 660);

            Detail(
                "OCCUPATION",
                g->Occupation,
                35, 705, 400);

            Detail(
                "MONTHLY INCOME",
                "LKR " + g->MonthlyIncome.ToString(
                    "N2", CultureInfo::InvariantCulture),
                470, 705, 390);

            Section("RECORD INFORMATION", 800);

            Detail(
                "CREATED BY",
                profile->CreatedBy,
                35, 845, 400);

            Detail(
                "REGISTRATION DATE",
                profile->CreatedAt.ToString(
                    "dd MMM yyyy HH:mm",
                    CultureInfo::InvariantCulture) + " UTC",
                470, 845, 390);

            Button^ closeButton = gcnew Button();
            closeButton->Text = "CLOSE PROFILE";
            closeButton->Location = Point(655, 945);
            closeButton->Size = Drawing::Size(205, 43);
            closeButton->BackColor =
                Color::FromArgb(22, 163, 74);
            closeButton->ForeColor = Color::White;
            closeButton->FlatStyle = FlatStyle::Flat;
            closeButton->FlatAppearance->BorderSize = 0;
            closeButton->DialogResult =
                System::Windows::Forms::DialogResult::OK;

            this->Controls->Add(closeButton);
            this->CancelButton = closeButton;
        }
    };
}
