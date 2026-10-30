#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "ApplicantEditService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class ApplicantProfileForm : public Form
    {
    // GROUPR_PHASE94_V2
protected:
    virtual void OnShown(System::EventArgs^ e) override
    {
        System::Windows::Forms::Form::OnShown(e);
        GroupRModernTheme::Apply(this);
    }

    private:
        AuthenticatedUser^ currentUser;
        ApplicantEditRecord^ applicant;

        Label^ AddSection(String^ title, int y)
        {
            Label^ heading = gcnew Label();
            heading->Text = title;
            heading->Location = Point(35, y);
            heading->Size = Drawing::Size(820, 30);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 13.0f, FontStyle::Bold);
            this->Controls->Add(heading);
            return heading;
        }

        void AddDetail(
            String^ title, String^ value,
            int x, int y, int width)
        {
            Label^ caption = gcnew Label();
            caption->Text = title;
            caption->Location = Point(x, y);
            caption->Size = Drawing::Size(width, 22);
            caption->ForeColor =
                Color::FromArgb(155, 181, 162);
            caption->Font = gcnew Drawing::Font(
                "Segoe UI", 9.0f);
            this->Controls->Add(caption);

            Label^ content = gcnew Label();
            content->Text =
                String::IsNullOrWhiteSpace(value)
                ? "Not provided" : value;
            content->Location = Point(x, y + 24);
            content->Size = Drawing::Size(width, 43);
            content->ForeColor = Color::White;
            content->Font = gcnew Drawing::Font(
                "Segoe UI", 10.5f, FontStyle::Bold);
            content->AutoEllipsis = true;
            this->Controls->Add(content);
        }

        static String^ Money(Decimal amount)
        {
            return "LKR " + amount.ToString(
                "N2", CultureInfo::InvariantCulture);
        }

    public:
        ApplicantProfileForm(
            AuthenticatedUser^ actor, int applicantID)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;
            applicant = ApplicantEditService::GetApplicant(
                currentUser, applicantID);

            this->Text = "Group R - Applicant Profile";
            this->ClientSize = Drawing::Size(900, 750);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor =
                Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->AutoScroll = true;

            Label^ heading = gcnew Label();
            heading->Text = "APPLICANT PROFILE";
            heading->Location = Point(35, 20);
            heading->Size = Drawing::Size(820, 42);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 23.0f, FontStyle::Bold);
            this->Controls->Add(heading);

            Label^ code = gcnew Label();
            code->Text =
                applicant->ApplicantCode + "   |   ID: " +
                applicant->ApplicantID.ToString();
            code->Location = Point(37, 70);
            code->Size = Drawing::Size(800, 30);
            code->ForeColor = Color::Gainsboro;
            code->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            this->Controls->Add(code);

            int left = 40;
            int right = 465;
            int fieldWidth = 385;

            AddSection("PERSONAL DETAILS", 122);

            AddDetail("FULL NAME",
                applicant->FullName,
                left, 162, fieldWidth);

            AddDetail("NIC",
                applicant->NIC,
                right, 162, fieldWidth);

            String^ dob = applicant->DateOfBirth.HasValue
                ? applicant->DateOfBirth.Value.ToString(
                    "dd MMMM yyyy",
                    CultureInfo::InvariantCulture)
                : "Not provided";

            AddDetail("DATE OF BIRTH",
                dob, left, 233, fieldWidth);

            AddDetail("CREDIT SCORE",
                applicant->CreditScore.HasValue
                ? applicant->CreditScore.Value.ToString()
                : "Not provided",
                right, 233, fieldWidth);

            AddSection("CONTACT INFORMATION", 320);

            AddDetail("PHONE",
                applicant->Phone,
                left, 360, fieldWidth);

            AddDetail("EMAIL",
                applicant->Email,
                right, 360, fieldWidth);

            AddDetail("ADDRESS",
                applicant->Address,
                left, 431, 810);

            AddSection("EMPLOYMENT DETAILS", 516);

            AddDetail("OCCUPATION",
                applicant->Occupation,
                left, 556, fieldWidth);

            AddDetail("EMPLOYER",
                applicant->Employer,
                right, 556, fieldWidth);

            AddSection("FINANCIAL INFORMATION", 641);

            AddDetail("MONTHLY INCOME",
                Money(applicant->MonthlyIncome),
                left, 681, fieldWidth);

            AddDetail("EXISTING MONTHLY DEBT",
                Money(applicant->ExistingMonthlyDebt),
                right, 681, fieldWidth);

            Button^ closeButton = gcnew Button();
            closeButton->Text = "CLOSE PROFILE";
            closeButton->Location = Point(670, 778);
            closeButton->Size = Drawing::Size(185, 42);
            closeButton->BackColor =
                Color::FromArgb(22, 163, 74);
            closeButton->ForeColor = Color::White;
            closeButton->FlatStyle = FlatStyle::Flat;
            closeButton->FlatAppearance->BorderSize = 0;
            closeButton->DialogResult =
                System::Windows::Forms::DialogResult::OK;
            this->Controls->Add(closeButton);

            // Expand the scrollable form's content area.
            this->AutoScrollMinSize =
                Drawing::Size(880, 850);

            this->CancelButton = closeButton;
        }
    };
}
