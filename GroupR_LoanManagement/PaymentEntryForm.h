#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "PaymentRecordingService.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class PaymentEntryForm : public Form
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
        int scheduleID;

        Label^ installmentInfo;
        Label^ outstandingInfo;
        NumericUpDown^ amountInput;
        ComboBox^ methodInput;
        TextBox^ transactionInput;
        TextBox^ notesInput;
        Button^ saveButton;

        Color bg = Color::FromArgb(13, 21, 17);
        Color panelColor = Color::FromArgb(26, 43, 33);
        Color green = Color::FromArgb(48, 204, 121);
        Color white = Color::FromArgb(241, 247, 242);
        Color muted = Color::FromArgb(169, 187, 174);

        Label^ MakeLabel(
            String^ caption,
            int x, int y, int width,
            int height, float size,
            Color color, bool bold)
        {
            Label^ label = gcnew Label();
            label->Text = caption;
            label->Location = Point(x, y);
            label->Size =
                System::Drawing::Size(width, height);
            label->Font = gcnew System::Drawing::Font(
                "Segoe UI", size,
                bold ? FontStyle::Bold :
                       FontStyle::Regular);
            label->ForeColor = color;
            label->BackColor = Color::Transparent;
            label->AutoEllipsis = true;
            return label;
        }

        void LoadInstallment()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT l.LoanCode, "
                    "r.InstallmentNumber, r.DueDate, "
                    "r.PrincipalDue, r.InterestDue, "
                    "r.PrincipalPaid, r.InterestPaid "
                    "FROM dbo.RepaymentSchedule r "
                    "INNER JOIN dbo.Loans l "
                    "ON l.LoanID=r.LoanID "
                    "WHERE r.RepaymentScheduleID=@id",
                    con);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = scheduleID;

                SqlDataReader^ reader =
                    cmd->ExecuteReader();

                try
                {
                    if (!reader->Read())
                        throw gcnew InvalidOperationException(
                            "Selected installment was not found.");

                    String^ code = Convert::ToString(
                        reader["LoanCode"]);

                    int number = Convert::ToInt32(
                        reader["InstallmentNumber"]);

                    DateTime dueDate = Convert::ToDateTime(
                        reader["DueDate"]);

                    Decimal principalRemaining =
                        Convert::ToDecimal(reader["PrincipalDue"]) -
                        Convert::ToDecimal(reader["PrincipalPaid"]);

                    Decimal interestRemaining =
                        Convert::ToDecimal(reader["InterestDue"]) -
                        Convert::ToDecimal(reader["InterestPaid"]);

                    Decimal outstanding =
                        principalRemaining + interestRemaining;

                    if (outstanding < Decimal::Zero)
                        throw gcnew InvalidOperationException(
                            "Stored installment balances are invalid.");

                    installmentInfo->Text =
                        code + "  |  Installment #" +
                        number.ToString() +
                        "  |  Due: " +
                        dueDate.ToString("dd MMM yyyy");

                    outstandingInfo->Text =
                        "Rs. " + outstanding.ToString("N2");

                    if (outstanding <= Decimal::Zero)
                    {
                        amountInput->Enabled = false;
                        saveButton->Enabled = false;
                        outstandingInfo->Text +=
                            "  (Already paid)";
                    }
                    else
                    {
                        amountInput->Maximum = outstanding;
                        amountInput->Value = outstanding;
                        amountInput->Enabled = true;
                        saveButton->Enabled = true;
                    }
                }
                finally
                {
                    reader->Close();
                }
            }
            finally
            {
                delete con;
            }
        }

        void SavePayment(
            Object^ sender, EventArgs^ e)
        {
            Decimal amount = amountInput->Value;
            String^ method =
                Convert::ToString(methodInput->SelectedItem);

            if (amount <= Decimal::Zero)
            {
                MessageBox::Show(
                    "Enter a payment amount greater than zero.");
                return;
            }

            if (String::IsNullOrWhiteSpace(method))
            {
                MessageBox::Show(
                    "Choose a payment method.");
                return;
            }

            if (method != "Cash" &&
                String::IsNullOrWhiteSpace(
                    transactionInput->Text))
            {
                MessageBox::Show(
                    "Enter the bank/card transaction reference.");
                return;
            }

            String^ confirmation =
                "Installment: #" + scheduleID.ToString() +
                "\nPayment amount: Rs. " +
                amount.ToString("N2") +
                "\nMethod: " + method +
                "\n\nConfirm this payment?";

            System::Windows::Forms::DialogResult choice =
                MessageBox::Show(
                    confirmation,
                    "Confirm Payment",
                    MessageBoxButtons::YesNo,
                    MessageBoxIcon::Question);

            if (choice !=
                System::Windows::Forms::DialogResult::Yes)
                return;

            saveButton->Enabled = false;

            try
            {
                String^ paymentReference =
                    PaymentRecordingService::RecordPayment(
                        actor,
                        scheduleID,
                        amount,
                        method,
                        transactionInput->Text,
                        notesInput->Text);

                MessageBox::Show(
                    "Payment recorded successfully.\n\n"
                    "Reference: " + paymentReference,
                    "Payment Saved",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;

                this->Close();
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Payment Not Saved",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);

                saveButton->Enabled = true;
            }
        }

        void OnFormShown(
            Object^ sender, EventArgs^ e)
        {
            try
            {
                LoadInstallment();
            }
            catch (Exception^ ex)
            {
                saveButton->Enabled = false;

                MessageBox::Show(
                    ex->Message,
                    "Installment Error",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void BuildUI()
        {
            this->Text =
                "Group R | Record Payment";
            this->BackColor = bg;
            this->ForeColor = white;
            this->ClientSize =
                System::Drawing::Size(570, 655);
            this->MinimumSize =
                System::Drawing::Size(570, 655);
            this->MaximumSize =
                System::Drawing::Size(570, 655);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;

            this->Controls->Add(
                MakeLabel("RECORD PAYMENT",
                    26, 19, 500, 42,
                    21, green, true));

            this->Controls->Add(
                MakeLabel(
                    "Apply a payment to the selected installment",
                    27, 65, 510, 27,
                    10, muted, false));

            Panel^ infoPanel = gcnew Panel();
            infoPanel->Location = Point(25, 107);
            infoPanel->Size =
                System::Drawing::Size(518, 117);
            infoPanel->BackColor = panelColor;

            installmentInfo = MakeLabel(
                "Loading installment...",
                18, 13, 481, 28,
                10, white, true);

            outstandingInfo = MakeLabel(
                "Rs. 0.00",
                17, 60, 480, 37,
                20, green, true);

            infoPanel->Controls->Add(
                MakeLabel(
                    "OUTSTANDING",
                    18, 43, 250, 22,
                    8, muted, false));
            infoPanel->Controls->Add(installmentInfo);
            infoPanel->Controls->Add(outstandingInfo);
            this->Controls->Add(infoPanel);

            this->Controls->Add(
                MakeLabel(
                    "PAYMENT AMOUNT (LKR)",
                    27, 245, 300, 24,
                    10, white, true));

            amountInput = gcnew NumericUpDown();
            amountInput->Location = Point(27, 275);
            amountInput->Size =
                System::Drawing::Size(515, 32);
            amountInput->DecimalPlaces = 2;
            amountInput->Minimum = Decimal::Zero;
            amountInput->Maximum = Decimal(1000000000);
            amountInput->ThousandsSeparator = true;
            amountInput->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 12.0f);
            this->Controls->Add(amountInput);

            this->Controls->Add(
                MakeLabel(
                    "PAYMENT METHOD",
                    27, 322, 300, 24,
                    10, white, true));

            methodInput = gcnew ComboBox();
            methodInput->Location = Point(27, 351);
            methodInput->Size =
                System::Drawing::Size(515, 31);
            methodInput->DropDownStyle =
                ComboBoxStyle::DropDownList;
            methodInput->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 11.0f);
            methodInput->Items->Add("Cash");
            methodInput->Items->Add("Bank Transfer");
            methodInput->Items->Add("Card");
            methodInput->SelectedIndex = 0;

            this->Controls->Add(methodInput);

            this->Controls->Add(
                MakeLabel(
                    "TRANSACTION REFERENCE (BANK / CARD)",
                    27, 396, 490, 24,
                    9, white, true));

            transactionInput = gcnew TextBox();
            transactionInput->Location =
                Point(27, 424);
            transactionInput->Size =
                System::Drawing::Size(515, 31);
            transactionInput->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 11.0f);
            this->Controls->Add(transactionInput);

            this->Controls->Add(
                MakeLabel(
                    "NOTES (OPTIONAL)",
                    27, 465, 350, 24,
                    9, white, true));

            notesInput = gcnew TextBox();
            notesInput->Location = Point(27, 491);
            notesInput->Size =
                System::Drawing::Size(515, 63);
            notesInput->Multiline = true;
            notesInput->ScrollBars =
                ScrollBars::Vertical;
            notesInput->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10.0f);

            this->Controls->Add(notesInput);

            saveButton = gcnew Button();
            saveButton->Text = "CONFIRM PAYMENT";
            saveButton->Location = Point(302, 579);
            saveButton->Size =
                System::Drawing::Size(240, 45);
            saveButton->BackColor =
                Color::FromArgb(32, 143, 81);
            saveButton->ForeColor = white;
            saveButton->FlatStyle =
                FlatStyle::Flat;
            saveButton->FlatAppearance->BorderSize = 0;
            saveButton->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10.0f,
                    FontStyle::Bold);
            saveButton->Enabled = false;

            saveButton->Click += gcnew EventHandler(
                this, &PaymentEntryForm::SavePayment);

            this->Controls->Add(saveButton);

            Button^ cancel = gcnew Button();
            cancel->Text = "CANCEL";
            cancel->Location = Point(27, 579);
            cancel->Size =
                System::Drawing::Size(145, 45);
            cancel->BackColor =
                Color::FromArgb(66, 73, 69);
            cancel->ForeColor = white;
            cancel->FlatStyle = FlatStyle::Flat;
            cancel->FlatAppearance->BorderSize = 0;
            cancel->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Controls->Add(cancel);

            this->CancelButton = cancel;

            this->Shown += gcnew EventHandler(
                this, &PaymentEntryForm::OnFormShown);
        }

    public:
        PaymentEntryForm(
            AuthenticatedUser^ user,
            int repaymentScheduleID)
        {
        this->Shown += gcnew System::EventHandler(this, &PaymentEntryForm::ApplyPhase96ThemeOnShown);

            if (user == nullptr)
                throw gcnew ArgumentNullException("user");

            if (repaymentScheduleID <= 0)
                throw gcnew ArgumentOutOfRangeException(
                    "repaymentScheduleID");

            actor = user;
            scheduleID = repaymentScheduleID;
            BuildUI();
        }
    };
}
