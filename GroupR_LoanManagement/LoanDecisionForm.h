#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanDecisionService.h"
#include "LoanDetailsService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoanDecisionForm : public Form
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

        Label^ titleLabel;
        Label^ applicantLabel;
        Label^ amountLabel;
        Label^ statusLabel;
        Label^ feedbackLabel;

        NumericUpDown^ approvedBox;
        TextBox^ rejectionBox;

        Button^ approveButton;
        Button^ rejectButton;
        Button^ closeButton;

        Label^ MakeLabel(String^ text, int x, int y, int w)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(w, 32);
            label->ForeColor = Color::White;
            return label;
        }

        Button^ MakeButton(
            String^ text, int x, int y, int w, Color color)
        {
            Button^ button = gcnew Button();
            button->Text = text;
            button->Location = Point(x, y);
            button->Size = Drawing::Size(w, 44);
            button->BackColor = color;
            button->ForeColor = Color::White;
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->Cursor = Cursors::Hand;
            return button;
        }

        void LoadLoan()
        {
            approveButton->Enabled = false;
            rejectButton->Enabled = false;

            try
            {
                DataSet^ data =
                    LoanDetailsService::GetDetails(actor, loanID);

                DataRow^ loan = data->Tables["Loan"]->Rows[0];

                String^ loanCode =
                    Convert::ToString(loan["LoanCode"]);

                String^ status =
                    Convert::ToString(loan["LoanStatus"]);

                Decimal requested =
                    Convert::ToDecimal(loan["RequestedAmount"]);

                titleLabel->Text =
                    "LOAN DECISION - " + loanCode;

                applicantLabel->Text =
                    "APPLICANT: " +
                    Convert::ToString(loan["ApplicantName"]);

                amountLabel->Text =
                    "REQUESTED AMOUNT: LKR " +
                    requested.ToString("N2");

                statusLabel->Text =
                    "CURRENT STATUS: " + status;

                approvedBox->Minimum = Decimal::Zero;

                if (Decimal::Compare(
                    requested, approvedBox->Maximum) > 0)
                {
                    approvedBox->Maximum = requested;
                }

                approvedBox->Value = requested;

                bool pending = String::Equals(
                    status->Trim(), "Pending",
                    StringComparison::OrdinalIgnoreCase);

                bool administrator =
                    actor->RoleID == 1 || actor->RoleID == 2;

                bool alreadyDecided =
                    loan["DecisionAt"] != DBNull::Value;

                bool permitted =
                    pending && administrator && !alreadyDecided;

                approveButton->Enabled = permitted;
                rejectButton->Enabled = permitted;

                feedbackLabel->Text = permitted
                    ? "Choose APPROVE or REJECT."
                    : "Decision unavailable: check role or loan status.";

                feedbackLabel->ForeColor = permitted
                    ? Color::FromArgb(130, 225, 160)
                    : Color::FromArgb(250, 175, 110);
            }
            catch (Exception^ ex)
            {
                feedbackLabel->Text = "Unable to load this loan.";

                MessageBox::Show(
                    this, ex->Message,
                    "Loan Decision",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnApprove(Object^ sender, EventArgs^ e)
        {
            if (MessageBox::Show(
                this,
                "Approve this loan for LKR " +
                    approvedBox->Value.ToString("N2") + "?",
                "Confirm Loan Approval",
                MessageBoxButtons::YesNo,
                MessageBoxIcon::Question) !=
                System::Windows::Forms::DialogResult::Yes)
                return;

            try
            {
                approveButton->Enabled = false;
                rejectButton->Enabled = false;

                LoanDecisionService::Approve(
                    actor, loanID, approvedBox->Value);

                MessageBox::Show(
                    this,
                    "Loan approved and audit entry saved.",
                    "Approval Successful",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex)
            {
                LoadLoan();

                MessageBox::Show(
                    this, ex->Message,
                    "Approval Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnReject(Object^ sender, EventArgs^ e)
        {
            String^ reason = rejectionBox->Text->Trim();

            if (String::IsNullOrWhiteSpace(reason))
            {
                MessageBox::Show(
                    this,
                    "Enter a rejection reason before continuing.",
                    "Reason Required",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
                return;
            }

            if (MessageBox::Show(
                this,
                "Reject this loan application?",
                "Confirm Loan Rejection",
                MessageBoxButtons::YesNo,
                MessageBoxIcon::Warning) !=
                System::Windows::Forms::DialogResult::Yes)
                return;

            try
            {
                approveButton->Enabled = false;
                rejectButton->Enabled = false;

                LoanDecisionService::Reject(
                    actor, loanID, reason);

                MessageBox::Show(
                    this,
                    "Loan rejected and audit entry saved.",
                    "Rejection Successful",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (Exception^ ex)
            {
                LoadLoan();

                MessageBox::Show(
                    this, ex->Message,
                    "Rejection Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadLoan();
        }

        void InitializeUI()
        {
            this->Text = "Group R - Loan Decision";
            this->ClientSize = Drawing::Size(720, 600);
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(18, 23, 21);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);

            titleLabel =
                MakeLabel("LOAN DECISION", 30, 24, 660);
            titleLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 18.0f, FontStyle::Bold);
            titleLabel->Height = 48;

            applicantLabel =
                MakeLabel("APPLICANT: -", 30, 90, 660);

            amountLabel =
                MakeLabel("REQUESTED AMOUNT: -", 30, 134, 660);

            statusLabel =
                MakeLabel("CURRENT STATUS: -", 30, 178, 660);
            statusLabel->ForeColor =
                Color::FromArgb(120, 225, 155);

            Label^ approvedLabel =
                MakeLabel("APPROVED AMOUNT (LKR)", 30, 235, 660);

            approvedBox = gcnew NumericUpDown();
            approvedBox->Location = Point(30, 272);
            approvedBox->Size = Drawing::Size(660, 35);
            approvedBox->DecimalPlaces = 2;
            approvedBox->ThousandsSeparator = true;
            approvedBox->Maximum =
                Decimal::Parse("999999999999999.99",
                    System::Globalization::CultureInfo::InvariantCulture);

            Label^ reasonLabel =
                MakeLabel("REJECTION REASON", 30, 332, 660);

            rejectionBox = gcnew TextBox();
            rejectionBox->Location = Point(30, 368);
            rejectionBox->Size = Drawing::Size(660, 76);
            rejectionBox->Multiline = true;
            rejectionBox->ScrollBars = ScrollBars::Vertical;
            rejectionBox->MaxLength = 4000;

            feedbackLabel =
                MakeLabel("Loading loan...", 30, 466, 660);

            approveButton = MakeButton(
                "APPROVE LOAN", 30, 517, 212,
                Color::FromArgb(30, 159, 86));

            rejectButton = MakeButton(
                "REJECT LOAN", 254, 517, 212,
                Color::FromArgb(170, 60, 60));

            closeButton = MakeButton(
                "CANCEL", 478, 517, 212,
                Color::FromArgb(69, 77, 71));

            approveButton->Enabled = false;
            rejectButton->Enabled = false;

            this->Controls->Add(titleLabel);
            this->Controls->Add(applicantLabel);
            this->Controls->Add(amountLabel);
            this->Controls->Add(statusLabel);
            this->Controls->Add(approvedLabel);
            this->Controls->Add(approvedBox);
            this->Controls->Add(reasonLabel);
            this->Controls->Add(rejectionBox);
            this->Controls->Add(feedbackLabel);
            this->Controls->Add(approveButton);
            this->Controls->Add(rejectButton);
            this->Controls->Add(closeButton);

            approveButton->Click += gcnew EventHandler(
                this, &LoanDecisionForm::OnApprove);

            rejectButton->Click += gcnew EventHandler(
                this, &LoanDecisionForm::OnReject);

            closeButton->Click += gcnew EventHandler(
                this, &LoanDecisionForm::OnClose);

            this->Shown += gcnew EventHandler(
                this, &LoanDecisionForm::OnShown);

            this->CancelButton = closeButton;
        }

    public:
        LoanDecisionForm(
            AuthenticatedUser^ currentUser,
            int selectedLoanID)
        {
        this->Shown += gcnew System::EventHandler(this, &LoanDecisionForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
                throw gcnew ArgumentNullException("currentUser");

            if (selectedLoanID <= 0)
                throw gcnew ArgumentException("Invalid Loan ID.");

            actor = currentUser;
            loanID = selectedLoanID;
            InitializeUI();
        }
    };
}
