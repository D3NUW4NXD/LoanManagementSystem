#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "EligibilityHistoryService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class EligibilityHistoryForm : public Form
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
        Label^ summaryLabel;
        Label^ statusLabel;

        DataGridView^ historyGrid;

        Button^ refreshButton;
        Button^ closeButton;

        void LoadHistory()
        {
            refreshButton->Enabled = false;
            statusLabel->Text = "Loading assessment history...";

            try
            {
                DataTable^ history =
                    EligibilityHistoryService::GetHistory(
                        actor, loanID);

                historyGrid->DataSource = nullptr;
                historyGrid->DataSource = history;

                summaryLabel->Text =
                    "RECORDS DISPLAYED: " +
                    history->Rows->Count.ToString() +
                    " (maximum 200)";

                statusLabel->Text =
                    history->Rows->Count == 0
                    ? "No saved eligibility assessments found."
                    : "Assessment records loaded successfully.";

                statusLabel->ForeColor =
                    Color::FromArgb(125, 225, 160);
            }
            catch (Exception^ ex)
            {
                historyGrid->DataSource = nullptr;
                summaryLabel->Text = "RECORDS DISPLAYED: 0";
                statusLabel->Text =
                    "Unable to load assessment history.";
                statusLabel->ForeColor =
                    Color::FromArgb(255, 165, 130);

                MessageBox::Show(
                    this,
                    ex->Message,
                    "Eligibility History",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
            finally
            {
                refreshButton->Enabled = true;
            }
        }

        void OnRefresh(Object^ sender, EventArgs^ e)
        {
            LoadHistory();
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadHistory();
        }

        void InitializeUI()
        {
            this->Text =
                "Group R - Eligibility Assessment History";

            this->ClientSize = Drawing::Size(1000, 640);
            this->MinimumSize = Drawing::Size(760, 500);
            this->StartPosition =
                FormStartPosition::CenterParent;

            this->BackColor =
                Color::FromArgb(18, 23, 21);
            this->ForeColor = Color::White;
            this->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            titleLabel = gcnew Label();
            titleLabel->Text =
                "ELIGIBILITY ASSESSMENT HISTORY";
            titleLabel->Location = Point(28, 22);
            titleLabel->Size = Drawing::Size(930, 45);
            titleLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 19.0f, FontStyle::Bold);
            titleLabel->Anchor =
                AnchorStyles::Top |
                AnchorStyles::Left |
                AnchorStyles::Right;

            summaryLabel = gcnew Label();
            summaryLabel->Text = "RECORDS DISPLAYED: -";
            summaryLabel->Location = Point(30, 82);
            summaryLabel->Size = Drawing::Size(800, 30);
            summaryLabel->ForeColor =
                Color::FromArgb(115, 225, 155);

            historyGrid = gcnew DataGridView();
            historyGrid->Location = Point(28, 130);
            historyGrid->Size = Drawing::Size(944, 365);
            historyGrid->Anchor =
                AnchorStyles::Top |
                AnchorStyles::Bottom |
                AnchorStyles::Left |
                AnchorStyles::Right;

            historyGrid->ReadOnly = true;
            historyGrid->AllowUserToAddRows = false;
            historyGrid->AllowUserToDeleteRows = false;
            historyGrid->AllowUserToResizeRows = false;

            historyGrid->AutoGenerateColumns = true;
            historyGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::DisplayedCells;

            historyGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            historyGrid->MultiSelect = false;
            historyGrid->RowHeadersVisible = false;

            historyGrid->BackgroundColor =
                Color::FromArgb(27, 36, 30);
            historyGrid->GridColor =
                Color::FromArgb(54, 68, 58);
            historyGrid->EnableHeadersVisualStyles = false;

            historyGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(35, 103, 58);
            historyGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            historyGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(29, 39, 33);
            historyGrid->DefaultCellStyle->ForeColor =
                Color::White;

            historyGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(43, 128, 73);
            historyGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            statusLabel = gcnew Label();
            statusLabel->Text = "Ready.";
            statusLabel->Location = Point(30, 514);
            statusLabel->Size = Drawing::Size(920, 32);
            statusLabel->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Left |
                AnchorStyles::Right;

            refreshButton = gcnew Button();
            refreshButton->Text = "REFRESH HISTORY";
            refreshButton->Location = Point(28, 570);
            refreshButton->Size = Drawing::Size(270, 42);
            refreshButton->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Left;
            refreshButton->BackColor =
                Color::FromArgb(32, 163, 92);
            refreshButton->ForeColor = Color::White;
            refreshButton->FlatStyle = FlatStyle::Flat;
            refreshButton->FlatAppearance->BorderSize = 0;

            closeButton = gcnew Button();
            closeButton->Text = "CLOSE";
            closeButton->Location = Point(754, 570);
            closeButton->Size = Drawing::Size(218, 42);
            closeButton->Anchor =
                AnchorStyles::Bottom |
                AnchorStyles::Right;
            closeButton->BackColor =
                Color::FromArgb(67, 74, 70);
            closeButton->ForeColor = Color::White;
            closeButton->FlatStyle = FlatStyle::Flat;
            closeButton->FlatAppearance->BorderSize = 0;

            this->Controls->Add(titleLabel);
            this->Controls->Add(summaryLabel);
            this->Controls->Add(historyGrid);
            this->Controls->Add(statusLabel);
            this->Controls->Add(refreshButton);
            this->Controls->Add(closeButton);

            refreshButton->Click += gcnew EventHandler(
                this, &EligibilityHistoryForm::OnRefresh);

            closeButton->Click += gcnew EventHandler(
                this, &EligibilityHistoryForm::OnClose);

            this->Shown += gcnew EventHandler(
                this, &EligibilityHistoryForm::OnShown);

            this->CancelButton = closeButton;
        }

    public:
        EligibilityHistoryForm(
            AuthenticatedUser^ currentUser,
            int selectedLoanID)
        {
        this->Shown += gcnew System::EventHandler(this, &EligibilityHistoryForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
                throw gcnew ArgumentNullException(
                    "currentUser");

            if (selectedLoanID <= 0)
                throw gcnew ArgumentException(
                    "Invalid Loan ID.");

            actor = currentUser;
            loanID = selectedLoanID;

            InitializeUI();
        }
    };
}
