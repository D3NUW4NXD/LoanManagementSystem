#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "DatabaseConnection.h"
#include "AuthenticationService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Windows::Forms;

#include "DetailedReportsForm.h"

namespace GroupR_LoanManagement
{
    public ref class ReportsDashboardForm : public Form
    {

    // GROUPR_PHASE97_PREMIUM_UI
private:
    void ApplyPhase97ThemeOnShown(
        System::Object^ sender,
        System::EventArgs^ e)
    {
        ::GroupR_LoanManagement::GroupRModernTheme::Apply(this);
    }


    private:
        AuthenticatedUser^ actor;
        Label^ totalLoans;
        Label^ approvedLoans;
        Label^ pendingLoans;
        Label^ rejectedLoans;
        Label^ collected;
        Label^ outstanding;
        Label^ approvedAmount;
        Label^ requestedAmount;
        DataGridView^ monthlyGrid;
        DataGridView^ statusGrid;
        Label^ lastUpdated;

        Color bg = Color::FromArgb(12, 21, 16);
        Color panelColor = Color::FromArgb(23, 39, 29);
        Color green = Color::FromArgb(48, 204, 121);
        Color white = Color::FromArgb(239, 247, 241);
        Color muted = Color::FromArgb(161, 186, 169);

        Label^ MakeLabel(
            String^ text,
            int x, int y, int w, int h,
            float size, bool bold, Color color)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = System::Drawing::Size(w, h);
            label->ForeColor = color;
            label->BackColor = Color::Transparent;
            label->Font = gcnew System::Drawing::Font(
                "Segoe UI", size,
                bold ? FontStyle::Bold : FontStyle::Regular);
            label->AutoEllipsis = true;
            return label;
        }

        Label^ AddMetric(
            String^ heading,
            int x, int y)
        {
            Panel^ panel = gcnew Panel();
            panel->Location = Point(x, y);
            panel->Size = System::Drawing::Size(245, 112);
            panel->BackColor = panelColor;
            this->Controls->Add(panel);

            panel->Controls->Add(
                MakeLabel(heading, 15, 12, 215, 25,
                    9.0f, true, muted));

            Label^ value = MakeLabel(
                "—", 15, 43, 215, 48,
                19.0f, true, white);

            panel->Controls->Add(value);
            return value;
        }

        DataGridView^ MakeGrid(
            int x, int y, int width, int height)
        {
            DataGridView^ grid = gcnew DataGridView();
            grid->Location = Point(x, y);
            grid->Size =
                System::Drawing::Size(width, height);
            grid->ReadOnly = true;
            grid->AllowUserToAddRows = false;
            grid->AllowUserToDeleteRows = false;
            grid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            grid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            grid->BackgroundColor = panelColor;
            grid->GridColor =
                Color::FromArgb(48, 68, 53);
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(31, 80, 47);
            grid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;
            grid->DefaultCellStyle->BackColor = panelColor;
            grid->DefaultCellStyle->ForeColor = white;
            grid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(33, 118, 66);
            grid->DefaultCellStyle->SelectionForeColor =
                Color::White;
            grid->RowHeadersVisible = false;
            return grid;
        }

        DataTable^ Query(
            SqlConnection^ con,
            String^ sql)
        {
            SqlCommand^ cmd =
                gcnew SqlCommand(sql, con);

            SqlDataAdapter^ adapter =
                gcnew SqlDataAdapter(cmd);

            DataTable^ table = gcnew DataTable();

            try {
                adapter->Fill(table);
            }
            finally {
                delete adapter;
                delete cmd;
            }

            return table;
        }

        void RefreshData()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                DataTable^ summary = Query(con,
                    "SELECT "
                    "(SELECT COUNT(*) FROM dbo.Loans) AS TotalLoans, "
                    "(SELECT COUNT(*) FROM dbo.Loans "
                    "WHERE LoanStatus='Approved') AS ApprovedLoans, "
                    "(SELECT COUNT(*) FROM dbo.Loans "
                    "WHERE LoanStatus='Pending') AS PendingLoans, "
                    "(SELECT COUNT(*) FROM dbo.Loans "
                    "WHERE LoanStatus='Rejected') AS RejectedLoans, "
                    "(SELECT COALESCE(SUM(RequestedAmount),0) "
                    "FROM dbo.Loans) AS RequestedAmount, "
                    "(SELECT COALESCE(SUM(ApprovedAmount),0) "
                    "FROM dbo.Loans) AS ApprovedAmount, "
                    "(SELECT COALESCE(SUM("
                    "PrincipalAmount+InterestAmount),0) "
                    "FROM dbo.Payments) AS Collected, "
                    "(SELECT COALESCE(SUM("
                    "(PrincipalDue-PrincipalPaid)+"
                    "(InterestDue-InterestPaid)),0) "
                    "FROM dbo.RepaymentSchedule) AS Outstanding");

                DataRow^ row = summary->Rows[0];

                totalLoans->Text =
                    Convert::ToString(row["TotalLoans"]);
                approvedLoans->Text =
                    Convert::ToString(row["ApprovedLoans"]);
                pendingLoans->Text =
                    Convert::ToString(row["PendingLoans"]);
                rejectedLoans->Text =
                    Convert::ToString(row["RejectedLoans"]);

                requestedAmount->Text =
                    "Rs. " + Convert::ToDecimal(
                        row["RequestedAmount"]).ToString("N2");

                approvedAmount->Text =
                    "Rs. " + Convert::ToDecimal(
                        row["ApprovedAmount"]).ToString("N2");

                collected->Text =
                    "Rs. " + Convert::ToDecimal(
                        row["Collected"]).ToString("N2");

                outstanding->Text =
                    "Rs. " + Convert::ToDecimal(
                        row["Outstanding"]).ToString("N2");

                monthlyGrid->DataSource = Query(con,
                    "SELECT "
                    "CONVERT(char(7),PaymentDate,120) AS [Month], "
                    "COUNT(*) AS [Payments], "
                    "SUM(PrincipalAmount) AS [Principal], "
                    "SUM(InterestAmount) AS [Interest], "
                    "SUM(PrincipalAmount+InterestAmount) "
                    "AS [Total Collected] "
                    "FROM dbo.Payments "
                    "GROUP BY CONVERT(char(7),PaymentDate,120) "
                    "ORDER BY [Month] DESC");

                statusGrid->DataSource = Query(con,
                    "SELECT LoanStatus AS [Status], "
                    "COUNT(*) AS [Loan Count], "
                    "SUM(RequestedAmount) AS [Requested Amount], "
                    "COALESCE(SUM(ApprovedAmount),0) "
                    "AS [Approved Amount] "
                    "FROM dbo.Loans "
                    "GROUP BY LoanStatus "
                    "ORDER BY [Loan Count] DESC");

                lastUpdated->Text =
                    "Last refreshed: " +
                    DateTime::Now.ToString("dd MMM yyyy HH:mm:ss");
            }
            finally
            {
                delete con;
            }
        }

        void RefreshClicked(Object^ sender, EventArgs^ e)
        {
            try {
                RefreshData();
            }
            catch (Exception^ ex) {
                MessageBox::Show(
                    ex->Message,
                    "Reports Loading Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void ReportsShown(Object^ sender, EventArgs^ e)
        {
            RefreshClicked(sender, e);
        }

    public:
                void OpenDetailedReportsClicked(Object^ sender, EventArgs^ e)
        {
            try
            {
                DetailedReportsForm^ screen =
                    gcnew DetailedReportsForm(actor);

                screen->ShowDialog(this);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Detailed Reports",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        ReportsDashboardForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &ReportsDashboardForm::ApplyPhase97ThemeOnShown);

            actor = user;

            this->Text = "Group R | Financial Reports";
            this->BackColor = bg;
            this->ForeColor = white;
            this->ClientSize =
                System::Drawing::Size(1100, 830);
            this->MinimumSize =
                System::Drawing::Size(1100, 830);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->AutoScroll = true;

            this->Controls->Add(
                MakeLabel(
                    "FINANCIAL REPORTS",
                    25, 18, 700, 48,
                    23.0f, true, green));

            this->Controls->Add(
                MakeLabel(
                    "Live loan and repayment statistics",
                    26, 64, 700, 28,
                    10.0f, false, muted));

            Button^ refresh = gcnew Button();
            refresh->Text = "REFRESH REPORTS";
            refresh->Location = Point(850, 30);
            refresh->Size =
                System::Drawing::Size(205, 40);
            refresh->BackColor =
                Color::FromArgb(30, 140, 79);
            refresh->ForeColor = white;
            refresh->FlatStyle = FlatStyle::Flat;
            refresh->FlatAppearance->BorderSize = 0;
            refresh->Click += gcnew EventHandler(
                this, &ReportsDashboardForm::RefreshClicked);
            this->Controls->Add(refresh);

            Button^ detailsButton = gcnew Button();
            detailsButton->Text = "DETAILED REPORTS / CSV";
            detailsButton->Size = System::Drawing::Size(200, 40);
            detailsButton->Location =
                Point(refresh->Left - 210, refresh->Top);
            detailsButton->Anchor =
                AnchorStyles::Top | AnchorStyles::Right;
            detailsButton->BackColor =
                Color::FromArgb(29, 144, 81);
            detailsButton->ForeColor = white;
            detailsButton->FlatStyle = FlatStyle::Flat;
            detailsButton->FlatAppearance->BorderSize = 0;
            detailsButton->Click += gcnew EventHandler(
                this,
                &ReportsDashboardForm::OpenDetailedReportsClicked);
            this->Controls->Add(detailsButton);

            totalLoans =
                AddMetric("TOTAL LOANS", 25, 115);
            approvedLoans =
                AddMetric("APPROVED LOANS", 290, 115);
            pendingLoans =
                AddMetric("PENDING LOANS", 555, 115);
            rejectedLoans =
                AddMetric("REJECTED LOANS", 820, 115);

            requestedAmount =
                AddMetric("TOTAL REQUESTED", 25, 245);
            approvedAmount =
                AddMetric("TOTAL APPROVED", 290, 245);
            collected =
                AddMetric("TOTAL COLLECTED", 555, 245);
            outstanding =
                AddMetric("SCHEDULED OUTSTANDING", 820, 245);

            this->Controls->Add(
                MakeLabel(
                    "MONTHLY PAYMENT COLLECTIONS",
                    25, 393, 650, 35,
                    15.0f, true, white));

            monthlyGrid =
                MakeGrid(25, 435, 1040, 155);
            this->Controls->Add(monthlyGrid);

            this->Controls->Add(
                MakeLabel(
                    "LOAN STATUS BREAKDOWN",
                    25, 620, 650, 35,
                    15.0f, true, white));

            statusGrid =
                MakeGrid(25, 663, 1040, 105);
            this->Controls->Add(statusGrid);

            lastUpdated = MakeLabel(
                "Not refreshed",
                25, 785, 900, 25,
                9.0f, false, muted);
            this->Controls->Add(lastUpdated);

            this->Shown += gcnew EventHandler(
                this, &ReportsDashboardForm::ReportsShown);
        }
    };
}

