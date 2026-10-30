#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanListService.h"
#include "EligibilityHistoryForm.h"
#include "EligibilityPreviewForm.h"
#include "LoanDecisionForm.h"
#include "LoanDetailsForm.h"
#include "LoanRegistrationForm.h"
#include "EditLoanForm.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoanManagementForm : public Form
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
        TextBox^ searchBox;
        DataGridView^ loanGrid;
        Label^ countLabel;
        Label^ statusLabel;
        Button^ searchButton;
        Button^ clearButton;
        Button^ refreshButton;
        Button^ closeButton;
        Button^ eligibilityButton;
        Button^ decideLoanButton;
        Button^ viewDetailsButton;
        Button^ newLoanButton;
        Button^ editLoanButton;

        Label^ MakeLabel(
            String^ text, int x, int y, int w, int h)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(w, h);
            label->ForeColor = Color::White;
            label->BackColor = Color::Transparent;
            return label;
        }

        Button^ MakeButton(
            String^ text, int x, int y, int w, int h)
        {
            Button^ button = gcnew Button();
            button->Text = text;
            button->Location = Point(x, y);
            button->Size = Drawing::Size(w, h);
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->BackColor = Color::FromArgb(32, 164, 94);
            button->ForeColor = Color::White;
            button->Cursor = Cursors::Hand;
            return button;
        }

        void ConfigureColumns()
        {
            if (loanGrid->Columns->Count == 0)
                return;

            if (loanGrid->Columns->Contains("LoanID"))
                loanGrid->Columns["LoanID"]->Visible = false;

            if (loanGrid->Columns->Contains("LoanCode"))
                loanGrid->Columns["LoanCode"]->HeaderText =
                    "LOAN CODE";

            if (loanGrid->Columns->Contains("ApplicantCode"))
                loanGrid->Columns["ApplicantCode"]->HeaderText =
                    "APPLICANT CODE";

            if (loanGrid->Columns->Contains("ApplicantName"))
                loanGrid->Columns["ApplicantName"]->HeaderText =
                    "APPLICANT NAME";

            if (loanGrid->Columns->Contains("LoanType"))
                loanGrid->Columns["LoanType"]->HeaderText =
                    "LOAN TYPE";

            if (loanGrid->Columns->Contains("LoanPurpose"))
                loanGrid->Columns["LoanPurpose"]->HeaderText =
                    "PURPOSE";

            if (loanGrid->Columns->Contains("RequestedAmount"))
            {
                loanGrid->Columns["RequestedAmount"]->HeaderText =
                    "REQUESTED AMOUNT";
                loanGrid->Columns["RequestedAmount"]
                    ->DefaultCellStyle->Format = "N2";
            }

            if (loanGrid->Columns->Contains("AnnualInterestRate"))
            {
                loanGrid->Columns["AnnualInterestRate"]->HeaderText =
                    "INTEREST (%)";
                loanGrid->Columns["AnnualInterestRate"]
                    ->DefaultCellStyle->Format = "N2";
            }

            if (loanGrid->Columns->Contains("TermMonths"))
                loanGrid->Columns["TermMonths"]->HeaderText =
                    "TERM (MONTHS)";

            if (loanGrid->Columns->Contains("LoanStatus"))
                loanGrid->Columns["LoanStatus"]->HeaderText =
                    "STATUS";

            if (loanGrid->Columns->Contains("ApplicationDate"))
            {
                loanGrid->Columns["ApplicationDate"]->HeaderText =
                    "APPLICATION DATE";
                loanGrid->Columns["ApplicationDate"]
                    ->DefaultCellStyle->Format = "yyyy-MM-dd";
            }
        }

        void LoadLoans()
        {
            try
            {
                statusLabel->Text = "Loading loan applications...";

                DataTable^ loans =
                    LoanListService::GetAllLoans(
                        actor, searchBox->Text);

                loanGrid->DataSource = loans;

                ConfigureColumns();

                int total =
                    LoanListService::GetTotalLoans(actor);

                countLabel->Text =
                    "SHOWING: " + loans->Rows->Count.ToString()
                    + " / TOTAL: " + total.ToString();

                if (total == 0)
                {
                    statusLabel->Text =
                        "No loan applications yet. "
                        "Loan registration will be added in Phase 4.3.2.";
                }
                else if (loans->Rows->Count == 0)
                {
                    statusLabel->Text =
                        "No applications match your search.";
                }
                else
                {
                    statusLabel->Text =
                        "Loan applications loaded successfully.";
                }
            }
            catch (Exception^ ex)
            {
                loanGrid->DataSource = nullptr;
                countLabel->Text = "SHOWING: - / TOTAL: -";
                statusLabel->Text = "Unable to load loan applications.";

                MessageBox::Show(
                    this,
                    ex->Message,
                    "Loan Management",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnEditLoan(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (loanGrid->CurrentRow == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a loan application first.");

                DataRowView^ selected =
                    dynamic_cast<DataRowView^>(
                        loanGrid->CurrentRow->DataBoundItem);

                if (selected == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a valid loan application.");

                int loanID = Convert::ToInt32(
                    selected->Row["LoanID"]);

                EditLoanForm^ form =
                    gcnew EditLoanForm(actor, loanID);

                try
                {
                    if (form->ShowDialog(this) ==
                        System::Windows::Forms::DialogResult::OK)
                    {
                        LoadLoans();
                    }
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this, ex->Message,
                    "Edit Loan Application",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnNewLoan(Object^ sender, EventArgs^ e)
        {
            try
            {
                LoanRegistrationForm^ form =
                    gcnew LoanRegistrationForm(actor);

                try
                {
                    if (form->ShowDialog(this) ==
                        System::Windows::Forms::DialogResult::OK)
                    {
                        LoadLoans();
                    }
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this, ex->Message,
                    "Loan Management",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnViewLoanDetails(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (loanGrid->CurrentRow == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a loan first.");

                DataRowView^ row =
                    dynamic_cast<DataRowView^>(
                        loanGrid->CurrentRow->DataBoundItem);

                if (row == nullptr)
                    throw gcnew InvalidOperationException(
                        "No valid loan selected.");

                int id = Convert::ToInt32(row->Row["LoanID"]);

                LoanDetailsForm^ form =
                    gcnew LoanDetailsForm(actor, id);

                try
                {
                    form->ShowDialog(this);
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this, ex->Message,
                    "Loan Details",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
                void OnDecideLoan(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (loanGrid->CurrentRow == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a loan first.");

                DataRowView^ selected =
                    dynamic_cast<DataRowView^>(
                        loanGrid->CurrentRow->DataBoundItem);

                if (selected == nullptr)
                    throw gcnew InvalidOperationException(
                        "No valid loan selected.");

                int loanID = Convert::ToInt32(
                    selected->Row["LoanID"]);

                LoanDecisionForm^ form =
                    gcnew LoanDecisionForm(actor, loanID);

                try
                {
                    if (form->ShowDialog(this) ==
                        System::Windows::Forms::DialogResult::OK)
                    {
                        LoadLoans();
                    }
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this, ex->Message,
                    "Loan Decision",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnAssessEligibility(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (loanGrid->CurrentRow == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a loan application first.");

                DataRowView^ selected =
                    dynamic_cast<DataRowView^>(
                        loanGrid->CurrentRow->DataBoundItem);

                if (selected == nullptr)
                    throw gcnew InvalidOperationException(
                        "Please select a valid loan row.");

                int loanID = Convert::ToInt32(
                    selected->Row["LoanID"]);

                EligibilityPreviewForm^ form =
                    gcnew EligibilityPreviewForm(
                        actor, loanID);

                try
                {
                    form->ShowDialog(this);
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this,
                    ex->Message,
                    "Eligibility Assessment",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
                void OnViewEligibilityHistory(Object^ sender, EventArgs^ e)
        {
            try
            {
                if (loanGrid->CurrentRow == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a loan application first.");

                DataRowView^ selected =
                    dynamic_cast<DataRowView^>(
                        loanGrid->CurrentRow->DataBoundItem);

                if (selected == nullptr)
                    throw gcnew InvalidOperationException(
                        "Select a valid loan record.");

                int selectedLoanID = Convert::ToInt32(
                    selected->Row["LoanID"]);

                EligibilityHistoryForm^ form =
                    gcnew EligibilityHistoryForm(
                        actor, selectedLoanID);

                try
                {
                    form->ShowDialog(this);
                }
                finally
                {
                    delete form;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    this,
                    ex->Message,
                    "Eligibility History",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnSearch(Object^ sender, EventArgs^ e)
        {
            LoadLoans();
        }

        void OnRefresh(Object^ sender, EventArgs^ e)
        {
            LoadLoans();
        }

        void OnClear(Object^ sender, EventArgs^ e)
        {
            searchBox->Clear();
            LoadLoans();
        }

        void OnSearchKeyDown(
            Object^ sender, KeyEventArgs^ e)
        {
            if (e->KeyCode == Keys::Enter)
            {
                e->SuppressKeyPress = true;
                LoadLoans();
            }
        }

        void OnShown(Object^ sender, EventArgs^ e)
        {
            LoadLoans();
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void InitializeUI()
        {
            this->Text = "Group R - Loan Application Management";
            this->ClientSize = Drawing::Size(1120, 710);
            this->MinimumSize = Drawing::Size(900, 650);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(17, 22, 21);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);

            Label^ title = MakeLabel(
                "LOAN APPLICATION MANAGEMENT",
                30, 22, 1020, 45);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 20.0f, FontStyle::Bold);

            Label^ subtitle = MakeLabel(
                "View, search and monitor registered loan applications.",
                32, 75, 950, 27);
            subtitle->ForeColor =
                Color::FromArgb(175, 195, 184);

            Label^ searchLabel =
                MakeLabel("SEARCH LOANS", 32, 130, 300, 26);

            searchBox = gcnew TextBox();
            searchBox->Location = Point(32, 162);
            searchBox->Size = Drawing::Size(500, 36);
            searchBox->MaxLength = 100;
            searchBox->BorderStyle = BorderStyle::FixedSingle;
            searchBox->BackColor = Color::FromArgb(34, 42, 39);
            searchBox->ForeColor = Color::White;
            searchBox->Font =
                gcnew Drawing::Font("Segoe UI", 11.0f);

            searchButton =
                MakeButton("SEARCH", 548, 160, 155, 39);

            clearButton =
                MakeButton("CLEAR", 715, 160, 145, 39);
            clearButton->BackColor =
                Color::FromArgb(67, 74, 70);

            refreshButton =
                MakeButton("REFRESH", 870, 160, 200, 39);

            editLoanButton =
                MakeButton("EDIT SELECTED", 690, 220, 165, 38);
            newLoanButton =
                MakeButton("NEW LOAN", 870, 220, 200, 38);
            viewDetailsButton =
                MakeButton("VIEW DETAILS", 505, 220, 170, 38);
            decideLoanButton =
                MakeButton("DECIDE LOAN", 316, 213, 174, 38);
            eligibilityButton =
                MakeButton("ASSESS ELIGIBILITY", 32, 215, 270, 38);
            countLabel =
                MakeLabel("SHOWING: 0 / TOTAL: 0", 32, 231, 700, 30);
            countLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f, FontStyle::Bold);
            countLabel->ForeColor =
                Color::FromArgb(98, 225, 145);

            loanGrid = gcnew DataGridView();
            loanGrid->Location = Point(32, 276);
            loanGrid->Size = Drawing::Size(1038, 334);
            loanGrid->Anchor =
                AnchorStyles::Top | AnchorStyles::Bottom |
                AnchorStyles::Left | AnchorStyles::Right;

            loanGrid->ReadOnly = true;
            loanGrid->AllowUserToAddRows = false;
            loanGrid->AllowUserToDeleteRows = false;
            loanGrid->AllowUserToResizeRows = false;
            loanGrid->RowHeadersVisible = false;
            loanGrid->MultiSelect = false;
            loanGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            loanGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::DisplayedCells;
            loanGrid->BackgroundColor =
                Color::FromArgb(25, 32, 29);
            loanGrid->GridColor =
                Color::FromArgb(52, 65, 57);
            loanGrid->EnableHeadersVisualStyles = false;

            loanGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(29, 80, 51);
            loanGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            loanGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(29, 36, 32);
            loanGrid->DefaultCellStyle->ForeColor =
                Color::White;
            loanGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(39, 127, 78);
            loanGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            statusLabel =
                MakeLabel("Ready.", 32, 631, 700, 42);
            statusLabel->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Left;
            statusLabel->ForeColor =
                Color::FromArgb(170, 210, 180);

            closeButton =
                MakeButton("CLOSE", 912, 628, 158, 42);
            closeButton->BackColor =
                Color::FromArgb(68, 74, 72);
            closeButton->Anchor =
                AnchorStyles::Bottom | AnchorStyles::Right;

            this->Controls->Add(title);
            this->Controls->Add(subtitle);
            this->Controls->Add(searchLabel);
            this->Controls->Add(searchBox);
            this->Controls->Add(searchButton);
            this->Controls->Add(clearButton);
            this->Controls->Add(refreshButton);
            this->Controls->Add(eligibilityButton);
            this->Controls->Add(decideLoanButton);
            this->Controls->Add(viewDetailsButton);
            this->Controls->Add(newLoanButton);
            this->Controls->Add(editLoanButton);
            countLabel->Width = 550;
                        countLabel->Location = Point(32, 254);
            countLabel->Size = Drawing::Size(460, 22);
            this->Controls->Add(countLabel);
            this->Controls->Add(loanGrid);
            if (loanGrid->ContextMenuStrip == nullptr)
            {
                loanGrid->ContextMenuStrip =
                    gcnew System::Windows::Forms::ContextMenuStrip();
            }

            ToolStripMenuItem^ eligibilityHistoryItem =
                gcnew ToolStripMenuItem(
                    "VIEW ELIGIBILITY HISTORY");

            eligibilityHistoryItem->Click +=
                gcnew EventHandler(
                    this,
                    &LoanManagementForm::OnViewEligibilityHistory);

            loanGrid->ContextMenuStrip->Items->Add(
                eligibilityHistoryItem);

            this->Controls->Add(statusLabel);
            this->Controls->Add(closeButton);

            editLoanButton->Click += gcnew EventHandler(
                this, &LoanManagementForm::OnEditLoan);
            newLoanButton->Click += gcnew EventHandler(
                this, &LoanManagementForm::OnNewLoan);
            viewDetailsButton->Click += gcnew EventHandler(
                this, &LoanManagementForm::OnViewLoanDetails);
            decideLoanButton->Click += gcnew EventHandler(
                this, &LoanManagementForm::OnDecideLoan);
            eligibilityButton->Click += gcnew EventHandler(
                this, &LoanManagementForm::OnAssessEligibility);
            searchButton->Click +=
                gcnew EventHandler(
                    this, &LoanManagementForm::OnSearch);

            clearButton->Click +=
                gcnew EventHandler(
                    this, &LoanManagementForm::OnClear);

            refreshButton->Click +=
                gcnew EventHandler(
                    this, &LoanManagementForm::OnRefresh);

            closeButton->Click +=
                gcnew EventHandler(
                    this, &LoanManagementForm::OnClose);

            searchBox->KeyDown +=
                gcnew KeyEventHandler(
                    this, &LoanManagementForm::OnSearchKeyDown);

            this->Shown +=
                gcnew EventHandler(
                    this, &LoanManagementForm::OnShown);
        }

    public:
        LoanManagementForm(AuthenticatedUser^ currentUser)
        {
        this->Shown += gcnew System::EventHandler(this, &LoanManagementForm::ApplyPhase95ThemeOnShown);

            if (currentUser == nullptr)
            {
                throw gcnew ArgumentNullException(
                    "currentUser");
            }

            actor = currentUser;
            InitializeUI();
        }
    };
}







