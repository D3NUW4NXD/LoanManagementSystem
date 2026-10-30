#pragma once
// GROUPR_PHASE94_V2

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

namespace GroupR_LoanManagement
{
    public ref class GroupRModernTheme abstract sealed
    {
    public:
        static void Apply(System::Windows::Forms::Control^ root)
        {
            if (root == nullptr)
                return;

            for each (
                System::Windows::Forms::Control^ item
                in root->Controls)
            {
                System::Windows::Forms::DataGridView^ grid =
                    dynamic_cast<System::Windows::Forms::DataGridView^>(item);

                if (grid != nullptr)
                {
                    grid->BackgroundColor =
                        System::Drawing::Color::FromArgb(13, 22, 18);

                    grid->BorderStyle =
                        System::Windows::Forms::BorderStyle::None;

                    grid->GridColor =
                        System::Drawing::Color::FromArgb(42, 65, 49);

                    grid->EnableHeadersVisualStyles = false;

                    grid->ColumnHeadersHeightSizeMode =
                        System::Windows::Forms::
                        DataGridViewColumnHeadersHeightSizeMode::DisableResizing;

                    grid->ColumnHeadersHeight = 46;

                    grid->ColumnHeadersDefaultCellStyle->BackColor =
                        System::Drawing::Color::FromArgb(27, 59, 40);

                    grid->ColumnHeadersDefaultCellStyle->ForeColor =
                        System::Drawing::Color::FromArgb(92, 230, 149);

                    grid->ColumnHeadersDefaultCellStyle->Font =
                        gcnew System::Drawing::Font(
                            "Segoe UI",
                            10,
                            System::Drawing::FontStyle::Bold);

                    grid->DefaultCellStyle->BackColor =
                        System::Drawing::Color::FromArgb(22, 37, 29);

                    grid->DefaultCellStyle->ForeColor =
                        System::Drawing::Color::White;

                    grid->DefaultCellStyle->SelectionBackColor =
                        System::Drawing::Color::FromArgb(37, 111, 70);

                    grid->DefaultCellStyle->SelectionForeColor =
                        System::Drawing::Color::White;

                    grid->DefaultCellStyle->Font =
                        gcnew System::Drawing::Font("Segoe UI", 10);

                    grid->DefaultCellStyle->Padding =
                        System::Windows::Forms::Padding(7, 4, 7, 4);

                    grid->AlternatingRowsDefaultCellStyle->BackColor =
                        System::Drawing::Color::FromArgb(27, 45, 34);

                    grid->RowTemplate->Height = 40;
                    grid->RowHeadersVisible = false;
                }

                System::Windows::Forms::Button^ button =
                    dynamic_cast<System::Windows::Forms::Button^>(item);

                if (button != nullptr)
                {
                    button->FlatStyle =
                        System::Windows::Forms::FlatStyle::Flat;

                    button->FlatAppearance->BorderSize = 0;

                    button->FlatAppearance->MouseOverBackColor =
                        System::Drawing::Color::FromArgb(37, 130, 80);

                    button->Cursor =
                        System::Windows::Forms::Cursors::Hand;

                    button->Font =
                        gcnew System::Drawing::Font(
                            "Segoe UI",
                            9,
                            System::Drawing::FontStyle::Bold);
                }

                System::Windows::Forms::TextBox^ input =
                    dynamic_cast<System::Windows::Forms::TextBox^>(item);

                if (input != nullptr)
                {
                    if (!input->ReadOnly)
                    {
                        input->BackColor =
                            System::Drawing::Color::FromArgb(24, 40, 31);

                        input->ForeColor =
                            System::Drawing::Color::White;
                    }

                    input->BorderStyle =
                        System::Windows::Forms::BorderStyle::FixedSingle;
                }

                System::Windows::Forms::ComboBox^ combo =
                    dynamic_cast<System::Windows::Forms::ComboBox^>(item);

                if (combo != nullptr)
                {
                    combo->BackColor =
                        System::Drawing::Color::FromArgb(24, 40, 31);

                    combo->ForeColor =
                        System::Drawing::Color::White;
                }

                                // GROUPR_PHASE98_POLISH
                System::Windows::Forms::NumericUpDown^ numeric =
                    dynamic_cast<System::Windows::Forms::NumericUpDown^>(item);

                if (numeric != nullptr)
                {
                    numeric->BackColor =
                        System::Drawing::Color::FromArgb(24, 40, 31);
                    numeric->ForeColor = System::Drawing::Color::White;
                    numeric->Font =
                        gcnew System::Drawing::Font("Segoe UI", 10);
                }

                System::Windows::Forms::DateTimePicker^ datePicker =
                    dynamic_cast<System::Windows::Forms::DateTimePicker^>(item);

                if (datePicker != nullptr)
                {
                    datePicker->CalendarMonthBackground =
                        System::Drawing::Color::FromArgb(24, 40, 31);
                    datePicker->CalendarForeColor =
                        System::Drawing::Color::FromArgb(20, 40, 28);
                    datePicker->Font =
                        gcnew System::Drawing::Font("Segoe UI", 10);
                }

                System::Windows::Forms::RichTextBox^ richText =
                    dynamic_cast<System::Windows::Forms::RichTextBox^>(item);

                if (richText != nullptr && !richText->ReadOnly)
                {
                    richText->BackColor =
                        System::Drawing::Color::FromArgb(24, 40, 31);
                    richText->ForeColor = System::Drawing::Color::White;
                }

                System::Windows::Forms::CheckBox^ check =
                    dynamic_cast<System::Windows::Forms::CheckBox^>(item);

                if (check != nullptr)
                {
                    check->ForeColor =
                        System::Drawing::Color::FromArgb(225, 237, 228);
                }

                if (item->HasChildren && grid == nullptr)
                {
                    Apply(item);
                }
            }
        }
    };
}