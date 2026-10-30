#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "PaymentReceiptForm.h"
#include "AuthenticationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class PaymentHistoryForm : public Form
    {

    // GROUPR_PHASE96_PREMIUM_UI
private:
    void ApplyPhase96ThemeOnShown(
        System::Object^ sender,
        System::EventArgs^ e)
    {
        ::GroupR_LoanManagement::GroupRModernTheme::Apply(this);
    }


    private:
        AuthenticatedUser^ actor;
        DataGridView^ grid;
        TextBox^ search;
        ComboBox^ loanFilter;
        Label^ countLabel;
        Button^ receiptButton;

        void LoadLoans()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT LoanID, LoanCode "
                    "FROM dbo.Loans ORDER BY LoanID DESC",
                    con);

                DataTable^ table = gcnew DataTable();
                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                adapter->Fill(table);

                DataRow^ all = table->NewRow();
                all["LoanID"] = 0;
                all["LoanCode"] = "All Loans";
                table->Rows->InsertAt(all, 0);

                loanFilter->DisplayMember = "LoanCode";
                loanFilter->ValueMember = "LoanID";
                loanFilter->DataSource = table;
            }
            finally
            {
                delete con;
            }
        }

        void LoadPayments()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT p.PaymentID AS [Payment ID], "
                    "p.PaymentReference AS [Reference], "
                    "l.LoanCode AS [Loan], "
                    "r.InstallmentNumber AS [Installment], "
                    "p.PaymentDate AS [Date], "
                    "p.PrincipalAmount AS [Principal], "
                    "p.InterestAmount AS [Interest], "
                    "(p.PrincipalAmount+p.InterestAmount) AS [Total], "
                    "p.PaymentMethod AS [Method], "
                    "p.TransactionReference AS [Transaction], "
                    "p.RecordedByUserID AS [Recorded By] "
                    "FROM dbo.Payments p "
                    "JOIN dbo.RepaymentSchedule r "
                    "ON r.RepaymentScheduleID=p.RepaymentScheduleID "
                    "JOIN dbo.Loans l ON l.LoanID=r.LoanID "
                    "WHERE (@loan=0 OR l.LoanID=@loan) "
                    "AND (@search='' "
                    "OR p.PaymentReference LIKE @pattern "
                    "OR l.LoanCode LIKE @pattern "
                    "OR p.TransactionReference LIKE @pattern) "
                    "ORDER BY p.PaymentDate DESC, p.PaymentID DESC",
                    con);

                int loanID = 0;

                if (loanFilter->SelectedValue != nullptr &&
                    dynamic_cast<DataRowView^>(
                        loanFilter->SelectedValue) == nullptr)
                {
                    try {
                        loanID = Convert::ToInt32(
                            loanFilter->SelectedValue);
                    }
                    catch (Exception^) {}
                }

                String^ term = search->Text->Trim();

                cmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                cmd->Parameters->Add(
                    "@search", SqlDbType::NVarChar, 200
                )->Value = term;

                cmd->Parameters->Add(
                    "@pattern", SqlDbType::NVarChar, 250
                )->Value = "%" + term + "%";

                DataTable^ table = gcnew DataTable();
                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                adapter->Fill(table);

                grid->DataSource = table;

                if (grid->Columns->Contains("Payment ID"))
                    grid->Columns["Payment ID"]->Visible = false;

                countLabel->Text =
                    table->Rows->Count.ToString() +
                    " payment record(s)";

                receiptButton->Enabled =
                    table->Rows->Count > 0;
            }
            finally
            {
                delete con;
            }
        }

        void RefreshClicked(Object^ sender, EventArgs^ e)
        {
            try { LoadPayments(); }
            catch (Exception^ ex) {
                MessageBox::Show(
                    ex->Message, "Payment History Error");
            }
        }

        void ReceiptClicked(Object^ sender, EventArgs^ e)
        {
            if (grid->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Select a payment first.");
                return;
            }

            try
            {
                Object^ raw =
                    grid->SelectedRows[0]
                        ->Cells["Payment ID"]->Value;

                int id = Convert::ToInt32(raw);

                PaymentReceiptForm^ receipt =
                    gcnew PaymentReceiptForm(id);

                try {
                    receipt->ShowDialog(this);
                }
                finally {
                    delete receipt;
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message, "Receipt Error");
            }
        }

        void HistoryShown(Object^ sender, EventArgs^ e)
        {
            try {
                LoadLoans();
                LoadPayments();
            }
            catch (Exception^ ex) {
                MessageBox::Show(
                    ex->Message, "History Loading Error");
            }
        }

    public:
        PaymentHistoryForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &PaymentHistoryForm::ApplyPhase96ThemeOnShown);

            actor = user;

            this->Text = "Group R | Payment History";
            this->BackColor =
                Color::FromArgb(13, 21, 17);
            this->ForeColor = Color::White;
            this->Size =
                System::Drawing::Size(1160, 730);
            this->MinimumSize =
                System::Drawing::Size(900, 600);
            this->StartPosition =
                FormStartPosition::CenterParent;

            Label^ heading = gcnew Label();
            heading->Text = "PAYMENT HISTORY";
            heading->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 22.0f,
                    FontStyle::Bold);
            heading->ForeColor =
                Color::FromArgb(48, 204, 121);
            heading->Location = Point(25, 22);
            heading->Size =
                System::Drawing::Size(650, 50);
            this->Controls->Add(heading);

            search = gcnew TextBox();
            search->Location = Point(25, 96);
            search->Size =
                System::Drawing::Size(340, 32);
            search->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 11.0f);
            this->Controls->Add(search);

            loanFilter = gcnew ComboBox();
            loanFilter->Location = Point(380, 96);
            loanFilter->Size =
                System::Drawing::Size(240, 32);
            loanFilter->DropDownStyle =
                ComboBoxStyle::DropDownList;
            this->Controls->Add(loanFilter);

            Button^ refresh = gcnew Button();
            refresh->Text = "SEARCH / REFRESH";
            refresh->Location = Point(640, 95);
            refresh->Size =
                System::Drawing::Size(185, 35);
            refresh->BackColor =
                Color::FromArgb(27, 135, 76);
            refresh->ForeColor = Color::White;
            refresh->FlatStyle = FlatStyle::Flat;
            refresh->Click += gcnew EventHandler(
                this, &PaymentHistoryForm::RefreshClicked);
            this->Controls->Add(refresh);

            receiptButton = gcnew Button();
            receiptButton->Text = "VIEW RECEIPT";
            receiptButton->Location = Point(840, 95);
            receiptButton->Size =
                System::Drawing::Size(180, 35);
            receiptButton->BackColor =
                Color::FromArgb(35, 90, 59);
            receiptButton->ForeColor = Color::White;
            receiptButton->FlatStyle = FlatStyle::Flat;
            receiptButton->Enabled = false;
            receiptButton->Click += gcnew EventHandler(
                this, &PaymentHistoryForm::ReceiptClicked);
            this->Controls->Add(receiptButton);

            countLabel = gcnew Label();
            countLabel->Text = "0 payment record(s)";
            countLabel->ForeColor =
                Color::FromArgb(170, 193, 177);
            countLabel->Location = Point(25, 148);
            countLabel->Size =
                System::Drawing::Size(500, 25);
            this->Controls->Add(countLabel);

            grid = gcnew DataGridView();
            grid->Location = Point(25, 185);
            grid->Size =
                System::Drawing::Size(1085, 465);
            grid->Anchor =
                AnchorStyles::Top |
                AnchorStyles::Bottom |
                AnchorStyles::Left |
                AnchorStyles::Right;
            grid->ReadOnly = true;
            grid->AllowUserToAddRows = false;
            grid->AllowUserToDeleteRows = false;
            grid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            grid->MultiSelect = false;
            grid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            grid->BackgroundColor =
                Color::FromArgb(21, 34, 26);
            grid->GridColor =
                Color::FromArgb(49, 70, 54);
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(29, 65, 43);
            grid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;
            grid->DefaultCellStyle->BackColor =
                Color::FromArgb(21, 34, 26);
            grid->DefaultCellStyle->ForeColor =
                Color::White;
            grid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(29, 119, 66);
            grid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            this->Controls->Add(grid);

            this->Shown += gcnew EventHandler(
                this, &PaymentHistoryForm::HistoryShown);
        }
    };
}
