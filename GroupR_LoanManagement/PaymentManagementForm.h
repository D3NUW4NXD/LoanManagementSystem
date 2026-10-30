#include "PaymentHistoryForm.h"
#include "PaymentEntryForm.h"
#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class PaymentManagementForm : public Form
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

        ComboBox^ loanSelector;
        DataGridView^ ledger;
        Label^ statusText;
        Label^ totalDueValue;
        Label^ totalPaidValue;
        Label^ outstandingValue;
        Label^ overdueValue;
        Button^ refreshButton;

        Color bg = Color::FromArgb(13, 20, 17);
        Color panelBg = Color::FromArgb(26, 42, 33);
        Color green = Color::FromArgb(48, 204, 121);
        Color muted = Color::FromArgb(165, 181, 170);
        Color white = Color::FromArgb(242, 246, 243);

        Label^ MakeLabel(
            String^ text, int x, int y,
            int width, int height,
            float size, Color foreground, bool bold)
        {
            Label^ result = gcnew Label();

            result->Text = text;
            result->Location = Point(x, y);
            result->Size = System::Drawing::Size(
                width, height);
            result->ForeColor = foreground;
            result->BackColor = Color::Transparent;
            result->Font = gcnew System::Drawing::Font(
                "Segoe UI", size,
                bold ? FontStyle::Bold : FontStyle::Regular);
            result->AutoEllipsis = true;

            return result;
        }

        Panel^ MakeStat(
            String^ title,
            Label^% value,
            int left)
        {
            Panel^ card = gcnew Panel();
            card->Location = Point(left, 10);
            card->Size = System::Drawing::Size(244, 103);
            card->BackColor = panelBg;

            card->Controls->Add(
                MakeLabel(
                    title, 16, 14, 218, 24,
                    9, muted, false));

            value = MakeLabel(
                "Rs. 0.00",
                16, 44, 220, 44,
                18, green, true);

            card->Controls->Add(value);
            return card;
        }

        int SelectedLoan()
        {
            if (loanSelector->SelectedValue == nullptr ||
                loanSelector->SelectedValue == DBNull::Value)
                return 0;

            // During ComboBox data binding the selected
            // value may temporarily be a DataRowView.
            if (dynamic_cast<DataRowView^>(
                    loanSelector->SelectedValue) != nullptr)
                return 0;

            return Convert::ToInt32(
                loanSelector->SelectedValue);
        }

        void ResetSummary()
        {
            totalDueValue->Text = "Rs. 0.00";
            totalPaidValue->Text = "Rs. 0.00";
            outstandingValue->Text = "Rs. 0.00";
            overdueValue->Text = "0";
        }

        void LoadLoans()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT LoanID, "
                    "LoanCode + N'  |  ' + "
                    "LoanStatus AS LoanDisplay "
                    "FROM dbo.Loans "
                    "ORDER BY LoanID DESC",
                    con);

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                DataTable^ result = gcnew DataTable();
                adapter->Fill(result);

                loanSelector->DataSource = nullptr;
                loanSelector->DisplayMember = "LoanDisplay";
                loanSelector->ValueMember = "LoanID";
                loanSelector->DataSource = result;
            }
            finally
            {
                delete con;
            }
        }

        void LoadLedger()
        {
            int loanID = SelectedLoan();

            if (loanID <= 0)
            {
                ledger->DataSource = nullptr;
                ResetSummary();
                statusText->Text =
                    "No loan selected. Register a loan "
                    "to view its payment schedule.";
                return;
            }

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT "
                    "RepaymentScheduleID AS [Schedule ID], " 
                    "InstallmentNumber AS [Installment], "
                    "DueDate AS [Due Date], "
                    "PrincipalDue AS [Principal Due], "
                    "InterestDue AS [Interest Due], "
                    "CAST(PrincipalDue+InterestDue "
                    "AS decimal(19,2)) AS [Total Due], "
                    "CAST(PrincipalPaid+InterestPaid "
                    "AS decimal(19,2)) AS [Total Paid], "
                    "CAST(PrincipalDue+InterestDue "
                    "-PrincipalPaid-InterestPaid "
                    "AS decimal(19,2)) AS [Outstanding], "
                    "PaymentStatus AS [Status], "
                    "LastPaymentDate AS [Last Payment] "
                    "FROM dbo.RepaymentSchedule "
                    "WHERE LoanID=@loan "
                    "ORDER BY InstallmentNumber",
                    con);

                cmd->Parameters->Add(
                    "@loan", SqlDbType::Int
                )->Value = loanID;

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                DataTable^ result = gcnew DataTable();
                adapter->Fill(result);

                ledger->DataSource = result;

                if (ledger->Columns->Contains("Schedule ID"))
                    ledger->Columns["Schedule ID"]->Visible = false;

                Decimal due = Decimal::Zero;
                Decimal paid = Decimal::Zero;
                Decimal outstanding = Decimal::Zero;
                int overdue = 0;

                for each (DataRow^ row in result->Rows)
                {
                    Decimal rowDue = Convert::ToDecimal(
                        row["Total Due"]);

                    Decimal rowPaid = Convert::ToDecimal(
                        row["Total Paid"]);

                    Decimal rowOutstanding =
                        Convert::ToDecimal(
                            row["Outstanding"]);

                    due += rowDue;
                    paid += rowPaid;
                    outstanding += rowOutstanding;

                    DateTime dueDate =
                        Convert::ToDateTime(
                            row["Due Date"]);

                    if (dueDate.Date < DateTime::Today &&
                        rowOutstanding > Decimal::Zero)
                        overdue++;
                }

                totalDueValue->Text =
                    "Rs. " + due.ToString("N2");

                totalPaidValue->Text =
                    "Rs. " + paid.ToString("N2");

                outstandingValue->Text =
                    "Rs. " + outstanding.ToString("N2");

                overdueValue->Text =
                    overdue.ToString();

                for each (DataGridViewColumn^ column
                    in ledger->Columns)
                {
                    String^ name = column->Name;

                    if (name == "Principal Due" ||
                        name == "Interest Due" ||
                        name == "Total Due" ||
                        name == "Total Paid" ||
                        name == "Outstanding")
                    {
                        column->DefaultCellStyle->Format =
                            "N2";
                    }

                    if (name == "Due Date" ||
                        name == "Last Payment")
                    {
                        column->DefaultCellStyle->Format =
                            "yyyy-MM-dd";
                    }
                }

                statusText->Text =
                    "Loan #" + loanID.ToString() +
                    "  |  Installments: " +
                    result->Rows->Count.ToString() +
                    "  |  Read-only payment overview";

                if (result->Rows->Count == 0)
                {
                    statusText->Text =
                        "This loan has no generated "
                        "repayment schedule yet.";
                }
            }
            finally
            {
                delete con;
            }
        }

        void SelectedLoanChanged(
            Object^ sender,
            EventArgs^ e)
        {
            // Event may fire while the selector
            // is still being populated.
            if (ledger == nullptr ||
                totalDueValue == nullptr)
                return;

            try
            {
                LoadLedger();
            }
            catch (Exception^ ex)
            {
                statusText->Text =
                    "Unable to load payment data: " +
                    ex->Message;
            }
        }

        void RefreshClicked(
            Object^ sender,
            EventArgs^ e)
        {
            try
            {
                LoadLoans();
                LoadLedger();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Payment Management",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void RecordPaymentClicked(
            Object^ sender, EventArgs^ e)
        {
            if (ledger->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Select an installment from the table first.",
                    "Record Payment");
                return;
            }

            try
            {
                DataGridViewRow^ selected =
                    ledger->SelectedRows[0];

                Object^ rawID =
                    selected->Cells["Schedule ID"]->Value;

                if (rawID == nullptr ||
                    rawID == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "The installment ID is unavailable.");

                int id = Convert::ToInt32(rawID);

                PaymentEntryForm^ form =
                    gcnew PaymentEntryForm(actor, id);

                try
                {
                    if (form->ShowDialog(this) ==
                        System::Windows::Forms::DialogResult::OK)
                    {
                        LoadLedger();
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
                    ex->Message,
                    "Record Payment",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void HistoryClicked(Object^ sender, EventArgs^ e)
        {
            try
            {
                PaymentHistoryForm^ form =
                    gcnew PaymentHistoryForm(actor);

                try {
                    form->ShowDialog(this);
                }
                finally {
                    delete form;
                }

                LoadLedger();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Payment History Error");
            }
        }



        void ScreenShown(
            Object^ sender,
            EventArgs^ e)
        {
            RefreshClicked(nullptr, EventArgs::Empty);
        }

        void BuildUI()
        {
            this->Text =
                "Group R | Payment Management";
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->ClientSize =
                System::Drawing::Size(1170, 710);
            this->MinimumSize =
                System::Drawing::Size(1060, 630);
            this->BackColor = bg;
            this->ForeColor = white;

            Panel^ header = gcnew Panel();
            header->Dock = DockStyle::Top;
            header->Height = 155;
            header->BackColor =
                Color::FromArgb(19, 33, 25);

            header->Controls->Add(
                MakeLabel(
                    "PAYMENT MANAGEMENT",
                    28, 17, 680, 42,
                    21, green, true));

            header->Controls->Add(
                MakeLabel(
                    "Repayment tracking and installment overview",
                    29, 57, 800, 25,
                    10, muted, false));

            header->Controls->Add(
                MakeLabel(
                    "SELECT LOAN",
                    29, 98, 170, 24,
                    9, white, true));

            loanSelector = gcnew ComboBox();
            loanSelector->Location = Point(166, 97);
            loanSelector->Size =
                System::Drawing::Size(430, 31);
            loanSelector->DropDownStyle =
                ComboBoxStyle::DropDownList;
            loanSelector->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10.0f);
            loanSelector->SelectedIndexChanged +=
                gcnew EventHandler(
                    this,
                    &PaymentManagementForm::
                    SelectedLoanChanged);

            header->Controls->Add(loanSelector);

            refreshButton = gcnew Button();
            refreshButton->Text = "REFRESH";
            refreshButton->Location = Point(615, 95);
            refreshButton->Size =
                System::Drawing::Size(145, 36);
            refreshButton->BackColor =
                Color::FromArgb(40, 130, 76);
            refreshButton->ForeColor = white;
            refreshButton->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10.0f,
                    FontStyle::Bold);
            refreshButton->FlatStyle =
                FlatStyle::Flat;
            refreshButton->FlatAppearance->BorderSize = 0;
            refreshButton->Click +=
                gcnew EventHandler(
                    this,
                    &PaymentManagementForm::RefreshClicked);

            header->Controls->Add(refreshButton);

            Button^ historyButton = gcnew Button();
            historyButton->Text = "PAYMENT HISTORY";
            historyButton->Location = Point(580, 95);
            historyButton->Size =
                System::Drawing::Size(190, 36);
            historyButton->BackColor =
                Color::FromArgb(31, 109, 66);
            historyButton->ForeColor = white;
            historyButton->FlatStyle = FlatStyle::Flat;
            historyButton->FlatAppearance->BorderSize = 0;
            historyButton->Click += gcnew EventHandler(
                this, &PaymentManagementForm::HistoryClicked);
            header->Controls->Add(historyButton);

            Button^ recordButton = gcnew Button();
            recordButton->Text = "RECORD PAYMENT";
            recordButton->Location = Point(785, 95);
            recordButton->Size =
                System::Drawing::Size(195, 36);
            recordButton->BackColor =
                Color::FromArgb(28, 138, 78);
            recordButton->ForeColor = white;
            recordButton->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10.0f,
                    FontStyle::Bold);
            recordButton->FlatStyle = FlatStyle::Flat;
            recordButton->FlatAppearance->BorderSize = 0;

            recordButton->Click +=
                gcnew EventHandler(
                    this,
                    &PaymentManagementForm::RecordPaymentClicked);

            header->Controls->Add(recordButton);

            Panel^ statistics = gcnew Panel();
            statistics->Dock = DockStyle::Top;
            statistics->Height = 130;
            statistics->BackColor = bg;
            statistics->AutoScroll = true;

            statistics->Controls->Add(
                MakeStat(
                    "TOTAL SCHEDULED",
                    totalDueValue, 20));

            statistics->Controls->Add(
                MakeStat(
                    "TOTAL RECEIVED",
                    totalPaidValue, 276));

            statistics->Controls->Add(
                MakeStat(
                    "OUTSTANDING AMOUNT",
                    outstandingValue, 532));

            statistics->Controls->Add(
                MakeStat(
                    "OVERDUE INSTALLMENTS",
                    overdueValue, 788));

            ledger = gcnew DataGridView();
            ledger->Dock = DockStyle::Fill;
            ledger->ReadOnly = true;
            ledger->MultiSelect = false;
            ledger->AllowUserToAddRows = false;
            ledger->AllowUserToDeleteRows = false;
            ledger->AllowUserToResizeRows = false;
            ledger->RowHeadersVisible = false;
            ledger->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            ledger->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            ledger->BackgroundColor = bg;
            ledger->BorderStyle = BorderStyle::None;
            ledger->GridColor =
                Color::FromArgb(52, 73, 59);
            ledger->EnableHeadersVisualStyles = false;

            ledger->ColumnHeadersHeight = 42;
            ledger->RowTemplate->Height = 36;

            ledger->ColumnHeadersDefaultCellStyle->
                BackColor = Color::FromArgb(36, 80, 53);
            ledger->ColumnHeadersDefaultCellStyle->
                ForeColor = white;
            ledger->ColumnHeadersDefaultCellStyle->
                Font = gcnew System::Drawing::Font(
                    "Segoe UI", 9.0f,
                    FontStyle::Bold);

            ledger->DefaultCellStyle->BackColor =
                panelBg;
            ledger->DefaultCellStyle->ForeColor =
                white;
            ledger->DefaultCellStyle->
                SelectionBackColor =
                Color::FromArgb(36, 121, 73);
            ledger->DefaultCellStyle->
                SelectionForeColor = white;

            Panel^ footer = gcnew Panel();
            footer->Dock = DockStyle::Bottom;
            footer->Height = 52;
            footer->BackColor =
                Color::FromArgb(19, 33, 25);

            statusText = MakeLabel(
                "Loading payment data...",
                22, 14, 1050, 28,
                10, muted, false);

            footer->Controls->Add(statusText);

            // Establish all UI controls before
            // loading data from SQL Server.
            this->Controls->Add(ledger);
            this->Controls->Add(footer);
            this->Controls->Add(statistics);
            this->Controls->Add(header);

            this->Shown +=
                gcnew EventHandler(
                    this,
                    &PaymentManagementForm::ScreenShown);
        }

    public:
        PaymentManagementForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &PaymentManagementForm::ApplyPhase96ThemeOnShown);

            if (user == nullptr)
                throw gcnew ArgumentNullException("user");

            actor = user;
            BuildUI();
        }
    };
}


