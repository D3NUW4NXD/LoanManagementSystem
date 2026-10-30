#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "RepaymentScheduleSaveService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class RepaymentScheduleForm : public Form
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
        AuthenticatedUser^ currentUser;
        ComboBox^ loanSelector;
        DateTimePicker^ firstDuePicker;
        DataGridView^ scheduleGrid;
        Label^ statusLabel;
        Button^ generateButton;
        Button^ refreshButton;

        Color backgroundColor = Color::FromArgb(12, 17, 15);
        Color surfaceColor = Color::FromArgb(24, 32, 28);
        Color accentColor = Color::FromArgb(65, 205, 125);

        Label^ MakeLabel(
            String^ caption,
            int x, int y,
            int width, int height,
            int fontSize)
        {
            Label^ label = gcnew Label();
            label->Text = caption;
            label->Location = Point(x, y);
            label->Size = System::Drawing::Size(
                width, height);
            label->ForeColor = Color::White;
            label->BackColor = Color::Transparent;
            label->Font = gcnew System::Drawing::Font(
                "Segoe UI", (float)fontSize,
                FontStyle::Regular);
            return label;
        }

        Button^ MakeButton(
            String^ caption,
            int x, int y,
            int width,
            Color color)
        {
            Button^ button = gcnew Button();
            button->Text = caption;
            button->Location = Point(x, y);
            button->Size = System::Drawing::Size(
                width, 38);
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->BackColor = color;
            button->ForeColor = Color::White;
            button->Font = gcnew System::Drawing::Font(
                "Segoe UI", 10.0f,
                FontStyle::Bold);
            button->Cursor = Cursors::Hand;
            return button;
        }

        int SelectedLoanID()
        {
            if (loanSelector->SelectedValue == nullptr ||
                loanSelector->SelectedValue == DBNull::Value)
                return 0;

            return Convert::ToInt32(
                loanSelector->SelectedValue);
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
                    "LoanCode + N' (' + LoanStatus + N')' "
                    "AS LoanLabel "
                    "FROM dbo.Loans "
                    "ORDER BY LoanID DESC",
                    con);

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(cmd);

                DataTable^ table = gcnew DataTable();
                adapter->Fill(table);

                loanSelector->DataSource = nullptr;
                loanSelector->DisplayMember = "LoanLabel";
                loanSelector->ValueMember = "LoanID";
                loanSelector->DataSource = table;

                bool hasLoans = table->Rows->Count > 0;

                generateButton->Enabled =
                    hasLoans &&
                    (currentUser->RoleID == 1 ||
                     currentUser->RoleID == 2);

                if (!hasLoans)
                {
                    scheduleGrid->DataSource = nullptr;
                    statusLabel->Text =
                        "No loan applications registered yet.";
                }
            }
            finally
            {
                delete con;
            }
        }

        void LoadSchedule()
        {
            int loanID = SelectedLoanID();

            if (loanID <= 0)
            {
                scheduleGrid->DataSource = nullptr;
                return;
            }

            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT "
                    "InstallmentNumber AS [No], "
                    "DueDate AS [Due Date], "
                    "PrincipalDue AS [Principal], "
                    "InterestDue AS [Interest], "
                    "TotalDue AS [Total Due], "
                    "PrincipalPaid AS [Principal Paid], "
                    "InterestPaid AS [Interest Paid], "
                    "TotalPaid AS [Total Paid], "
                    "BalanceRemaining AS [Balance], "
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

                DataTable^ table = gcnew DataTable();
                adapter->Fill(table);

                scheduleGrid->DataSource = table;

                statusLabel->Text =
                    "Loan #" + loanID.ToString() +
                    " | Installments: " +
                    table->Rows->Count.ToString();

                if (table->Rows->Count == 0)
                    statusLabel->Text +=
                        " | No schedule generated";
            }
            finally
            {
                delete con;
            }
        }

        void LoanChanged(
            Object^ sender,
            EventArgs^ e)
        {
            try
            {
                LoadSchedule();
            }
            catch (Exception^ ex)
            {
                statusLabel->Text =
                    "Unable to load schedule: " +
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
                LoadSchedule();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Refresh Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void GenerateClicked(
            Object^ sender,
            EventArgs^ e)
        {
            int loanID;

            try
            {
                loanID = SelectedLoanID();
            }
            catch (Exception^)
            {
                MessageBox::Show(
                    "Select a valid loan first.");
                return;
            }

            if (loanID <= 0)
            {
                MessageBox::Show(
                    "Select a loan first.");
                return;
            }

            System::Windows::Forms::DialogResult answer = MessageBox::Show(
                "Generate and save the repayment "
                "schedule for Loan #" +
                loanID.ToString() + "?\n\n"
                "First due date: " +
                firstDuePicker->Value.ToShortDateString() +
                "\n\nThis action creates database "
                "records and cannot be repeated "
                "for an existing schedule.",
                "Confirm Schedule Generation",
                MessageBoxButtons::YesNo,
                MessageBoxIcon::Question);

            if (answer != System::Windows::Forms::DialogResult::Yes)
                return;

            try
            {
                int count =
                    RepaymentScheduleSaveService::
                    GenerateAndSave(
                        currentUser,
                        loanID,
                        firstDuePicker->Value.Date);

                LoadSchedule();

                MessageBox::Show(
                    count.ToString() +
                    " installments saved successfully.",
                    "Schedule Generated",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Schedule Not Generated",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
            }
        }

        void FormShown(
            Object^ sender,
            EventArgs^ e)
        {
            try
            {
                LoadLoans();
                LoadSchedule();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Database Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void BuildInterface()
        {
            this->Text =
                "Group R | Repayment Schedule Management";
            this->BackColor = backgroundColor;
            this->ForeColor = Color::White;
            this->ClientSize =
                System::Drawing::Size(1150, 700);
            this->MinimumSize =
                System::Drawing::Size(900, 550);
            this->StartPosition =
                FormStartPosition::CenterParent;

            Panel^ top = gcnew Panel();
            top->Dock = DockStyle::Top;
            top->Height = 160;
            top->BackColor = surfaceColor;

            Label^ heading = MakeLabel(
                "REPAYMENT SCHEDULE MANAGEMENT",
                24, 14, 800, 38, 18);
            heading->ForeColor = accentColor;
            top->Controls->Add(heading);

            top->Controls->Add(
                MakeLabel("Select Loan", 24, 59,
                          200, 24, 10));

            loanSelector = gcnew ComboBox();
            loanSelector->Location = Point(24, 88);
            loanSelector->Size =
                System::Drawing::Size(330, 32);
            loanSelector->DropDownStyle =
                ComboBoxStyle::DropDownList;
            loanSelector->BackColor = Color::White;
            loanSelector->ForeColor = Color::Black;
            loanSelector->SelectedIndexChanged +=
                gcnew EventHandler(
                    this,
                    &RepaymentScheduleForm::LoanChanged);

            top->Controls->Add(loanSelector);

            top->Controls->Add(
                MakeLabel("First Due Date", 375, 59,
                          200, 24, 10));

            firstDuePicker = gcnew DateTimePicker();
            firstDuePicker->Location = Point(375, 88);
            firstDuePicker->Size =
                System::Drawing::Size(165, 32);
            firstDuePicker->Format =
                DateTimePickerFormat::Short;
            firstDuePicker->Value =
                DateTime::Today.AddMonths(1);

            top->Controls->Add(firstDuePicker);

            generateButton = MakeButton(
                "GENERATE SCHEDULE",
                560, 85, 210,
                Color::FromArgb(33, 133, 82));

            generateButton->Click +=
                gcnew EventHandler(
                    this,
                    &RepaymentScheduleForm::GenerateClicked);

            top->Controls->Add(generateButton);

            refreshButton = MakeButton(
                "REFRESH",
                790, 85, 125,
                Color::FromArgb(61, 72, 67));

            refreshButton->Click +=
                gcnew EventHandler(
                    this,
                    &RepaymentScheduleForm::RefreshClicked);

            top->Controls->Add(refreshButton);

            scheduleGrid = gcnew DataGridView();
            scheduleGrid->Dock = DockStyle::Fill;
            scheduleGrid->ReadOnly = true;
            scheduleGrid->AllowUserToAddRows = false;
            scheduleGrid->AllowUserToDeleteRows = false;
            scheduleGrid->AllowUserToResizeRows = false;
            scheduleGrid->MultiSelect = false;
            scheduleGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            scheduleGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            scheduleGrid->BackgroundColor =
                backgroundColor;
            scheduleGrid->GridColor =
                Color::FromArgb(52, 62, 57);
            scheduleGrid->BorderStyle =
                BorderStyle::None;
            scheduleGrid->RowHeadersVisible = false;
            scheduleGrid->EnableHeadersVisualStyles = false;
            scheduleGrid->ColumnHeadersDefaultCellStyle->
                BackColor = Color::FromArgb(32, 53, 41);
            scheduleGrid->ColumnHeadersDefaultCellStyle->
                ForeColor = Color::White;
            scheduleGrid->DefaultCellStyle->BackColor =
                surfaceColor;
            scheduleGrid->DefaultCellStyle->ForeColor =
                Color::White;
            scheduleGrid->DefaultCellStyle->
                SelectionBackColor =
                Color::FromArgb(28, 121, 74);
            scheduleGrid->DefaultCellStyle->
                SelectionForeColor = Color::White;

            Panel^ footer = gcnew Panel();
            footer->Dock = DockStyle::Bottom;
            footer->Height = 52;
            footer->BackColor = surfaceColor;

            statusLabel = MakeLabel(
                "Loading repayment information...",
                24, 13, 1000, 26, 10);
            statusLabel->ForeColor = accentColor;
            footer->Controls->Add(statusLabel);

            this->Controls->Add(scheduleGrid);
            this->Controls->Add(footer);
            this->Controls->Add(top);

            this->Shown +=
                gcnew EventHandler(
                    this,
                    &RepaymentScheduleForm::FormShown);
        }

    public:
        RepaymentScheduleForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &RepaymentScheduleForm::ApplyPhase96ThemeOnShown);

            if (user == nullptr)
                throw gcnew ArgumentNullException("user");

            currentUser = user;
            BuildInterface();
        }
    };
}

