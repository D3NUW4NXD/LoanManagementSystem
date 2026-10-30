#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanDetailsService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoanDetailsForm : public Form
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
        TableLayoutPanel^ detailTable;
        DataGridView^ guarantorGrid;
        Label^ titleLabel;
        Label^ statusLabel;
        Label^ guarantorCount;
        Button^ closeButton;

        String^ DisplayValue(Object^ value)
        {
            if (value == nullptr || value == DBNull::Value)
                return "-";

            String^ result = Convert::ToString(value);
            return String::IsNullOrWhiteSpace(result)
                ? "-" : result;
        }

        void AddDetail(String^ label, String^ value)
        {
            int row = detailTable->RowCount++;
            detailTable->RowStyles->Add(
                gcnew RowStyle(SizeType::AutoSize));

            Label^ key = gcnew Label();
            key->Text = label;
            key->ForeColor = Color::FromArgb(140, 215, 170);
            key->AutoSize = true;
            key->Margin = System::Windows::Forms::Padding(8, 6, 8, 6);

            Label^ val = gcnew Label();
            val->Text = value;
            val->ForeColor = Color::White;
            val->AutoSize = true;
            val->MaximumSize = Drawing::Size(560, 0);
            val->Margin = System::Windows::Forms::Padding(8, 6, 8, 6);

            detailTable->Controls->Add(key, 0, row);
            detailTable->Controls->Add(val, 1, row);
        }

        void LoadDetails()
        {
            try
            {
                DataSet^ data =
                    LoanDetailsService::GetDetails(actor, loanID);

                DataRow^ r = data->Tables["Loan"]->Rows[0];

                titleLabel->Text =
                    "LOAN DETAILS - " +
                    DisplayValue(r["LoanCode"]);

                detailTable->SuspendLayout();
                detailTable->Controls->Clear();
                detailTable->RowStyles->Clear();
                detailTable->RowCount = 0;

                AddDetail("Loan Code", DisplayValue(r["LoanCode"]));
                AddDetail("Applicant Code", DisplayValue(r["ApplicantCode"]));
                AddDetail("Applicant", DisplayValue(r["ApplicantName"]));
                AddDetail("Applicant Phone", DisplayValue(r["ApplicantPhone"]));
                AddDetail("Applicant Email", DisplayValue(r["ApplicantEmail"]));
                AddDetail("Loan Type", DisplayValue(r["LoanType"]));
                AddDetail("Purpose", DisplayValue(r["LoanPurpose"]));

                AddDetail(
                    "Requested Amount (LKR)",
                    Convert::ToDecimal(r["RequestedAmount"]).ToString("N2"));

                AddDetail(
                    "Annual Interest (%)",
                    Convert::ToDecimal(r["AnnualInterestRate"]).ToString("N2"));

                AddDetail("Term (Months)", DisplayValue(r["TermMonths"]));
                AddDetail("Status", DisplayValue(r["LoanStatus"]));
                AddDetail("Applied On", DisplayValue(r["ApplicationDate"]));
                AddDetail("Created By", DisplayValue(r["CreatedBy"]));
                AddDetail("Approved Amount", DisplayValue(r["ApprovedAmount"]));
                AddDetail("Decision By", DisplayValue(r["DecisionBy"]));
                AddDetail("Decision At", DisplayValue(r["DecisionAt"]));
                AddDetail("Rejection Reason", DisplayValue(r["RejectionReason"]));
                AddDetail("Disbursement Date", DisplayValue(r["DisbursementDate"]));

                detailTable->ResumeLayout();

                DataTable^ guarantors =
                    data->Tables["Guarantors"];

                guarantorGrid->DataSource = guarantors;

                guarantorCount->Text =
                    "LINKED GUARANTORS: " +
                    guarantors->Rows->Count.ToString();

                statusLabel->Text =
                    "Loan details loaded successfully.";
            }
            catch (Exception^ ex)
            {
                statusLabel->Text =
                    "Unable to load loan details.";

                MessageBox::Show(
                    this, ex->Message,
                    "Loan Details",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadDetails();
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void InitializeUI()
        {
            this->Text = "Group R - Loan Details";
            this->ClientSize = Drawing::Size(980, 760);
            this->MinimumSize = Drawing::Size(850, 680);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(18, 23, 21);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);

            titleLabel = gcnew Label();
            titleLabel->Text = "LOAN DETAILS";
            titleLabel->Location = Point(28, 18);
            titleLabel->Size = Drawing::Size(900, 48);
            titleLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 19.0f, FontStyle::Bold);
            titleLabel->ForeColor = Color::White;

            Panel^ scrollPanel = gcnew Panel();
            scrollPanel->Location = Point(28, 85);
            scrollPanel->Size = Drawing::Size(920, 420);
            scrollPanel->Anchor =
                AnchorStyles::Top | AnchorStyles::Bottom |
                AnchorStyles::Left | AnchorStyles::Right;
            scrollPanel->AutoScroll = true;
            scrollPanel->BackColor =
                Color::FromArgb(27, 35, 30);

            detailTable = gcnew TableLayoutPanel();
            detailTable->Dock = DockStyle::Top;
            detailTable->AutoSize = true;
            detailTable->AutoSizeMode =
                System::Windows::Forms::AutoSizeMode::GrowAndShrink;
            detailTable->ColumnCount = 2;
            detailTable->ColumnStyles->Add(
                gcnew ColumnStyle(SizeType::Absolute, 230));
            detailTable->ColumnStyles->Add(
                gcnew ColumnStyle(SizeType::Percent, 100));
            detailTable->Padding = System::Windows::Forms::Padding(12);

            scrollPanel->Controls->Add(detailTable);

            guarantorCount = gcnew Label();
            guarantorCount->Text = "LINKED GUARANTORS: 0";
            guarantorCount->Location = Point(28, 525);
            guarantorCount->Size = Drawing::Size(800, 30);
            guarantorCount->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Left;
            guarantorCount->ForeColor =
                Color::FromArgb(110, 225, 150);

            guarantorGrid = gcnew DataGridView();
            guarantorGrid->Location = Point(28, 560);
            guarantorGrid->Size = Drawing::Size(920, 125);
            guarantorGrid->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Left |
                AnchorStyles::Right;
            guarantorGrid->ReadOnly = true;
            guarantorGrid->AllowUserToAddRows = false;
            guarantorGrid->AllowUserToDeleteRows = false;
            guarantorGrid->RowHeadersVisible = false;
            guarantorGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            guarantorGrid->BackgroundColor =
                Color::FromArgb(27, 35, 30);
            guarantorGrid->GridColor =
                Color::FromArgb(55, 70, 60);
            guarantorGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(30, 40, 34);
            guarantorGrid->DefaultCellStyle->ForeColor =
                Color::White;
            guarantorGrid->EnableHeadersVisualStyles = false;
            guarantorGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(30, 90, 53);
            guarantorGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            statusLabel = gcnew Label();
            statusLabel->Text = "Ready.";
            statusLabel->Location = Point(28, 704);
            statusLabel->Size = Drawing::Size(690, 35);
            statusLabel->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Left;
            statusLabel->ForeColor =
                Color::FromArgb(155, 210, 170);

            closeButton = gcnew Button();
            closeButton->Text = "CLOSE";
            closeButton->Location = Point(800, 700);
            closeButton->Size = Drawing::Size(148, 40);
            closeButton->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Right;
            closeButton->BackColor =
                Color::FromArgb(64, 72, 67);
            closeButton->ForeColor = Color::White;
            closeButton->FlatStyle = FlatStyle::Flat;
            closeButton->FlatAppearance->BorderSize = 0;

            this->Controls->Add(titleLabel);
            this->Controls->Add(scrollPanel);
            this->Controls->Add(guarantorCount);
            this->Controls->Add(guarantorGrid);
            this->Controls->Add(statusLabel);
            this->Controls->Add(closeButton);

            closeButton->Click += gcnew EventHandler(
                this, &LoanDetailsForm::OnClose);

            this->Shown += gcnew EventHandler(
                this, &LoanDetailsForm::OnShown);
        }

    public:
        LoanDetailsForm(
            AuthenticatedUser^ currentUser,
            int selectedLoanID)
        {
        this->Shown += gcnew System::EventHandler(this, &LoanDetailsForm::ApplyPhase95ThemeOnShown);

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

