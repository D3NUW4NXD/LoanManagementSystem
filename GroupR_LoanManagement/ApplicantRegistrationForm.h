#pragma once
#include "GroupRModernTheme.h"
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "ApplicantRegistrationService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class ApplicantRegistrationForm : public Form
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
            String^ label, int x, int y,
            int maxLength, bool required)
        {
            Label^ title = gcnew Label();
            title->Text = label + (required ? " *" : "");
            title->Location = Point(x, y);
            title->Size = Drawing::Size(340, 23);
            title->ForeColor =
                Color::FromArgb(134, 239, 172);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);
            this->Controls->Add(title);

            TextBox^ field = gcnew TextBox();
            field->Location = Point(x, y + 26);
            field->Size = Drawing::Size(338, 32);
            field->MaxLength = maxLength;
            field->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            field->BackColor =
                Color::FromArgb(31, 44, 39);
            field->ForeColor = Color::White;
            field->BorderStyle =
                BorderStyle::FixedSingle;

            this->Controls->Add(field);
            return field;
        }

        static Decimal ReadMoney(
            String^ value, String^ label)
        {
            Decimal result;

            if (!Decimal::TryParse(
                value->Trim(),
                NumberStyles::Number,
                CultureInfo::InvariantCulture,
                result))
            {
                throw gcnew ArgumentException(
                    label + " must be a valid number. "
                    "Example: 50000.00");
            }

            return result;
        }

        void SaveApplicant(Object^ sender, EventArgs^ e)
        {
            try
            {
                Nullable<DateTime> dob;

                if (dobPicker->Checked)
                    dob = Nullable<DateTime>(
                        dobPicker->Value.Date);

                Nullable<int> creditScore;

                if (!String::IsNullOrWhiteSpace(
                    scoreBox->Text))
                {
                    int value;

                    if (!Int32::TryParse(
                        scoreBox->Text->Trim(), value))
                    {
                        throw gcnew ArgumentException(
                            "Credit score must be a whole number.");
                    }

                    creditScore = Nullable<int>(value);
                }

                Decimal income = ReadMoney(
                    incomeBox->Text, "Monthly income");

                Decimal debt = ReadMoney(
                    debtBox->Text, "Existing monthly debt");

                saveButton->Enabled = false;

                int applicantID =
                    ApplicantRegistrationService::Register(
                        currentUser,
                        nameBox->Text,
                        nicBox->Text,
                        dob,
                        phoneBox->Text,
                        emailBox->Text,
                        addressBox->Text,
                        occupationBox->Text,
                        employerBox->Text,
                        income,
                        debt,
                        creditScore);

                MessageBox::Show(
                    "Applicant registered successfully!" +
                    Environment::NewLine +
                    "Applicant ID: " +
                    applicantID.ToString(),
                    "Registration Complete",
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
                    "Could not register applicant." +
                    Environment::NewLine + ex->Message,
                    "Registration Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
            finally
            {
                saveButton->Enabled = true;
            }
        }

    public:
        ApplicantRegistrationForm(
            AuthenticatedUser^ actor)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Please sign in again.");

            currentUser = actor;

            this->Text = "Group R - Register Applicant";
            this->ClientSize = Drawing::Size(790, 725);
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
            heading->Text = "REGISTER NEW APPLICANT";
            heading->Location = Point(35, 25);
            heading->Size = Drawing::Size(710, 45);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 22.0f, FontStyle::Bold);
            heading->ForeColor =
                Color::FromArgb(74, 222, 128);
            this->Controls->Add(heading);

            Label^ sub = gcnew Label();
            sub->Text =
                "Fields marked * are required. "
                "Applicant code is generated automatically.";
            sub->Location = Point(35, 77);
            sub->Size = Drawing::Size(710, 25);
            sub->ForeColor = Color::Silver;
            this->Controls->Add(sub);

            int left = 35;
            int right = 412;

            nameBox = AddField(
                "FULL NAME", left, 112, 150, true);
            phoneBox = AddField(
                "PHONE NUMBER", right, 112, 25, true);

            nicBox = AddField(
                "NIC", left, 192, 20, false);
            emailBox = AddField(
                "EMAIL", right, 192, 150, false);

            addressBox = AddField(
                "ADDRESS", left, 272, 300, false);
            occupationBox = AddField(
                "OCCUPATION", right, 272, 100, false);

            employerBox = AddField(
                "EMPLOYER", left, 352, 150, false);
            incomeBox = AddField(
                "MONTHLY INCOME (LKR)",
                right, 352, 30, true);

            debtBox = AddField(
                "EXISTING MONTHLY DEBT (LKR)",
                left, 432, 30, true);
            debtBox->Text = "0";

            scoreBox = AddField(
                "CREDIT SCORE", right, 432, 10, false);

            Label^ dobLabel = gcnew Label();
            dobLabel->Text = "DATE OF BIRTH (OPTIONAL)";
            dobLabel->Location = Point(left, 516);
            dobLabel->Size = Drawing::Size(340, 24);
            dobLabel->ForeColor = Color::Gainsboro;
            this->Controls->Add(dobLabel);

            dobPicker = gcnew DateTimePicker();
            dobPicker->Location = Point(left, 544);
            dobPicker->Size = Drawing::Size(338, 32);
            dobPicker->Format =
                DateTimePickerFormat::Short;
            dobPicker->ShowCheckBox = true;
            dobPicker->Checked = false;
            dobPicker->MaxDate = DateTime::Today;
            this->Controls->Add(dobPicker);

            Button^ cancel = gcnew Button();
            cancel->Text = "CANCEL";
            cancel->Location = Point(427, 644);
            cancel->Size = Drawing::Size(140, 42);
            cancel->BackColor =
                Color::FromArgb(57, 70, 63);
            cancel->ForeColor = Color::White;
            cancel->FlatStyle = FlatStyle::Flat;
            cancel->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Controls->Add(cancel);

            saveButton = gcnew Button();
            saveButton->Text = "SAVE APPLICANT";
            saveButton->Location = Point(580, 644);
            saveButton->Size = Drawing::Size(172, 42);
            saveButton->BackColor =
                Color::FromArgb(22, 163, 74);
            saveButton->ForeColor = Color::White;
            saveButton->FlatStyle = FlatStyle::Flat;
            saveButton->FlatAppearance->BorderSize = 0;

            saveButton->Click +=
                gcnew EventHandler(
                    this,
                    &ApplicantRegistrationForm::SaveApplicant);

            this->Controls->Add(saveButton);

            this->AcceptButton = saveButton;
            this->CancelButton = cancel;
        }
    };
}
