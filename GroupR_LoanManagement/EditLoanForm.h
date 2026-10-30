#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanEditService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class EditLoanForm : public Form
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
        int loanID;

        Label^ loanLabel;
        Label^ applicantLabel;
        Label^ statusLabel;

        ComboBox^ typeBox;
        TextBox^ purposeBox;
        NumericUpDown^ amountBox;
        NumericUpDown^ rateBox;
        NumericUpDown^ termBox;

        Button^ saveButton;
        Button^ cancelButton;

        Label^ MakeLabel(
            String^ text, int x, int y, int w)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(w, 28);
            label->ForeColor = Color::White;
            label->BackColor = Color::Transparent;
            return label;
        }

        Button^ MakeButton(
            String^ text, int x, int y, int w)
        {
            Button^ button = gcnew Button();
            button->Text = text;
            button->Location = Point(x, y);
            button->Size = Drawing::Size(w, 44);
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->BackColor =
                Color::FromArgb(32, 165, 93);
            button->ForeColor = Color::White;
            button->Cursor = Cursors::Hand;
            return button;
        }

        void LoadLoan()
        {
            try
            {
                DataTable^ table =
                    LoanEditService::GetLoan(actor, loanID);

                DataRow^ row = table->Rows[0];

                loanLabel->Text =
                    "LOAN: " + Convert::ToString(
                        row["LoanCode"]);

                applicantLabel->Text =
                    "APPLICANT: " +
                    Convert::ToString(row["ApplicantCode"]) +
                    " - " +
                    Convert::ToString(row["ApplicantName"]);

                typeBox->Text =
                    Convert::ToString(row["LoanType"]);

                purposeBox->Text =
                    Convert::ToString(row["LoanPurpose"]);

                amountBox->Value =
                    Convert::ToDecimal(row["RequestedAmount"]);

                rateBox->Value =
                    Convert::ToDecimal(row["AnnualInterestRate"]);

                termBox->Value =
                    Decimal(Convert::ToInt32(row["TermMonths"]));

                String^ status =
                    Convert::ToString(row["LoanStatus"]);

                bool editable =
                    LoanEditService::CanEditStatus(status);

                saveButton->Enabled = editable;

                statusLabel->Text = editable
                    ? "STATUS: " + status +
                      " | Editing is permitted."
                    : "STATUS: " + status +
                      " | This loan is locked for editing.";

                statusLabel->ForeColor = editable
                    ? Color::FromArgb(111, 224, 151)
                    : Color::FromArgb(255, 181, 100);
            }
            catch (Exception^ ex)
            {
                saveButton->Enabled = false;
                statusLabel->Text =
                    "Unable to load loan details.";

                MessageBox::Show(
                    this, ex->Message,
                    "Edit Loan",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadLoan();
        }

        void OnSave(Object^ sender, EventArgs^ e)
        {
            try
            {
                saveButton->Enabled = false;

                LoanEditService::UpdateLoan(
                    actor,
                    loanID,
                    typeBox->Text,
                    purposeBox->Text,
                    amountBox->Value,
                    rateBox->Value,
                    Decimal::ToInt32(termBox->Value));

                MessageBox::Show(
                    this,
                    "Loan application updated successfully.",
                    "Update Successful",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex)
            {
                // Reload from the database, including its latest status.
                LoadLoan();

                MessageBox::Show(
                    this, ex->Message,
                    "Loan Update Failed",
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
            this->Text = "Group R - Edit Loan Application";
            this->ClientSize = Drawing::Size(780, 710);
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor =
                Color::FromArgb(18, 23, 21);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);

            Label^ title = MakeLabel(
                "EDIT LOAN APPLICATION", 30, 18, 710);
            title->Height = 45;
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 19.0f, FontStyle::Bold);

            loanLabel =
                MakeLabel("LOAN: -", 32, 82, 710);

            applicantLabel =
                MakeLabel("APPLICANT: -", 32, 119, 710);
            applicantLabel->ForeColor =
                Color::FromArgb(115, 227, 155);

            Label^ typeLabel =
                MakeLabel("LOAN TYPE", 32, 170, 710);

            typeBox = gcnew ComboBox();
            typeBox->Location = Point(32, 201);
            typeBox->Size = Drawing::Size(712, 36);
            typeBox->DropDownStyle = ComboBoxStyle::DropDown;
            typeBox->MaxLength = 40;

            array<String^>^ types = gcnew array<String^>{
                "Personal", "Business", "Education",
                "Vehicle", "Housing", "Emergency", "Other"
            };
            typeBox->Items->AddRange(types);

            Label^ purposeLabel =
                MakeLabel("LOAN PURPOSE", 32, 253, 710);

            purposeBox = gcnew TextBox();
            purposeBox->Location = Point(32, 284);
            purposeBox->Size = Drawing::Size(712, 79);
            purposeBox->Multiline = true;
            purposeBox->MaxLength = 200;
            purposeBox->ScrollBars = ScrollBars::Vertical;

            Label^ amountLabel =
                MakeLabel("REQUESTED AMOUNT (LKR)", 32, 386, 325);

            amountBox = gcnew NumericUpDown();
            amountBox->Location = Point(32, 417);
            amountBox->Size = Drawing::Size(333, 36);
            amountBox->Minimum = Decimal(1);
            amountBox->Maximum = Decimal(99999999);
            amountBox->DecimalPlaces = 2;
            amountBox->ThousandsSeparator = true;

            Label^ rateLabel =
                MakeLabel("ANNUAL INTEREST RATE (%)", 397, 386, 347);

            rateBox = gcnew NumericUpDown();
            rateBox->Location = Point(397, 417);
            rateBox->Size = Drawing::Size(347, 36);
            rateBox->Minimum = Decimal(0);
            rateBox->Maximum = Decimal(100);
            rateBox->DecimalPlaces = 2;

            Label^ termLabel =
                MakeLabel("REPAYMENT TERM (MONTHS)", 32, 481, 712);

            termBox = gcnew NumericUpDown();
            termBox->Location = Point(32, 512);
            termBox->Size = Drawing::Size(333, 36);
            termBox->Minimum = Decimal(1);
            termBox->Maximum = Decimal(360);

            statusLabel =
                MakeLabel("Loading...", 32, 573, 712);
            statusLabel->Height = 35;
            statusLabel->ForeColor =
                Color::FromArgb(115, 227, 155);

            saveButton =
                MakeButton("SAVE CHANGES", 32, 629, 450);
            saveButton->Enabled = false;

            cancelButton =
                MakeButton("CANCEL", 502, 629, 242);
            cancelButton->BackColor =
                Color::FromArgb(67, 74, 70);

            this->Controls->Add(title);
            this->Controls->Add(loanLabel);
            this->Controls->Add(applicantLabel);
            this->Controls->Add(typeLabel);
            this->Controls->Add(typeBox);
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
                this, &EditLoanForm::OnSave);

            cancelButton->Click += gcnew EventHandler(
                this, &EditLoanForm::OnCancel);

            this->Shown += gcnew EventHandler(
                this, &EditLoanForm::OnShown);

            this->AcceptButton = saveButton;
            this->CancelButton = cancelButton;
        }

    public:
        EditLoanForm(
            AuthenticatedUser^ currentUser,
            int selectedLoanID)
        {
        this->Shown += gcnew System::EventHandler(this, &EditLoanForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
                throw gcnew ArgumentNullException("currentUser");

            if (selectedLoanID <= 0)
                throw gcnew ArgumentException(
                    "Select a valid loan.");

            actor = currentUser;
            loanID = selectedLoanID;
            InitializeUI();
        }
    };
}
