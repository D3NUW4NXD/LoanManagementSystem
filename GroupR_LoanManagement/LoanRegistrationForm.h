#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanRegistrationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoanRegistrationForm : public Form
    {

    // GROUPR_PHASE95_PREMIUM_UI_V2
private:
    void ApplyPhase95ThemeOnShown(
        System::Object^ sender,
        System::EventArgs^ e)
    {
        ::GroupR_LoanManagement::GroupRModernTheme::Apply(this);
    }


    private:
        AuthenticatedUser^ actor;
        ComboBox^ applicantBox;
        ComboBox^ loanTypeBox;
        TextBox^ purposeBox;
        NumericUpDown^ amountBox;
        NumericUpDown^ rateBox;
        NumericUpDown^ termBox;
        Label^ statusLabel;
        Button^ saveButton;
        Button^ cancelButton;

        Label^ MakeLabel(
            String^ text, int x, int y, int w)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(w, 27);
            label->ForeColor = Color::White;
            return label;
        }

        Button^ MakeButton(
            String^ text, int x, int y, int w)
        {
            Button^ button = gcnew Button();
            button->Text = text;
            button->Location = Point(x, y);
            button->Size = Drawing::Size(w, 43);
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->BackColor = Color::FromArgb(30, 164, 92);
            button->ForeColor = Color::White;
            button->Cursor = Cursors::Hand;
            return button;
        }

        void LoadApplicants()
        {
            try
            {
                DataTable^ applicants =
                    LoanRegistrationService::GetApplicants(actor);

                applicantBox->DataSource = nullptr;
                applicantBox->DisplayMember = "DisplayName";
                applicantBox->ValueMember = "ApplicantID";
                applicantBox->DataSource = applicants;

                bool available = applicants->Rows->Count > 0;
                saveButton->Enabled = available;

                statusLabel->Text = available
                    ? "Enter the loan application details."
                    : "No applicants found. Register an applicant first.";
            }
            catch (Exception^ ex)
            {
                saveButton->Enabled = false;
                statusLabel->Text = "Cannot load applicants.";

                MessageBox::Show(
                    this,
                    ex->Message,
                    "Loan Registration",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadApplicants();
        }

        void OnSave(Object^ sender, EventArgs^ e)
        {
            DataRowView^ selected =
                dynamic_cast<DataRowView^>(
                    applicantBox->SelectedItem);

            if (selected == nullptr)
            {
                MessageBox::Show(
                    this, "Select an applicant.",
                    "Validation",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
                return;
            }

            try
            {
                int applicantID = Convert::ToInt32(
                    selected->Row["ApplicantID"]);

                saveButton->Enabled = false;
                statusLabel->Text = "Saving loan application...";

                int loanID =
                    LoanRegistrationService::RegisterLoan(
                        actor,
                        applicantID,
                        loanTypeBox->Text,
                        purposeBox->Text,
                        amountBox->Value,
                        rateBox->Value,
                        Decimal::ToInt32(termBox->Value));

                MessageBox::Show(
                    this,
                    "Loan application saved successfully.\n"
                    "Loan ID: " + loanID.ToString(),
                    "Registration Successful",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex)
            {
                saveButton->Enabled = true;
                statusLabel->Text = "Registration failed.";

                MessageBox::Show(
                    this, ex->Message,
                    "Loan Registration Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnCancel(Object^ sender, EventArgs^ e)
        {
            this->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Close();
        }

        void InitializeUI()
        {
            this->Text = "Group R - New Loan Application";
            this->ClientSize = Drawing::Size(780, 710);
            this->MinimumSize = Drawing::Size(780, 710);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(18, 23, 21);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;

            Label^ title = MakeLabel(
                "REGISTER NEW LOAN APPLICATION",
                30, 22, 710);
            title->Height = 46;
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 18.0f, FontStyle::Bold);

            Label^ subtitle = MakeLabel(
                "Create an application for an existing applicant.",
                32, 77, 690);
            subtitle->ForeColor =
                Color::FromArgb(175, 200, 185);

            Label^ applicantLabel =
                MakeLabel("APPLICANT", 32, 125, 690);

            applicantBox = gcnew ComboBox();
            applicantBox->Location = Point(32, 157);
            applicantBox->Size = Drawing::Size(712, 35);
            applicantBox->DropDownStyle =
                ComboBoxStyle::DropDownList;

            Label^ typeLabel =
                MakeLabel("LOAN TYPE", 32, 211, 690);

            loanTypeBox = gcnew ComboBox();
            loanTypeBox->Location = Point(32, 243);
            loanTypeBox->Size = Drawing::Size(712, 35);
            loanTypeBox->DropDownStyle =
                ComboBoxStyle::DropDown;
            loanTypeBox->MaxLength = 40;

            array<String^>^ loanTypes = gcnew array<String^>{
                "Personal", "Business", "Education",
                "Vehicle", "Housing", "Emergency", "Other"
            };
            loanTypeBox->Items->AddRange(loanTypes);
            loanTypeBox->SelectedIndex = 0;

            Label^ purposeLabel =
                MakeLabel("LOAN PURPOSE", 32, 297, 690);

            purposeBox = gcnew TextBox();
            purposeBox->Location = Point(32, 329);
            purposeBox->Size = Drawing::Size(712, 72);
            purposeBox->Multiline = true;
            purposeBox->MaxLength = 200;
            purposeBox->ScrollBars = ScrollBars::Vertical;

            Label^ amountLabel =
                MakeLabel("REQUESTED AMOUNT (LKR)", 32, 424, 335);

            amountBox = gcnew NumericUpDown();
            amountBox->Location = Point(32, 456);
            amountBox->Size = Drawing::Size(330, 35);
            amountBox->DecimalPlaces = 2;
            amountBox->ThousandsSeparator = true;
            amountBox->Minimum = Decimal(1);
            amountBox->Maximum = Decimal(99999999);
            amountBox->Value = Decimal(100000);

            Label^ rateLabel =
                MakeLabel("ANNUAL INTEREST RATE (%)", 399, 424, 345);

            rateBox = gcnew NumericUpDown();
            rateBox->Location = Point(399, 456);
            rateBox->Size = Drawing::Size(345, 35);
            rateBox->DecimalPlaces = 2;
            rateBox->Increment = Decimal(1);
            rateBox->Minimum = Decimal(0);
            rateBox->Maximum = Decimal(100);
            rateBox->Value = Decimal(12);

            Label^ termLabel =
                MakeLabel("REPAYMENT TERM (MONTHS)", 32, 515, 500);

            termBox = gcnew NumericUpDown();
            termBox->Location = Point(32, 547);
            termBox->Size = Drawing::Size(330, 35);
            termBox->Minimum = Decimal(1);
            termBox->Maximum = Decimal(360);
            termBox->Value = Decimal(12);

            statusLabel = MakeLabel(
                "Ready.", 32, 597, 712);
            statusLabel->ForeColor =
                Color::FromArgb(142, 220, 167);

            saveButton =
                MakeButton("SAVE APPLICATION", 32, 634, 450);

            cancelButton =
                MakeButton("CANCEL", 501, 634, 243);
            cancelButton->BackColor =
                Color::FromArgb(68, 76, 71);

            this->Controls->Add(title);
            this->Controls->Add(subtitle);
            this->Controls->Add(applicantLabel);
            this->Controls->Add(applicantBox);
            this->Controls->Add(typeLabel);
            this->Controls->Add(loanTypeBox);
            this->Controls->Add(purposeLabel);
            this->Controls->Add(purposeBox);
            this->Controls->Add(amountLabel);
            this->Controls->Add(amountBox);
            this->Controls->Add(rateLabel);
            this->Controls->Add(rateBox);
            this->Controls->Add(termLabel);
            this->Controls->Add(termBox);
            this->Controls->Add(statusLabel);
            this->Controls->Add(saveButton);
            this->Controls->Add(cancelButton);

            saveButton->Click += gcnew EventHandler(
                this, &LoanRegistrationForm::OnSave);

            cancelButton->Click += gcnew EventHandler(
                this, &LoanRegistrationForm::OnCancel);

            this->Shown += gcnew EventHandler(
                this, &LoanRegistrationForm::OnShown);

            this->AcceptButton = saveButton;
            this->CancelButton = cancelButton;
        }

    public:
        LoanRegistrationForm(AuthenticatedUser^ currentUser)
        {
        this->Shown += gcnew System::EventHandler(this, &LoanRegistrationForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
                throw gcnew ArgumentNullException("currentUser");

            actor = currentUser;
            InitializeUI();
        }
    };
}
