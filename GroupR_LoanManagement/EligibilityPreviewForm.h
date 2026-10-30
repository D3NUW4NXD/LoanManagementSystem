#pragma once
#include "GroupRModernTheme.h"
#include "EligibilitySaveService.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "EligibilityPreviewService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class EligibilityPreviewForm : public Form
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
        Label^ statusLabel;
        Label^ noteLabel;
        Panel^ detailsPanel;
        Button^ refreshButton;
        Button^ closeButton;
        Button^ saveButton;

        int nextRow;

        void AddLine(String^ key, String^ value, bool emphasis)
        {
            int y = nextRow * 38 + 12;
            nextRow++;

            Label^ nameLabel = gcnew Label();
            nameLabel->Text = key;
            nameLabel->Location = Point(15, y);
            nameLabel->Size = Drawing::Size(305, 30);
            nameLabel->ForeColor =
                Color::FromArgb(165, 192, 175);

            Label^ valueLabel = gcnew Label();
            valueLabel->Text = value;
            valueLabel->Location = Point(326, y);
            valueLabel->Size = Drawing::Size(375, 32);
            valueLabel->ForeColor = emphasis
                ? Color::FromArgb(104, 237, 151)
                : Color::White;

            if (emphasis)
            {
                valueLabel->Font = gcnew Drawing::Font(
                    "Segoe UI", 11.0f, FontStyle::Bold);
            }

            detailsPanel->Controls->Add(nameLabel);
            detailsPanel->Controls->Add(valueLabel);
        }

        String^ FormatMoney(Decimal value)
        {
            return String::Format("LKR {0:N2}", value);
        }

        void LoadAssessment()
        {
            refreshButton->Enabled = false;
            saveButton->Enabled = false;
            statusLabel->Text = "Loading assessment...";

            try
            {
                DataTable^ table =
                    EligibilityPreviewService::GetLoanData(
                        actor, loanID);

                DataRow^ row = table->Rows[0];

                Decimal income = Convert::ToDecimal(
                    row["MonthlyIncome"]);

                Decimal existingDebt = Convert::ToDecimal(
                    row["ExistingMonthlyDebt"]);

                Decimal amount = Convert::ToDecimal(
                    row["RequestedAmount"]);

                Decimal rate = Convert::ToDecimal(
                    row["AnnualInterestRate"]);

                int months = Convert::ToInt32(
                    row["TermMonths"]);

                LoanEligibilityResult^ result =
                    LoanEligibilityCalculator::Calculate(
                        income, existingDebt,
                        amount, rate, months);

                titleLabel->Text =
                    "ELIGIBILITY ASSESSMENT - " +
                    Convert::ToString(row["LoanCode"]);

                detailsPanel->Controls->Clear();
                nextRow = 0;

                AddLine(
                    "Applicant",
                    Convert::ToString(row["ApplicantName"]),
                    false);

                AddLine(
                    "Applicant Code",
                    Convert::ToString(row["ApplicantCode"]),
                    false);

                AddLine(
                    "Loan Type",
                    Convert::ToString(row["LoanType"]),
                    false);

                AddLine(
                    "Loan Status",
                    Convert::ToString(row["LoanStatus"]),
                    false);

                AddLine(
                    "Monthly Income",
                    FormatMoney(result->MonthlyIncome),
                    false);

                AddLine(
                    "Existing Monthly Debt",
                    FormatMoney(result->ExistingMonthlyDebt),
                    false);

                AddLine(
                    "Requested Amount",
                    FormatMoney(result->RequestedAmount),
                    false);

                AddLine(
                    "Annual Interest Rate",
                    String::Format(
                        "{0:N2}%",
                        result->AnnualInterestRate),
                    false);

                AddLine(
                    "Loan Term",
                    result->TermMonths.ToString() + " months",
                    false);

                AddLine(
                    "Estimated Monthly Installment",
                    FormatMoney(result->EstimatedInstallment),
                    true);

                AddLine(
                    "Total Monthly Debt",
                    FormatMoney(result->TotalMonthlyDebt),
                    false);

                AddLine(
                    "Debt-to-Income Ratio",
                    String::Format(
                        "{0:N2}%",
                        result->DebtToIncomePercent),
                    true);

                AddLine(
                    "Remaining Monthly Income",
                    FormatMoney(result->RemainingMonthlyIncome),
                    true);

                String^ creditScore =
                    row["CreditScore"] == DBNull::Value
                    ? "Not recorded"
                    : Convert::ToString(row["CreditScore"]);

                AddLine(
                    "Recorded Credit Score",
                    creditScore,
                    false);

                noteLabel->Text = result->AssessmentNote;

                saveButton->Enabled = String::Equals(
                    Convert::ToString(row["LoanStatus"])->Trim(),
                    "Pending",
                    StringComparison::OrdinalIgnoreCase);

                statusLabel->Text =
                    "Assessment calculated from current "
                    "database values. No changes saved.";

                statusLabel->ForeColor =
                    Color::FromArgb(117, 225, 151);
            }
            catch (Exception^ ex)
            {
                statusLabel->Text =
                    "Assessment could not be calculated.";

                statusLabel->ForeColor =
                    Color::FromArgb(255, 165, 130);

                MessageBox::Show(
                    this,
                    ex->Message,
                    "Eligibility Assessment",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
            finally
            {
                refreshButton->Enabled = true;
            }
        }

        void OnSaveAssessment(Object^ sender, EventArgs^ e)
        {
            if (MessageBox::Show(
                this,
                "Save a new preliminary eligibility assessment? "
                "This will NOT approve the loan.",
                "Confirm Assessment",
                MessageBoxButtons::YesNo,
                MessageBoxIcon::Question) !=
                System::Windows::Forms::DialogResult::Yes)
                return;

            saveButton->Enabled = false;

            try
            {
                int assessmentID =
                    EligibilitySaveService::SaveAssessment(
                        actor, loanID);

                statusLabel->Text =
                    "Assessment saved. ID: " +
                    assessmentID.ToString();

                statusLabel->ForeColor =
                    Color::FromArgb(115, 230, 155);

                MessageBox::Show(
                    this,
                    "Assessment saved successfully.\n"
                    "Assessment ID: " + assessmentID.ToString() +
                    "\nResult: Needs Review"
                    "\nLoan status was not changed.",
                    "Save Successful",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this,
                    ex->Message,
                    "Assessment Save Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);

                LoadAssessment();
            }
        }
        void OnRefresh(Object^ sender, EventArgs^ e)
        {
            LoadAssessment();
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadAssessment();
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void InitializeUI()
        {
            this->Text = "Group R - Eligibility Assessment";
            this->ClientSize = Drawing::Size(770, 720);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->MinimumSize = Drawing::Size(750, 680);

            this->BackColor =
                Color::FromArgb(17, 23, 20);
            this->ForeColor = Color::White;
            this->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            titleLabel = gcnew Label();
            titleLabel->Text = "ELIGIBILITY ASSESSMENT";
            titleLabel->Location = Point(24, 24);
            titleLabel->Size = Drawing::Size(710, 46);
            titleLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 18.0f, FontStyle::Bold);
            titleLabel->ForeColor = Color::White;

            Label^ subtitle = gcnew Label();
            subtitle->Text =
                "READ-ONLY | Preliminary Affordability Review";
            subtitle->Location = Point(26, 77);
            subtitle->Size = Drawing::Size(700, 28);
            subtitle->ForeColor =
                Color::FromArgb(110, 225, 145);

            detailsPanel = gcnew Panel();
            detailsPanel->Location = Point(24, 125);
            detailsPanel->Size = Drawing::Size(720, 425);
            detailsPanel->Anchor =
                AnchorStyles::Top |
                AnchorStyles::Bottom |
                AnchorStyles::Left |
                AnchorStyles::Right;
            detailsPanel->AutoScroll = true;
            detailsPanel->BackColor =
                Color::FromArgb(28, 38, 32);

            noteLabel = gcnew Label();
            noteLabel->Location = Point(26, 567);
            noteLabel->Size = Drawing::Size(710, 48);
            noteLabel->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Left |
                AnchorStyles::Right;
            noteLabel->Text =
                "Preliminary calculation only. "
                "This is not an approval decision.";
            noteLabel->ForeColor =
                Color::FromArgb(195, 198, 176);

            statusLabel = gcnew Label();
            statusLabel->Location = Point(26, 625);
            statusLabel->Size = Drawing::Size(710, 28);
            statusLabel->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Left;
            statusLabel->Text = "Ready.";

            saveButton = gcnew Button();
            saveButton->Text = "SAVE ASSESSMENT";
            saveButton->Location = Point(252, 667);
            saveButton->Size = Drawing::Size(246, 40);
            saveButton->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Left;
            saveButton->BackColor =
                Color::FromArgb(29, 125, 188);
            saveButton->ForeColor = Color::White;
            saveButton->FlatStyle = FlatStyle::Flat;
            saveButton->FlatAppearance->BorderSize = 0;
            saveButton->Enabled = false;
            refreshButton = gcnew Button();
            refreshButton->Text = "RECALCULATE";
            refreshButton->Location = Point(26, 667);
            refreshButton->Size = Drawing::Size(210, 40);
            refreshButton->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Left;
            refreshButton->BackColor =
                Color::FromArgb(32, 160, 88);
            refreshButton->ForeColor = Color::White;
            refreshButton->FlatStyle = FlatStyle::Flat;
            refreshButton->FlatAppearance->BorderSize = 0;

            closeButton = gcnew Button();
            closeButton->Text = "CLOSE";
            closeButton->Location = Point(514, 667);
            closeButton->Size = Drawing::Size(230, 40);
            closeButton->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Right;
            closeButton->BackColor =
                Color::FromArgb(67, 75, 70);
            closeButton->ForeColor = Color::White;
            closeButton->FlatStyle = FlatStyle::Flat;
            closeButton->FlatAppearance->BorderSize = 0;

            this->Controls->Add(titleLabel);
            this->Controls->Add(subtitle);
            this->Controls->Add(detailsPanel);
            this->Controls->Add(noteLabel);
            this->Controls->Add(statusLabel);
            this->Controls->Add(refreshButton);
            this->Controls->Add(saveButton);
            this->Controls->Add(closeButton);

            saveButton->Click += gcnew EventHandler(
                this, &EligibilityPreviewForm::OnSaveAssessment);
            refreshButton->Click += gcnew EventHandler(
                this, &EligibilityPreviewForm::OnRefresh);

            closeButton->Click += gcnew EventHandler(
                this, &EligibilityPreviewForm::OnClose);

            this->Shown += gcnew EventHandler(
                this, &EligibilityPreviewForm::OnShown);

            this->CancelButton = closeButton;
        }

    public:
        EligibilityPreviewForm(
            AuthenticatedUser^ currentUser,
            int selectedLoanID)
        {
        this->Shown += gcnew System::EventHandler(this, &EligibilityPreviewForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
                throw gcnew ArgumentNullException(
                    "currentUser");

            if (selectedLoanID <= 0)
                throw gcnew ArgumentException(
                    "Invalid Loan ID.");

            actor = currentUser;
            loanID = selectedLoanID;
            nextRow = 0;

            InitializeUI();
        }
    };
}

