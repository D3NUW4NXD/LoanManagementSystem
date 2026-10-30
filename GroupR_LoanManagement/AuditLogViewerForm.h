#pragma once
#include "GroupRModernTheme.h"
#include "AuditLoggingService.h"

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
    public ref class AuditLogViewerForm : public Form
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
        bool auditViewRecorded;
        DataGridView^ grid;
        TextBox^ searchBox;
        ComboBox^ resultFilter;
        DateTimePicker^ fromDate;
        DateTimePicker^ toDate;
        Label^ countLabel;
        DataTable^ currentData;

        Color bg = Color::FromArgb(12, 20, 16);
        Color panelBg = Color::FromArgb(24, 39, 30);
        Color green = Color::FromArgb(48, 204, 121);
        Color white = Color::FromArgb(240, 247, 242);

        Button^ MakeButton(String^ caption, int x, int y, int width)
        {
            Button^ button = gcnew Button();
            button->Text = caption;
            button->Location = Point(x, y);
            button->Size = System::Drawing::Size(width, 38);
            button->BackColor = Color::FromArgb(29, 144, 81);
            button->ForeColor = white;
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            return button;
        }

        Label^ MakeLabel(String^ caption, int x, int y, int width)
        {
            Label^ label = gcnew Label();
            label->Text = caption;
            label->Location = Point(x, y);
            label->Size = System::Drawing::Size(width, 25);
            label->ForeColor = white;
            return label;
        }

        void CheckAccess(SqlConnection^ connection)
        {
            if (actor == nullptr ||
                (actor->RoleID != 1 && actor->RoleID != 2))
            {
                throw gcnew UnauthorizedAccessException(
                    "Administrator access is required.");
            }

            SqlCommand^ command = gcnew SqlCommand(
                "SELECT COUNT(*) FROM dbo.Users "
                "WHERE UserID=@id AND Username=@username "
                "AND RoleID=@role AND IsActive=1",
                connection);

            command->Parameters->Add("@id", SqlDbType::Int)->Value =
                actor->UserID;

            command->Parameters->Add(
                "@username", SqlDbType::NVarChar, 256)->Value =
                actor->Username;

            command->Parameters->Add("@role", SqlDbType::Int)->Value =
                actor->RoleID;

            if (Convert::ToInt32(command->ExecuteScalar()) != 1)
                throw gcnew UnauthorizedAccessException(
                    "Your account is not authorized to view audit logs.");
        }

        void LoadLogs()
        {
            if (fromDate->Checked && toDate->Checked &&
                fromDate->Value.Date > toDate->Value.Date)
            {
                throw gcnew ArgumentException(
                    "The start date cannot be later than the end date.");
            }

            SqlConnection^ connection =
                DatabaseConnection::CreateConnection();

            try
            {
                connection->Open();
                                CheckAccess(connection);

                if (!auditViewRecorded)
                {
                    AuditLoggingService::Record(
                        Nullable<int>(actor->UserID),
                        "AUDIT_LOG_VIEW",
                        "AuditLogs",
                        nullptr,
                        "Administrator opened the Audit Log Viewer.",
                        nullptr,
                        nullptr,
                        true,
                        nullptr);

                    auditViewRecorded = true;
                }

                String^ sql =
                    "SELECT TOP (5000) "
                    "a.AuditLogID AS [Audit ID], "
                    "a.CreatedAt AS [Timestamp], "
                    "a.UserID AS [User ID], "
                    "u.Username AS [Username], "
                    "a.Action AS [Action], "
                    "a.EntityType AS [Entity Type], "
                    "a.EntityID AS [Entity ID], "
                    "a.IsSuccessful AS [Successful], "
                    "a.Description AS [Description], "
                    "a.ErrorMessage AS [Error Message], "
                    "a.ClientMachine AS [Client Machine], "
                    "a.OldValues AS [Old Values], "
                    "a.NewValues AS [New Values] "
                    "FROM dbo.AuditLogs a "
                    "LEFT JOIN dbo.Users u ON a.UserID=u.UserID "
                    "WHERE "
                    "(@search='' OR "
                    "a.Action LIKE @pattern OR "
                    "a.EntityType LIKE @pattern OR "
                    "a.EntityID LIKE @pattern OR "
                    "a.Description LIKE @pattern OR "
                    "u.Username LIKE @pattern) "
                    "AND (@fromEnabled=0 OR a.CreatedAt>=@fromDate) "
                    "AND (@toEnabled=0 OR a.CreatedAt<DATEADD(day,1,@toDate)) "
                    "AND (@result=-1 OR a.IsSuccessful=@result) "
                    "ORDER BY a.CreatedAt DESC, a.AuditLogID DESC";

                SqlCommand^ command = gcnew SqlCommand(sql, connection);

                String^ search = searchBox->Text->Trim();

                command->Parameters->Add(
                    "@search", SqlDbType::NVarChar, 200)->Value = search;

                command->Parameters->Add(
                    "@pattern", SqlDbType::NVarChar, 450)->Value =
                    "%" + search + "%";

                command->Parameters->Add(
                    "@fromEnabled", SqlDbType::Bit)->Value =
                    fromDate->Checked;

                command->Parameters->Add(
                    "@toEnabled", SqlDbType::Bit)->Value =
                    toDate->Checked;

                command->Parameters->Add(
                    "@fromDate", SqlDbType::DateTime2)->Value =
                    fromDate->Value.Date;

                command->Parameters->Add(
                    "@toDate", SqlDbType::DateTime2)->Value =
                    toDate->Value.Date;

                int result = -1;
                if (resultFilter->SelectedIndex == 1) result = 1;
                if (resultFilter->SelectedIndex == 2) result = 0;

                command->Parameters->Add(
                    "@result", SqlDbType::Int)->Value = result;

                DataTable^ table = gcnew DataTable();
                SqlDataAdapter^ adapter = gcnew SqlDataAdapter(command);

                try
                {
                    adapter->Fill(table);
                }
                finally
                {
                    delete adapter;
                }

                currentData = table;
                grid->DataSource = currentData;

                for each (String^ name in gcnew array<String^>{
                    "Description", "Error Message", "Old Values", "New Values"
                })
                {
                    if (grid->Columns->Contains(name))
                        grid->Columns[name]->Visible = false;
                }

                countLabel->Text =
                    "Showing " + currentData->Rows->Count.ToString("N0") +
                    " records (maximum 5,000 per query)";
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
                LoadLogs();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message, "Audit Log Error",
                    MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void DetailsClicked(Object^ sender, EventArgs^ e)
        {
            if (grid->CurrentRow == nullptr ||
                grid->CurrentRow->DataBoundItem == nullptr)
            {
                MessageBox::Show("Select an audit record first.");
                return;
            }

            DataRowView^ row =
                dynamic_cast<DataRowView^>(
                    grid->CurrentRow->DataBoundItem);

            if (row == nullptr) return;

            StringBuilder^ details = gcnew StringBuilder();

            for each (DataColumn^ column in currentData->Columns)
            {
                details->AppendLine(column->ColumnName + ":");
                details->AppendLine(
                    Convert::ToString(row->Row[column]));
                details->AppendLine();
            }

            Form^ dialog = gcnew Form();
            dialog->Text = "Audit Record Details";
            dialog->ClientSize = System::Drawing::Size(750, 580);
            dialog->StartPosition = FormStartPosition::CenterParent;
            dialog->BackColor = bg;

            TextBox^ text = gcnew TextBox();
            text->Multiline = true;
            text->ReadOnly = true;
            text->ScrollBars = ScrollBars::Both;
            text->WordWrap = true;
            text->Dock = DockStyle::Fill;
            text->BackColor = panelBg;
            text->ForeColor = white;
            text->Font = gcnew System::Drawing::Font(
                "Consolas", 10.0f);
            text->Text = details->ToString();

            dialog->Controls->Add(text);
            dialog->ShowDialog(this);
        }

        String^ CsvEscape(String^ value)
        {
            if (value == nullptr) value = String::Empty;

            String^ trimmed = value->TrimStart();

            if (trimmed->Length > 0)
            {
                wchar_t first = trimmed[0];

                if (first == '=' || first == '+' ||
                    first == '-' || first == '@')
                {
                    value = "'" + value;
                }
            }

            return "\"" + value->Replace("\"", "\"\"") + "\"";
        }

        void ExportClicked(Object^ sender, EventArgs^ e)
        {
            if (currentData == nullptr)
            {
                MessageBox::Show("Refresh the audit logs first.");
                return;
            }

            SaveFileDialog^ dialog = gcnew SaveFileDialog();
            dialog->Filter = "CSV files (*.csv)|*.csv";
            dialog->FileName =
                "GroupR_AuditLogs_" +
                DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".csv";

            if (dialog->ShowDialog(this) !=
                System::Windows::Forms::DialogResult::OK)
                return;

            try
            {
                StreamWriter^ writer = gcnew StreamWriter(
                    dialog->FileName, false, gcnew UTF8Encoding(true));

                try
                {
                    StringBuilder^ header = gcnew StringBuilder();

                    for each (DataColumn^ col in currentData->Columns)
                    {
                        if (header->Length > 0) header->Append(",");
                        header->Append(CsvEscape(col->ColumnName));
                    }

                    writer->WriteLine(header->ToString());

                    for each (DataRow^ row in currentData->Rows)
                    {
                        StringBuilder^ line = gcnew StringBuilder();

                        for each (DataColumn^ col in currentData->Columns)
                        {
                            if (line->Length > 0) line->Append(",");

                            Object^ value = row[col];

                            String^ cell = value == DBNull::Value ?
                                String::Empty :
                                Convert::ToString(
                                    value, CultureInfo::InvariantCulture);

                            line->Append(CsvEscape(cell));
                        }

                        writer->WriteLine(line->ToString());
                    }
                }
                finally
                {
                    delete writer;
                }

                MessageBox::Show(
                    "Audit log CSV exported successfully.",
                    "Export Complete");
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message, "CSV Export Error");
            }
        }

        void BuildUI()
        {
            this->Text = "Group R | Audit Logs";
            this->ClientSize = System::Drawing::Size(1190, 730);
            this->MinimumSize = System::Drawing::Size(950, 580);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = bg;
            this->ForeColor = white;

            Label^ title = MakeLabel(
                "AUDIT LOGS & ACTIVITY HISTORY", 24, 20, 800);
            title->Font = gcnew System::Drawing::Font(
                "Segoe UI", 21.0f, FontStyle::Bold);
            title->ForeColor = green;
            title->Height = 48;
            this->Controls->Add(title);

            Panel^ filters = gcnew Panel();
            filters->Location = Point(24, 91);
            filters->Size = System::Drawing::Size(1140, 124);
            filters->Anchor = AnchorStyles::Top |
                AnchorStyles::Left | AnchorStyles::Right;
            filters->BackColor = panelBg;
            this->Controls->Add(filters);

            filters->Controls->Add(
                MakeLabel("SEARCH ACTIVITY", 14, 12, 220));

            searchBox = gcnew TextBox();
            searchBox->Location = Point(14, 43);
            searchBox->Size = System::Drawing::Size(275, 28);
            filters->Controls->Add(searchBox);

            filters->Controls->Add(
                MakeLabel("RESULT", 306, 12, 140));

            resultFilter = gcnew ComboBox();
            resultFilter->Location = Point(306, 43);
            resultFilter->Size = System::Drawing::Size(140, 28);
            resultFilter->DropDownStyle = ComboBoxStyle::DropDownList;
            resultFilter->Items->Add("All Results");
            resultFilter->Items->Add("Successful");
            resultFilter->Items->Add("Failed");
            resultFilter->SelectedIndex = 0;
            filters->Controls->Add(resultFilter);

            filters->Controls->Add(
                MakeLabel("FROM DATE", 464, 12, 150));

            fromDate = gcnew DateTimePicker();
            fromDate->Location = Point(464, 43);
            fromDate->Size = System::Drawing::Size(150, 28);
            fromDate->Format = DateTimePickerFormat::Short;
            fromDate->ShowCheckBox = true;
            fromDate->Checked = false;
            filters->Controls->Add(fromDate);

            filters->Controls->Add(
                MakeLabel("TO DATE", 632, 12, 150));

            toDate = gcnew DateTimePicker();
            toDate->Location = Point(632, 43);
            toDate->Size = System::Drawing::Size(150, 28);
            toDate->Format = DateTimePickerFormat::Short;
            toDate->ShowCheckBox = true;
            toDate->Checked = false;
            filters->Controls->Add(toDate);

            Button^ refresh = MakeButton("REFRESH", 798, 39, 145);
            refresh->Click += gcnew EventHandler(
                this, &AuditLogViewerForm::RefreshClicked);
            filters->Controls->Add(refresh);

            Button^ exportButton =
                MakeButton("EXPORT CSV", 953, 39, 165);
            exportButton->Click += gcnew EventHandler(
                this, &AuditLogViewerForm::ExportClicked);
            filters->Controls->Add(exportButton);

            countLabel = MakeLabel(
                "No records loaded", 26, 229, 700);
            this->Controls->Add(countLabel);

            Button^ detailsButton =
                MakeButton("VIEW SELECTED DETAILS", 908, 224, 255);
            detailsButton->Anchor =
                AnchorStyles::Top | AnchorStyles::Right;
            detailsButton->Click += gcnew EventHandler(
                this, &AuditLogViewerForm::DetailsClicked);
            this->Controls->Add(detailsButton);

            grid = gcnew DataGridView();
            grid->Location = Point(24, 277);
            grid->Size = System::Drawing::Size(1140, 423);
            grid->Anchor = AnchorStyles::Top |
                AnchorStyles::Bottom |
                AnchorStyles::Left | AnchorStyles::Right;
            grid->ReadOnly = true;
            grid->AllowUserToAddRows = false;
            grid->AllowUserToDeleteRows = false;
            grid->RowHeadersVisible = false;
            grid->MultiSelect = false;
            grid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            grid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            grid->BackgroundColor = panelBg;
            grid->BorderStyle = BorderStyle::None;
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersHeight = 43;
            grid->RowTemplate->Height = 36;
            grid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(30, 71, 46);
            grid->ColumnHeadersDefaultCellStyle->ForeColor = white;
            grid->DefaultCellStyle->BackColor = panelBg;
            grid->DefaultCellStyle->ForeColor = white;
            grid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(35, 113, 69);
            grid->DefaultCellStyle->SelectionForeColor = white;

            grid->CellDoubleClick +=
                gcnew DataGridViewCellEventHandler(
                    this, &AuditLogViewerForm::RowDoubleClicked);

            this->Controls->Add(grid);

            this->Shown += gcnew EventHandler(
                this, &AuditLogViewerForm::RefreshClicked);
        }

        void RowDoubleClicked(
            Object^ sender, DataGridViewCellEventArgs^ e)
        {
            if (e->RowIndex >= 0)
                DetailsClicked(sender, EventArgs::Empty);
        }

    public:
        AuditLogViewerForm(AuthenticatedUser^ user)
        {
        this->Shown += gcnew System::EventHandler(this, &AuditLogViewerForm::ApplyPhase97ThemeOnShown);

            actor = user;

            if (actor == nullptr ||
                (actor->RoleID != 1 && actor->RoleID != 2))
            {
                throw gcnew UnauthorizedAccessException(
                    "Administrator access required.");
            }

            currentData = nullptr;
            auditViewRecorded = false;
            BuildUI();
        }
    };
}

