#pragma once
#include "GroupRModernTheme.h"
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "GuarantorRegistrationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class GuarantorRegistrationForm : public Form
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
        ComboBox^ applicantBox;
        TextBox^ nameBox;
        TextBox^ nicBox;
        TextBox^ phoneBox;
        TextBox^ emailBox;
        TextBox^ addressBox;
        TextBox^ relationshipBox;
        TextBox^ occupationBox;
        TextBox^ incomeBox;

        TextBox^ AddField(
            String^ title, int x, int y, int width)
        {
            Label^ label = gcnew Label();
            label->Text = title;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(width, 23);
            label->ForeColor = Color::LightGray;
            label->Font = gcnew Drawing::Font(
                "Segoe UI", 9.0f, FontStyle::Bold);
            this->Controls->Add(label);

            TextBox^ field = gcnew TextBox();
            field->Location = Point(x, y + 26);
            field->Size = Drawing::Size(width, 31);
            field->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            field->BackColor =
                Color::FromArgb(32, 47, 39);
            field->ForeColor = Color::White;
            field->BorderStyle = BorderStyle::FixedSingle;
            this->Controls->Add(field);
            return field;
        }

        void SaveGuarantor(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (applicantBox->SelectedValue == nullptr ||
                    applicantBox->SelectedValue == DBNull::Value)
                    throw gcnew ArgumentException(
                        "Select an applicant.");

                int applicantID = Convert::ToInt32(
                    applicantBox->SelectedValue);

                Decimal income;
                if (!Decimal::TryParse(
                    incomeBox->Text->Trim(),
                    NumberStyles::Number,
                    CultureInfo::InvariantCulture,
                    income))
                {
                    throw gcnew ArgumentException(
                        "Enter a valid monthly income. "
                        "Use a period for decimal values.");
                }

                int newID =
                    GuarantorRegistrationService::Register(
                        currentUser,
                        applicantID,
                        nameBox->Text,
                        nicBox->Text,
                        phoneBox->Text,
                        emailBox->Text,
                        addressBox->Text,
                        relationshipBox->Text,
                        occupationBox->Text,
                        income);

                MessageBox::Show(
                    "Guarantor registered successfully.\n"
                    "Guarantor ID: " + newID.ToString(),
                    "Registration Complete",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Registration Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

    public:
        GuarantorRegistrationForm(AuthenticatedUser^ actor)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;

            this->Text = "Register Guarantor - Group R";
            this->ClientSize = Drawing::Size(790, 695);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor =
                Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;

            Label^ heading = gcnew Label();
            heading->Text = "REGISTER GUARANTOR";
            heading->Location = Point(30, 20);
            heading->Size = Drawing::Size(725, 45);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 21.0f, FontStyle::Bold);
            this->Controls->Add(heading);

            Label^ applicantLabel = gcnew Label();
            applicantLabel->Text = "SELECT APPLICANT *";
            applicantLabel->Location = Point(35, 85);
            applicantLabel->Size = Drawing::Size(700, 23);
            applicantLabel->ForeColor = Color::LightGray;
            this->Controls->Add(applicantLabel);

            applicantBox = gcnew ComboBox();
            applicantBox->Location = Point(35, 112);
            applicantBox->Size = Drawing::Size(720, 34);
            applicantBox->DropDownStyle =
                ComboBoxStyle::DropDownList;
            applicantBox->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            this->Controls->Add(applicantBox);

            nameBox = AddField(
                "FULL NAME *", 35, 170, 340);
            nicBox = AddField(
                "NIC", 410, 170, 345);

            phoneBox = AddField(
                "PHONE *", 35, 250, 340);
            emailBox = AddField(
                "EMAIL", 410, 250, 345);

            addressBox = AddField(
                "ADDRESS", 35, 330, 720);

            relationshipBox = AddField(
                "RELATIONSHIP TO APPLICANT *",
                35, 410, 340);

            occupationBox = AddField(
                "OCCUPATION", 410, 410, 345);

            incomeBox = AddField(
                "MONTHLY INCOME (LKR) *",
                35, 490, 340);
            incomeBox->Text = "0.00";

            Button^ save = gcnew Button();
            save->Text = "SAVE GUARANTOR";
            save->Location = Point(410, 595);
            save->Size = Drawing::Size(200, 45);
            save->BackColor =
                Color::FromArgb(22, 163, 74);
            save->ForeColor = Color::White;
            save->FlatStyle = FlatStyle::Flat;
            save->FlatAppearance->BorderSize = 0;
            save->Click += gcnew EventHandler(
                this,
                &GuarantorRegistrationForm::SaveGuarantor);
            this->Controls->Add(save);

            Button^ cancel = gcnew Button();
            cancel->Text = "CANCEL";
            cancel->Location = Point(625, 595);
            cancel->Size = Drawing::Size(130, 45);
            cancel->BackColor =
                Color::FromArgb(58, 69, 63);
            cancel->ForeColor = Color::White;
            cancel->FlatStyle = FlatStyle::Flat;
            cancel->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Controls->Add(cancel);
            this->CancelButton = cancel;

            DataTable^ applicants =
                GuarantorRegistrationService::GetApplicants(
                    currentUser);

            applicantBox->DisplayMember = "DisplayName";
            applicantBox->ValueMember = "ApplicantID";
            applicantBox->DataSource = applicants;
            applicantBox->SelectedIndex = -1;

            if (applicants->Rows->Count == 0)
            {
                save->Enabled = false;
                MessageBox::Show(
                    "Register an applicant before "
                    "registering a guarantor.",
                    "No Applicants",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
            }
        }
    };
}
