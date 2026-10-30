#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "LoanGuarantorLinkService.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoanGuarantorLinkForm : public Form
    {
    // GROUPR_PHASE94_V2
protected:
    virtual void OnShown(System::EventArgs^ e) override
    {
        System::Windows::Forms::Form::OnShown(e);
        GroupRModernTheme::Apply(this);
    }

    private:
        AuthenticatedUser^ actor;
        ComboBox^ loanBox;
        ComboBox^ guarantorBox;
        DataGridView^ linkedGrid;
        Label^ applicantLabel;
        Label^ statusLabel;
        Label^ countLabel;
        Button^ refreshButton;
        Button^ assignButton;
        Button^ closeButton;
        bool isLoading;

        Label^ MakeLabel(String^ text, int x, int y, int w, int h)
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
            button->BackColor = Color::FromArgb(34, 175, 100);
            button->ForeColor = Color::White;
            button->Cursor = Cursors::Hand;
            return button;
        }

        int SelectedID(ComboBox^ combo, String^ column)
        {
            DataRowView^ row =
                dynamic_cast<DataRowView^>(combo->SelectedItem);

            if (row == nullptr ||
                row->Row[column] == DBNull::Value)
                return 0;

            return Convert::ToInt32(row->Row[column]);
        }

        void ClearDetails(String^ message)
        {
            applicantLabel->Text = "APPLICANT: -";
            guarantorBox->DataSource = nullptr;
            linkedGrid->DataSource = nullptr;
            countLabel->Text = "LINKED GUARANTORS: 0";
            statusLabel->Text = message;
            assignButton->Enabled = false;
        }

        void ShowProblem(Exception^ ex)
        {
            statusLabel->Text = "ERROR: " + ex->Message;
            MessageBox::Show(
                this,
                ex->Message,
                "Guarantor Loan Linking",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);
        }

        void LoadLoanDetails()
        {
            if (isLoading)
                return;

            int loanID = SelectedID(loanBox, "LoanID");

            if (loanID <= 0)
            {
                ClearDetails(
                    "No loan selected. Create a loan application first.");
                return;
            }

            try
            {
                DataRowView^ selected =
                    dynamic_cast<DataRowView^>(
                        loanBox->SelectedItem);

                if (selected != nullptr)
                {
                    applicantLabel->Text =
                        "APPLICANT: " +
                        Convert::ToString(
                            selected->Row["ApplicantName"]);
                }

                DataTable^ linked =
                    LoanGuarantorLinkService::GetLinkedGuarantors(
                        actor, loanID);

                linkedGrid->DataSource = linked;

                countLabel->Text =
                    "LINKED GUARANTORS: " +
                    linked->Rows->Count.ToString();

                DataTable^ eligible =
                    LoanGuarantorLinkService::GetEligibleGuarantors(
                        actor, loanID);

                isLoading = true;

                try
                {
                    guarantorBox->DataSource = nullptr;
                    guarantorBox->DisplayMember = "DisplayName";
                    guarantorBox->ValueMember = "GuarantorID";
                    guarantorBox->DataSource = eligible;

                    if (eligible->Rows->Count > 0)
                        guarantorBox->SelectedIndex = 0;
                }
                finally
                {
                    isLoading = false;
                }

                assignButton->Enabled =
                    eligible->Rows->Count > 0;

                statusLabel->Text =
                    eligible->Rows->Count > 0
                    ? "Select an eligible guarantor and click LINK."
                    : "No additional eligible guarantors for this loan.";
            }
            catch (Exception^ ex)
            {
                ClearDetails("Could not load loan details.");
                ShowProblem(ex);
            }
        }

        void RefreshLoans()
        {
            try
            {
                isLoading = true;

                DataTable^ loans =
                    LoanGuarantorLinkService::GetLoans(actor);

                loanBox->DataSource = nullptr;
                loanBox->DisplayMember = "DisplayName";
                loanBox->ValueMember = "LoanID";
                loanBox->DataSource = loans;

                if (loans->Rows->Count > 0)
                    loanBox->SelectedIndex = 0;
            }
            catch (Exception^ ex)
            {
                ClearDetails("Could not load loans.");
                ShowProblem(ex);
                return;
            }
            finally
            {
                isLoading = false;
            }

            LoadLoanDetails();
        }

        void OnFormShown(Object^ sender, EventArgs^ e)
        {
            RefreshLoans();
        }

        void OnRefresh(Object^ sender, EventArgs^ e)
        {
            RefreshLoans();
        }

        void OnLoanChanged(Object^ sender, EventArgs^ e)
        {
            LoadLoanDetails();
        }

        void OnAssign(Object^ sender, EventArgs^ e)
        {
            int loanID = SelectedID(loanBox, "LoanID");
            int guarantorID =
                SelectedID(guarantorBox, "GuarantorID");

            if (loanID <= 0 || guarantorID <= 0)
            {
                MessageBox::Show(
                    this,
                    "Please select a loan and an eligible guarantor.",
                    "Selection Required",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                return;
            }

            try
            {
                LoanGuarantorLinkService::Assign(
                    actor, loanID, guarantorID);

                LoadLoanDetails();

                statusLabel->Text =
                    "SUCCESS: Guarantor linked to loan.";

                MessageBox::Show(
                    this,
                    "Guarantor linked successfully.",
                    "Success",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
            }
            catch (Exception^ ex)
            {
                ShowProblem(ex);
            }
        }

        void OnClose(Object^ sender, EventArgs^ e)
        {
            this->Close();
        }

        void InitializeUI()
        {
            this->Text = "Group R - Link Guarantor to Loan";
            this->ClientSize = Drawing::Size(1000, 680);
            this->MinimumSize = Drawing::Size(850, 680);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(18, 22, 22);
            this->ForeColor = Color::White;
            this->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);

            Label^ title = MakeLabel(
                "LINK GUARANTOR TO LOAN", 30, 20, 800, 40);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 19.0f, FontStyle::Bold);

            Label^ description = MakeLabel(
                "Assign an existing guarantor to the correct applicant's loan.",
                32, 66, 850, 28);

            Label^ loanLabel =
                MakeLabel("SELECT LOAN APPLICATION", 32, 111, 450, 28);

            loanBox = gcnew ComboBox();
            loanBox->Location = Point(32, 143);
            loanBox->Size = Drawing::Size(615, 35);
            loanBox->DropDownStyle = ComboBoxStyle::DropDownList;
            loanBox->FlatStyle = FlatStyle::Flat;

            refreshButton =
                MakeButton("REFRESH LOANS", 665, 141, 290, 38);

            applicantLabel =
                MakeLabel("APPLICANT: -", 32, 193, 900, 30);
            applicantLabel->ForeColor =
                Color::FromArgb(100, 225, 150);

            Label^ guarantorLabel =
                MakeLabel("SELECT ELIGIBLE GUARANTOR", 32, 238, 450, 28);

            guarantorBox = gcnew ComboBox();
            guarantorBox->Location = Point(32, 270);
            guarantorBox->Size = Drawing::Size(615, 35);
            guarantorBox->DropDownStyle =
                ComboBoxStyle::DropDownList;
            guarantorBox->FlatStyle = FlatStyle::Flat;

            assignButton =
                MakeButton("LINK GUARANTOR", 665, 268, 290, 40);
            assignButton->Enabled = false;

            countLabel =
                MakeLabel("LINKED GUARANTORS: 0", 32, 331, 600, 30);
            countLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f, FontStyle::Bold);

            linkedGrid = gcnew DataGridView();
            linkedGrid->Location = Point(32, 369);
            linkedGrid->Size = Drawing::Size(923, 216);
            linkedGrid->ReadOnly = true;
            linkedGrid->AllowUserToAddRows = false;
            linkedGrid->AllowUserToDeleteRows = false;
            linkedGrid->MultiSelect = false;
            linkedGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            linkedGrid->RowHeadersVisible = false;
            linkedGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            linkedGrid->BackgroundColor =
                Color::FromArgb(28, 34, 34);
            linkedGrid->GridColor =
                Color::FromArgb(55, 65, 65);
            linkedGrid->EnableHeadersVisualStyles = false;
            linkedGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(35, 75, 55);
            linkedGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;
            linkedGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(30, 36, 36);
            linkedGrid->DefaultCellStyle->ForeColor =
                Color::White;
            linkedGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(35, 120, 75);
            linkedGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            statusLabel =
                MakeLabel("Ready.", 32, 603, 680, 45);
            statusLabel->ForeColor =
                Color::FromArgb(170, 220, 185);

            closeButton =
                MakeButton("CLOSE", 810, 602, 145, 39);
            closeButton->BackColor =
                Color::FromArgb(65, 70, 70);

            this->Controls->Add(title);
            this->Controls->Add(description);
            this->Controls->Add(loanLabel);
            this->Controls->Add(loanBox);
            this->Controls->Add(refreshButton);
            this->Controls->Add(applicantLabel);
            this->Controls->Add(guarantorLabel);
            this->Controls->Add(guarantorBox);
            this->Controls->Add(assignButton);
            this->Controls->Add(countLabel);
            this->Controls->Add(linkedGrid);
            this->Controls->Add(statusLabel);
            this->Controls->Add(closeButton);

            loanBox->SelectedIndexChanged +=
                gcnew EventHandler(
                    this, &LoanGuarantorLinkForm::OnLoanChanged);

            refreshButton->Click +=
                gcnew EventHandler(
                    this, &LoanGuarantorLinkForm::OnRefresh);

            assignButton->Click +=
                gcnew EventHandler(
                    this, &LoanGuarantorLinkForm::OnAssign);

            closeButton->Click +=
                gcnew EventHandler(
                    this, &LoanGuarantorLinkForm::OnClose);

            this->Shown +=
                gcnew EventHandler(
                    this, &LoanGuarantorLinkForm::OnFormShown);
        }

    public:
        LoanGuarantorLinkForm(AuthenticatedUser^ currentUser)
        {
            if (currentUser == nullptr)
                throw gcnew ArgumentNullException("currentUser");

            actor = currentUser;
            isLoading = false;
            InitializeUI();
        }
    };
}
