#pragma once
#include "GroupRModernTheme.h"
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "GuarantorEditService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class EditGuarantorForm : public Form
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
        GuarantorEditRecord^ record;

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
            String^ labelText, int x, int y, int width)
        {
            Label^ label = gcnew Label();
            label->Text = labelText;
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

        void SaveChanges(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (applicantBox->SelectedValue == nullptr ||
                    applicantBox->SelectedValue == DBNull::Value)
                    throw gcnew ArgumentException(
                        "Select an applicant.");

                Decimal income;
                if (!Decimal::TryParse(
                    incomeBox->Text->Trim(),
                    NumberStyles::Number,
                    CultureInfo::InvariantCulture,
                    income))
                    throw gcnew ArgumentException(
                        "Enter a valid monthly income.");

                if (MessageBox::Show(
                    "Save changes to this guarantor?",
                    "Confirm Update",
                    MessageBoxButtons::YesNo,
                    MessageBoxIcon::Question)
                    != System::Windows::Forms::DialogResult::Yes)
                    return;

                record->ApplicantID =
                    Convert::ToInt32(
                        applicantBox->SelectedValue);
                record->FullName = nameBox->Text;
                record->NIC = nicBox->Text;
                record->Phone = phoneBox->Text;
                record->Email = emailBox->Text;
                record->Address = addressBox->Text;
                record->RelationshipToApplicant =
                    relationshipBox->Text;
                record->Occupation = occupationBox->Text;
                record->MonthlyIncome = income;

                GuarantorEditService::Update(
                    currentUser, record);

                MessageBox::Show(
                    "Guarantor information updated successfully.",
                    "Update Complete",
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
                    "Update Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

    public:
        EditGuarantorForm(
            AuthenticatedUser^ actor, int guarantorID)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;
            record = GuarantorEditService::GetGuarantor(
                actor, guarantorID);

            this->Text = "Edit Guarantor - Group R";
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
            heading->Text = "EDIT GUARANTOR";
            heading->Location = Point(30, 18);
            heading->Size = Drawing::Size(725, 44);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 21.0f, FontStyle::Bold);
            this->Controls->Add(heading);

            Label^ codeLabel = gcnew Label();
            codeLabel->Text =
                record->GuarantorCode +
                "   |   ID: " + record->GuarantorID.ToString();
            codeLabel->Location = Point(35, 67);
            codeLabel->Size = Drawing::Size(720, 25);
            codeLabel->ForeColor = Color::Silver;
            this->Controls->Add(codeLabel);

            Label^ applicantLabel = gcnew Label();
            applicantLabel->Text = "LINKED APPLICANT *";
            applicantLabel->Location = Point(35, 104);
            applicantLabel->Size = Drawing::Size(700, 23);
            applicantLabel->ForeColor = Color::LightGray;
            this->Controls->Add(applicantLabel);

            applicantBox = gcnew ComboBox();
            applicantBox->Location = Point(35, 130);
            applicantBox->Size = Drawing::Size(720, 34);
            applicantBox->DropDownStyle =
                ComboBoxStyle::DropDownList;
            applicantBox->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            this->Controls->Add(applicantBox);

            nameBox = AddField(
                "FULL NAME *", 35, 182, 340);
            nicBox = AddField(
                "NIC", 410, 182, 345);

            phoneBox = AddField(
                "PHONE *", 35, 260, 340);
            emailBox = AddField(
                "EMAIL", 410, 260, 345);

            addressBox = AddField(
                "ADDRESS", 35, 338, 720);

            relationshipBox = AddField(
                "RELATIONSHIP TO APPLICANT *",
                35, 416, 340);
            occupationBox = AddField(
                "OCCUPATION", 410, 416, 345);

            incomeBox = AddField(
                "MONTHLY INCOME (LKR) *",
                35, 494, 340);

            nameBox->Text = record->FullName;
            nicBox->Text = record->NIC;
            phoneBox->Text = record->Phone;
            emailBox->Text = record->Email;
            addressBox->Text = record->Address;
            relationshipBox->Text =
                record->RelationshipToApplicant;
            occupationBox->Text = record->Occupation;
            incomeBox->Text =
                record->MonthlyIncome.ToString(
                    "F2", CultureInfo::InvariantCulture);

            DataTable^ applicants =
                GuarantorRegistrationService::GetApplicants(
                    currentUser);

            applicantBox->DisplayMember = "DisplayName";
            applicantBox->ValueMember = "ApplicantID";
            applicantBox->DataSource = applicants;
            applicantBox->SelectedValue = record->ApplicantID;

            Button^ save = gcnew Button();
            save->Text = "SAVE CHANGES";
            save->Location = Point(410, 595);
            save->Size = Drawing::Size(200, 45);
            save->BackColor =
                Color::FromArgb(22, 163, 74);
            save->ForeColor = Color::White;
            save->FlatStyle = FlatStyle::Flat;
            save->FlatAppearance->BorderSize = 0;
            save->Click += gcnew EventHandler(
                this, &EditGuarantorForm::SaveChanges);
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
        }
    };
}
