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
    public ref class EditApplicantForm : public Form
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

        TextBox^ nameBox;
        TextBox^ nicBox;
        TextBox^ phoneBox;
        TextBox^ emailBox;
        TextBox^ addressBox;
        TextBox^ occupationBox;
        TextBox^ employerBox;
        TextBox^ incomeBox;
        TextBox^ debtBox;
        TextBox^ scoreBox;

        DateTimePicker^ dobPicker;
        Button^ saveButton;

        TextBox^ AddField(
            String^ title, int x, int y, int limit)
        {
            Label^ label = gcnew Label();
            label->Text = title;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(335, 23);
            label->ForeColor =
                Color::FromArgb(134, 239, 172);
            label->Font = gcnew Drawing::Font(
                "Segoe UI", 9.5f);

            this->Controls->Add(label);

            TextBox^ box = gcnew TextBox();
            box->Location = Point(x, y + 25);
            box->Size = Drawing::Size(335, 31);
            box->MaxLength = limit;
            box->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            box->BackColor =
                Color::FromArgb(31, 44, 39);
            box->ForeColor = Color::White;
            box->BorderStyle = BorderStyle::FixedSingle;

            this->Controls->Add(box);
            return box;
        }

        static Decimal ParseAmount(
            String^ input, String^ fieldName)
        {
            Decimal value;

            if (!Decimal::TryParse(
                input->Trim(),
                NumberStyles::Number,
                CultureInfo::InvariantCulture,
                value))
            {
                throw gcnew ArgumentException(
                    fieldName + " must be a valid number. "
                    "Example: 45000.00");
            }

            return value;
        }

        void SaveChanges(Object^ sender, EventArgs^ e)
        {
            try
            {
                ApplicantEditRecord^ changes =
                    gcnew ApplicantEditRecord();

                changes->ApplicantID = applicant->ApplicantID;

                changes->FullName = nameBox->Text;
                changes->NIC = nicBox->Text;
                changes->Phone = phoneBox->Text;
                changes->Email = emailBox->Text;
                changes->Address = addressBox->Text;
                changes->Occupation = occupationBox->Text;
                changes->Employer = employerBox->Text;

                changes->MonthlyIncome =
                    ParseAmount(incomeBox->Text, "Monthly income");

                changes->ExistingMonthlyDebt =
                    ParseAmount(debtBox->Text, "Monthly debt");

                if (dobPicker->Checked)
                {
                    changes->DateOfBirth = Nullable<DateTime>(
                        dobPicker->Value.Date);
                }

                if (!String::IsNullOrWhiteSpace(scoreBox->Text))
                {
                    int score;
                    if (!Int32::TryParse(
                        scoreBox->Text->Trim(), score))
                    {
                        throw gcnew ArgumentException(
                            "Credit score must be a whole number.");
                    }

                    changes->CreditScore = Nullable<int>(score);
                }

                System::Windows::Forms::DialogResult confirm =
                    MessageBox::Show(
                        "Save changes to applicant " +
                        applicant->ApplicantCode + "?",
                        "Confirm Applicant Update",
                        MessageBoxButtons::YesNo,
                        MessageBoxIcon::Question);

                if (confirm !=
                    System::Windows::Forms::DialogResult::Yes)
                    return;

                saveButton->Enabled = false;

                ApplicantEditService::UpdateApplicant(
                    currentUser, changes);

                MessageBox::Show(
                    "Applicant information updated successfully.",
                    "Update Complete",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;

                this->Close();
            }
            catch (ArgumentException^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Check Applicant Details",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
            }
            catch (UnauthorizedAccessException^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Access Denied",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    "Could not update applicant." +
                    Environment::NewLine + ex->Message,
                    "Update Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
            finally
            {
                saveButton->Enabled = true;
            }
        }

    public:
        EditApplicantForm(
            AuthenticatedUser^ actor, int applicantID)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;

            // Load the latest values directly from SQL Server.
            applicant = ApplicantEditService::GetApplicant(
                currentUser, applicantID);

            this->Text = "Group R - Edit Applicant";
            this->ClientSize = Drawing::Size(790, 730);
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
            heading->Text = "EDIT APPLICANT";
            heading->Location = Point(35, 22);
            heading->Size = Drawing::Size(700, 43);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 23.0f, FontStyle::Bold);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            this->Controls->Add(heading);

            Label^ identity = gcnew Label();
            identity->Text =
                "ID: " + applicant->ApplicantID.ToString() +
                "     CODE: " + applicant->ApplicantCode;
            identity->Location = Point(35, 72);
            identity->Size = Drawing::Size(710, 28);
            identity->ForeColor = Color::Gainsboro;
            identity->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);
            this->Controls->Add(identity);

            int left = 35;
            int right = 412;

            nameBox = AddField(
                "FULL NAME *", left, 110, 150);
            phoneBox = AddField(
                "PHONE NUMBER *", right, 110, 25);

            nicBox = AddField(
                "NIC", left, 190, 20);
            emailBox = AddField(
                "EMAIL", right, 190, 150);

            addressBox = AddField(
                "ADDRESS", left, 270, 300);
            occupationBox = AddField(
                "OCCUPATION", right, 270, 100);

            employerBox = AddField(
                "EMPLOYER", left, 350, 150);
            incomeBox = AddField(
                "MONTHLY INCOME (LKR) *", right, 350, 30);

            debtBox = AddField(
                "EXISTING MONTHLY DEBT (LKR) *",
                left, 430, 30);
            scoreBox = AddField(
                "CREDIT SCORE", right, 430, 10);

            Label^ dobLabel = gcnew Label();
            dobLabel->Text = "DATE OF BIRTH (OPTIONAL)";
            dobLabel->Location = Point(left, 510);
            dobLabel->Size = Drawing::Size(340, 24);
            dobLabel->ForeColor = Color::Gainsboro;
            this->Controls->Add(dobLabel);

            dobPicker = gcnew DateTimePicker();
            dobPicker->Location = Point(left, 540);
            dobPicker->Size = Drawing::Size(335, 32);
            dobPicker->Format =
                DateTimePickerFormat::Short;
            dobPicker->ShowCheckBox = true;
            dobPicker->MaxDate = DateTime::Today;
            this->Controls->Add(dobPicker);

            // Populate every editable field.
            nameBox->Text = applicant->FullName;
            nicBox->Text = applicant->NIC;
            phoneBox->Text = applicant->Phone;
            emailBox->Text = applicant->Email;
            addressBox->Text = applicant->Address;
            occupationBox->Text = applicant->Occupation;
            employerBox->Text = applicant->Employer;

            incomeBox->Text =
                applicant->MonthlyIncome.ToString(
                    CultureInfo::InvariantCulture);

            debtBox->Text =
                applicant->ExistingMonthlyDebt.ToString(
                    CultureInfo::InvariantCulture);

            scoreBox->Text = applicant->CreditScore.HasValue
                ? applicant->CreditScore.Value.ToString()
                : String::Empty;

            dobPicker->Checked =
                applicant->DateOfBirth.HasValue;

            if (applicant->DateOfBirth.HasValue)
                dobPicker->Value =
                    applicant->DateOfBirth.Value.Date;

            Button^ cancel = gcnew Button();
            cancel->Text = "CANCEL";
            cancel->Location = Point(420, 640);
            cancel->Size = Drawing::Size(145, 43);
            cancel->BackColor =
                Color::FromArgb(57, 70, 63);
            cancel->ForeColor = Color::White;
            cancel->FlatStyle = FlatStyle::Flat;
            cancel->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Controls->Add(cancel);

            saveButton = gcnew Button();
            saveButton->Text = "SAVE CHANGES";
            saveButton->Location = Point(580, 640);
            saveButton->Size = Drawing::Size(172, 43);
            saveButton->BackColor =
                Color::FromArgb(22, 163, 74);
            saveButton->ForeColor = Color::White;
            saveButton->FlatStyle = FlatStyle::Flat;
            saveButton->FlatAppearance->BorderSize = 0;

            saveButton->Click += gcnew EventHandler(
                this, &EditApplicantForm::SaveChanges);

            this->Controls->Add(saveButton);

            this->AcceptButton = saveButton;
            this->CancelButton = cancel;
        }
    };
}
