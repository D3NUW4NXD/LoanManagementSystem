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
using namespace System::IO;
using namespace System::Text;
using namespace System::Globalization;

namespace GroupR_LoanManagement
{
    public ref class DetailedReportsForm : public Form
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
        ComboBox^ reportType;
        TextBox^ searchBox;
        DateTimePicker^ startDate;
        DateTimePicker^ endDate;
        DataGridView^ reportGrid;
        Label^ resultLabel;
        DataTable^ currentTable;

        Color background = Color::FromArgb(12, 20, 16);
        Color panelColor = Color::FromArgb(24, 39, 30);
        Color green = Color::FromArgb(48, 204, 121);
        Color white = Color::FromArgb(240, 247, 242);
        Color muted = Color::FromArgb(164, 184, 170);

        Label^ MakeLabel(String^ text, int x, int y, int w)
        {
            Label^ l = gcnew Label();
            l->Text = text;
            l->Location = Point(x, y);
            l->Size = System::Drawing::Size(w, 24);
            l->ForeColor = muted;
            l->BackColor = Color::Transparent;
            l->Font = gcnew System::Drawing::Font("Segoe UI", 9.0f);
            return l;
        }

        Button^ MakeButton(String^ text, int x, int y, int w)
        {
            Button^ b = gcnew Button();
            b->Text = text;
            b->Location = Point(x, y);
            b->Size = System::Drawing::Size(w, 39);
            b->BackColor = Color::FromArgb(29, 144, 81);
            b->ForeColor = white;
            b->FlatStyle = FlatStyle::Flat;
            b->FlatAppearance->BorderSize = 0;
            b->Font = gcnew System::Drawing::Font(
                "Segoe UI", 9.0f, FontStyle::Bold);
            return b;
        }

        String^ GetQuery()
        {
            if (reportType->SelectedIndex == 0)
            {
                return
                    "SELECT l.LoanCode AS [Loan Code], "
                    "l.ApplicantID AS [Applicant ID], "
                    "l.LoanType AS [Loan Type], "
                    "l.LoanStatus AS [Status], "
                    "l.RequestedAmount AS [Requested (LKR)], "
                    "l.ApprovedAmount AS [Approved (LKR)], "
                    "l.ApplicationDate AS [Application Date] "
                    "FROM dbo.Loans l "
                    "WHERE (@pattern='%%' "
                    "OR l.LoanCode LIKE @pattern "
                    "OR l.LoanType LIKE @pattern "
                    "OR l.LoanStatus LIKE @pattern "
                    "OR CONVERT(nvarchar(20),l.ApplicantID) LIKE @pattern) "
                    "AND (@useStart=0 OR l.ApplicationDate>=@startDate) "
                    "AND (@useEnd=0 OR l.ApplicationDate<DATEADD(day,1,@endDate)) "
                    "ORDER BY l.ApplicationDate DESC, l.LoanID DESC";
            }

            if (reportType->SelectedIndex == 1)
            {
                return
                    "SELECT p.PaymentReference AS [Payment Reference], "
                    "l.LoanCode AS [Loan Code], "
                    "r.InstallmentNumber AS [Installment], "
                    "p.PaymentDate AS [Payment Date], "
                    "p.PrincipalAmount AS [Principal (LKR)], "
                    "p.InterestAmount AS [Interest (LKR)], "
                    "(p.PrincipalAmount+p.InterestAmount) AS [Collected (LKR)], "
                    "p.PaymentMethod AS [Payment Method], "
                    "p.TransactionReference AS [Transaction Reference], "
                    "p.RecordedByUserID AS [Recorded By User ID] "
                    "FROM dbo.Payments p "
                    "JOIN dbo.RepaymentSchedule r "
                    "ON p.RepaymentScheduleID=r.RepaymentScheduleID "
                    "JOIN dbo.Loans l ON r.LoanID=l.LoanID "
                    "WHERE (@pattern='%%' "
                    "OR p.PaymentReference LIKE @pattern "
                    "OR l.LoanCode LIKE @pattern "
                    "OR p.PaymentMethod LIKE @pattern "
                    "OR COALESCE(p.TransactionReference,'') LIKE @pattern) "
                    "AND (@useStart=0 OR p.PaymentDate>=@startDate) "
                    "AND (@useEnd=0 OR p.PaymentDate<DATEADD(day,1,@endDate)) "
                    "ORDER BY p.PaymentDate DESC, p.PaymentID DESC";
            }

            return
                "SELECT l.LoanCode AS [Loan Code], "
                "r.InstallmentNumber AS [Installment], "
                "r.DueDate AS [Due Date], "
                "r.PrincipalDue AS [Principal Due (LKR)], "
                "r.InterestDue AS [Interest Due (LKR)], "
                "(r.PrincipalDue+r.InterestDue) AS [Total Due (LKR)], "
                "r.PrincipalPaid AS [Principal Paid (LKR)], "
                "r.InterestPaid AS [Interest Paid (LKR)], "
                "(r.PrincipalDue+r.InterestDue-"
                "r.PrincipalPaid-r.InterestPaid) AS [Outstanding (LKR)], "
                "r.PaymentStatus AS [Payment Status] "
                "FROM dbo.RepaymentSchedule r "
                "JOIN dbo.Loans l ON r.LoanID=l.LoanID "
                "WHERE (@pattern='%%' "
                "OR l.LoanCode LIKE @pattern "
                "OR r.PaymentStatus LIKE @pattern "
                "OR CONVERT(nvarchar(20),r.InstallmentNumber) LIKE @pattern) "
                "AND (@useStart=0 OR r.DueDate>=@startDate) "
                "AND (@useEnd=0 OR r.DueDate<DATEADD(day,1,@endDate)) "
                "ORDER BY r.DueDate DESC, l.LoanCode, r.InstallmentNumber";
        }

        void CheckAuthorization(SqlConnection^ connection)
        {
            if (actor == nullptr ||
                (actor->RoleID != 1 && actor->RoleID != 2))
            {
                throw gcnew UnauthorizedAccessException(
                    "Administrator permission required.");
            }

            SqlCommand^ check = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND Username=@username "
                "AND RoleID=@role AND IsActive=1",
                connection);

            check->Parameters->Add("@id", SqlDbType::Int)->Value =
                actor->UserID;

            check->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256)->Value =
                actor->Username;

            check->Parameters->Add("@role", SqlDbType::Int)->Value =
                actor->RoleID;

            if (Convert::ToInt32(check->ExecuteScalar()) != 1)
            {
                throw gcnew UnauthorizedAccessException(
                    "Your account does not have permission to view reports.");
            }
        }

        void LoadReport()
        {
            if (reportType->SelectedIndex < 0)
                return;

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();
                CheckAuthorization(connection);

                SqlCommand^ command =
                    gcnew SqlCommand(GetQuery(), connection);

                command->Parameters->Add(
                    "@pattern", SqlDbType::NVarChar, 400)->Value =
                    "%" + searchBox->Text->Trim() + "%";

                command->Parameters->Add(
                    "@useStart", SqlDbType::Bit)->Value =
                    startDate->Checked;

                command->Parameters->Add(
                    "@useEnd", SqlDbType::Bit)->Value =
                    endDate->Checked;

                command->Parameters->Add(
                    "@startDate", SqlDbType::DateTime2)->Value =
                    startDate->Value.Date;

                command->Parameters->Add(
                    "@endDate", SqlDbType::DateTime2)->Value =
                    endDate->Value.Date;

                DataTable^ loaded = gcnew DataTable();

                SqlDataAdapter^ adapter =
                    gcnew SqlDataAdapter(command);

                try
                {
                    adapter->Fill(loaded);
                }
                finally
                {
                    delete adapter;
                }

                currentTable = loaded;
                reportGrid->DataSource = currentTable;

                for each (DataGridViewColumn^ column
                    in reportGrid->Columns)
                {
                    if (column->HeaderText->Contains("(LKR)"))
                    {
                        column->DefaultCellStyle->Format = "N2";
                    }
                }

                resultLabel->Text =
                    "Records: " + currentTable->Rows->Count.ToString("N0") +
                    "  |  Updated: " +
                    DateTime::Now.ToString("dd MMM yyyy HH:mm");
            }
            finally
            {
                delete connection;
            }
        }

        void RefreshClicked(Object^ sender, EventArgs^ e)
        {
            try
            {
                LoadReport();
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

        String^ EscapeCsv(String^ value)
        {
            if (value == nullptr)
                value = String::Empty;

            String^ trimmed = value->TrimStart();

            if (trimmed->Length > 0)
            {
                wchar_t first = trimmed[0];

                if (first == '=' || first == '+' ||
                    first == '-' || first == '@' ||
                    first == '\t' || first == '\r')
                {
                    value = "'" + value;
                }
            }

            return "\"" + value->Replace("\"", "\"\"") + "\"";
        }

        String^ CsvValue(Object^ value)
        {
            if (value == nullptr || value == DBNull::Value)
                return String::Empty;

            if (dynamic_cast<DateTime^>(value) != nullptr)
            {
                DateTime date = safe_cast<DateTime>(value);
                return date.ToString("yyyy-MM-dd HH:mm:ss",
                    CultureInfo::InvariantCulture);
            }

            return Convert::ToString(
                value, CultureInfo::InvariantCulture);
        }

        void ExportClicked(Object^ sender, EventArgs^ e)
        {
            if (currentTable == nullptr)
            {
                MessageBox::Show("Load a report before exporting.");
                return;
            }

            SaveFileDialog^ dialog = gcnew SaveFileDialog();
            dialog->Filter = "CSV files (*.csv)|*.csv";
            dialog->DefaultExt = "csv";
            dialog->AddExtension = true;
            dialog->FileName =
                "GroupR_" +
                (reportType->SelectedIndex == 0 ? "Loans" :
                 reportType->SelectedIndex == 1 ? "Payments" :
                 "Repayment") +
                "_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") +
                ".csv";

            if (dialog->ShowDialog(this) !=
                System::Windows::Forms::DialogResult::OK)
                return;

            try
            {
                StreamWriter^ writer = gcnew StreamWriter(
                    dialog->FileName,
                    false,
                    gcnew UTF8Encoding(true));

                try
                {
                    StringBuilder^ heading = gcnew StringBuilder();

                    for each (DataColumn^ column
                        in currentTable->Columns)
                    {
                        if (heading->Length > 0)
                            heading->Append(",");

                        heading->Append(
                            EscapeCsv(column->ColumnName));
                    }

                    writer->WriteLine(heading->ToString());

                    for each (DataRow^ row in currentTable->Rows)
                    {
                        StringBuilder^ line =
                            gcnew StringBuilder();

                        for each (DataColumn^ column
                            in currentTable->Columns)
                        {
                            if (line->Length > 0)
                                line->Append(",");

                            line->Append(
                                EscapeCsv(CsvValue(row[column])));
                        }

                        writer->WriteLine(line->ToString());
                    }
                }
                finally
                {
                    delete writer;
                }

                MessageBox::Show(
                    "CSV exported successfully.",
                    "Export Complete",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "CSV Export Failed",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void ReportChanged(Object^ sender, EventArgs^ e)
        {
            RefreshClicked(sender, e);
        }

        void BuildInterface()
        {
            this->Text = "Group R | Detailed Financial Reports";
            this->ClientSize = System::Drawing::Size(1180, 710);
            this->MinimumSize = System::Drawing::Size(950, 560);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = background;
            this->ForeColor = white;

            Label^ heading = gcnew Label();
            heading->Text = "DETAILED FINANCIAL REPORTS";
            heading->Location = Point(25, 20);
            heading->Size = System::Drawing::Size(700, 46);
            heading->Font = gcnew System::Drawing::Font(
                "Segoe UI", 21.0f, FontStyle::Bold);
            heading->ForeColor = green;
            this->Controls->Add(heading);

            Label^ subtitle = MakeLabel(
                "Search and export loan, payment and repayment records",
                28, 70, 680);
            this->Controls->Add(subtitle);

            Panel^ filterPanel = gcnew Panel();
            filterPanel->Location = Point(24, 113);
            filterPanel->Size = System::Drawing::Size(1130, 120);
            filterPanel->Anchor =
                AnchorStyles::Top | AnchorStyles::Left |
                AnchorStyles::Right;
            filterPanel->BackColor = panelColor;
            this->Controls->Add(filterPanel);

            filterPanel->Controls->Add(
                MakeLabel("REPORT TYPE", 16, 11, 165));

            reportType = gcnew ComboBox();
            reportType->Location = Point(16, 39);
            reportType->Size = System::Drawing::Size(194, 30);
            reportType->DropDownStyle = ComboBoxStyle::DropDownList;
            reportType->Items->Add("Loan Report");
            reportType->Items->Add("Payment Report");
            reportType->Items->Add("Repayment Report");
            filterPanel->Controls->Add(reportType);

            filterPanel->Controls->Add(
                MakeLabel("SEARCH", 229, 11, 240));

            searchBox = gcnew TextBox();
            searchBox->Location = Point(229, 39);
            searchBox->Size = System::Drawing::Size(241, 30);
            filterPanel->Controls->Add(searchBox);

            filterPanel->Controls->Add(
                MakeLabel("FROM DATE (OPTIONAL)", 492, 11, 220));

            startDate = gcnew DateTimePicker();
            startDate->Location = Point(492, 39);
            startDate->Size = System::Drawing::Size(160, 30);
            startDate->Format = DateTimePickerFormat::Short;
            startDate->ShowCheckBox = true;
            startDate->Checked = false;
            filterPanel->Controls->Add(startDate);

            filterPanel->Controls->Add(
                MakeLabel("TO DATE (OPTIONAL)", 670, 11, 220));

            endDate = gcnew DateTimePicker();
            endDate->Location = Point(670, 39);
            endDate->Size = System::Drawing::Size(160, 30);
            endDate->Format = DateTimePickerFormat::Short;
            endDate->ShowCheckBox = true;
            endDate->Checked = false;
            filterPanel->Controls->Add(endDate);

            Button^ refresh = MakeButton(
                "REFRESH", 850, 37, 125);
            refresh->Click += gcnew EventHandler(
                this, &DetailedReportsForm::RefreshClicked);
            filterPanel->Controls->Add(refresh);

            Button^ exportBtn = MakeButton(
                "EXPORT CSV", 981, 37, 132);
            exportBtn->Click += gcnew EventHandler(
                this, &DetailedReportsForm::ExportClicked);
            filterPanel->Controls->Add(exportBtn);

            resultLabel = MakeLabel(
                "No report loaded", 27, 246, 800);
            this->Controls->Add(resultLabel);

            reportGrid = gcnew DataGridView();
            reportGrid->Location = Point(24, 281);
            reportGrid->Size = System::Drawing::Size(1130, 397);
            reportGrid->Anchor =
                AnchorStyles::Top | AnchorStyles::Bottom |
                AnchorStyles::Left | AnchorStyles::Right;
            reportGrid->BackgroundColor = panelColor;
            reportGrid->BorderStyle = BorderStyle::None;
            reportGrid->ReadOnly = true;
            reportGrid->AllowUserToAddRows = false;
            reportGrid->AllowUserToDeleteRows = false;
            reportGrid->RowHeadersVisible = false;
            reportGrid->MultiSelect = false;
            reportGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            reportGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            reportGrid->EnableHeadersVisualStyles = false;
            reportGrid->ColumnHeadersHeight = 42;
            reportGrid->RowTemplate->Height = 35;
            reportGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(30, 71, 46);
            reportGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                white;
            reportGrid->DefaultCellStyle->BackColor = panelColor;
            reportGrid->DefaultCellStyle->ForeColor = white;
            reportGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(35, 113, 69);
            reportGrid->DefaultCellStyle->SelectionForeColor =
                white;
            this->Controls->Add(reportGrid);

            reportType->SelectedIndex = 0;
            reportType->SelectedIndexChanged += gcnew EventHandler(
                this, &DetailedReportsForm::ReportChanged);
        }

    public:
        DetailedReportsForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &DetailedReportsForm::ApplyPhase97ThemeOnShown);

            actor = user;

            if (actor == nullptr ||
                (actor->RoleID != 1 && actor->RoleID != 2))
            {
                throw gcnew UnauthorizedAccessException(
                    "Only Administrators may view detailed reports.");
            }

            currentTable = nullptr;
            BuildInterface();
            this->Shown += gcnew EventHandler(
                this, &DetailedReportsForm::RefreshClicked);
        }
    };
}
