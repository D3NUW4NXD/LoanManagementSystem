#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "DatabaseConnection.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Drawing;
using namespace System::Drawing::Printing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class PaymentReceiptForm : public Form
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
        int paymentID;
        String^ receiptText;
        TextBox^ preview;
        PrintDocument^ document;

        void LoadReceipt()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                SqlCommand^ cmd = gcnew SqlCommand(
                    "SELECT p.PaymentReference, "
                    "p.PaymentDate, p.PrincipalAmount, "
                    "p.InterestAmount, p.PaymentMethod, "
                    "p.TransactionReference, p.Notes, "
                    "p.RecordedByUserID, "
                    "r.InstallmentNumber, l.LoanCode "
                    "FROM dbo.Payments p "
                    "INNER JOIN dbo.RepaymentSchedule r "
                    "ON r.RepaymentScheduleID=p.RepaymentScheduleID "
                    "INNER JOIN dbo.Loans l ON l.LoanID=r.LoanID "
                    "WHERE p.PaymentID=@id",
                    con);

                cmd->Parameters->Add(
                    "@id", SqlDbType::Int
                )->Value = paymentID;

                SqlDataReader^ reader =
                    cmd->ExecuteReader();

                try
                {
                    if (!reader->Read())
                        throw gcnew InvalidOperationException(
                            "Payment receipt not found.");

                    Decimal principal = Convert::ToDecimal(
                        reader["PrincipalAmount"]);

                    Decimal interest = Convert::ToDecimal(
                        reader["InterestAmount"]);

                    String^ transaction = reader["TransactionReference"] ==
                        DBNull::Value ? "-" :
                        Convert::ToString(reader["TransactionReference"]);

                    String^ notes = reader["Notes"] ==
                        DBNull::Value ? "-" :
                        Convert::ToString(reader["Notes"]);

                    receiptText =
                        "GROUP R LOAN MANAGEMENT SYSTEM\r\n" +
                        "PAYMENT RECEIPT\r\n" +
                        "========================================\r\n\r\n" +
                        "Payment Reference : " +
                        Convert::ToString(reader["PaymentReference"]) + "\r\n" +
                        "Payment Date      : " +
                        Convert::ToDateTime(reader["PaymentDate"])
                            .ToString("yyyy-MM-dd HH:mm") + "\r\n" +
                        "Loan Code         : " +
                        Convert::ToString(reader["LoanCode"]) + "\r\n" +
                        "Installment       : " +
                        Convert::ToString(reader["InstallmentNumber"]) + "\r\n" +
                        "Payment Method    : " +
                        Convert::ToString(reader["PaymentMethod"]) + "\r\n" +
                        "Transaction Ref.  : " + transaction + "\r\n" +
                        "Recorded By ID    : " +
                        Convert::ToString(reader["RecordedByUserID"]) + "\r\n" +
                        "----------------------------------------\r\n" +
                        "Principal         : Rs. " +
                        principal.ToString("N2") + "\r\n" +
                        "Interest          : Rs. " +
                        interest.ToString("N2") + "\r\n" +
                        "TOTAL PAID        : Rs. " +
                        (principal + interest).ToString("N2") + "\r\n" +
                        "----------------------------------------\r\n" +
                        "Notes             : " + notes + "\r\n\r\n" +
                        "This receipt reflects a recorded payment.\r\n" +
                        "========================================";
                }
                finally
                {
                    reader->Close();
                }

                preview->Text = receiptText;
            }
            finally
            {
                delete con;
            }
        }

        void PrintPage(
            Object^ sender,
            PrintPageEventArgs^ e)
        {
            System::Drawing::Font^ font =
                gcnew System::Drawing::Font(
                    "Consolas", 10.0f);

            try
            {
                e->Graphics->DrawString(
                    receiptText,
                    font,
                    Brushes::Black,
                    e->MarginBounds);

                e->HasMorePages = false;
            }
            finally
            {
                delete font;
            }
        }

        void PrintClicked(
            Object^ sender, EventArgs^ e)
        {
            PrintDialog^ dialog = gcnew PrintDialog();
            dialog->Document = document;
            dialog->UseEXDialog = true;

            if (dialog->ShowDialog(this) ==
                System::Windows::Forms::DialogResult::OK)
            {
                try {
                    document->Print();
                }
                catch (Exception^ ex) {
                    MessageBox::Show(
                        ex->Message, "Printing Error");
                }
            }
        }

    public:
        PaymentReceiptForm(int id)
        {
        this->Shown += gcnew System::EventHandler(this, &PaymentReceiptForm::ApplyPhase96ThemeOnShown);

            paymentID = id;
            receiptText = "";

            this->Text = "Payment Receipt";
            this->BackColor =
                Color::FromArgb(13, 21, 17);
            this->ForeColor = Color::White;
            this->Size =
                System::Drawing::Size(710, 740);
            this->StartPosition =
                FormStartPosition::CenterParent;

            Label^ heading = gcnew Label();
            heading->Text = "PAYMENT RECEIPT";
            heading->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 19.0f,
                    FontStyle::Bold);
            heading->ForeColor =
                Color::FromArgb(48, 204, 121);
            heading->Location = Point(20, 18);
            heading->Size =
                System::Drawing::Size(600, 45);
            this->Controls->Add(heading);

            preview = gcnew TextBox();
            preview->Multiline = true;
            preview->ReadOnly = true;
            preview->ScrollBars = ScrollBars::Both;
            preview->WordWrap = false;
            preview->Font =
                gcnew System::Drawing::Font(
                    "Consolas", 10.5f);
            preview->BackColor =
                Color::FromArgb(24, 38, 30);
            preview->ForeColor = Color::White;
            preview->Location = Point(20, 80);
            preview->Size =
                System::Drawing::Size(645, 535);
            this->Controls->Add(preview);

            Button^ printButton = gcnew Button();
            printButton->Text = "PRINT RECEIPT";
            printButton->Location = Point(20, 635);
            printButton->Size =
                System::Drawing::Size(220, 43);
            printButton->BackColor =
                Color::FromArgb(29, 146, 81);
            printButton->ForeColor = Color::White;
            printButton->FlatStyle = FlatStyle::Flat;
            printButton->Click += gcnew EventHandler(
                this, &PaymentReceiptForm::PrintClicked);
            this->Controls->Add(printButton);

            document = gcnew PrintDocument();
            document->DocumentName = "Payment Receipt";
            document->PrintPage +=
                gcnew PrintPageEventHandler(
                    this, &PaymentReceiptForm::PrintPage);

            this->Shown += gcnew EventHandler(
                this, &PaymentReceiptForm::ReceiptShown);
        }

    private:
        void ReceiptShown(Object^ sender, EventArgs^ e)
        {
            try {
                LoadReceipt();
            }
            catch (Exception^ ex) {
                MessageBox::Show(
                    ex->Message, "Receipt Error");
            }
        }
    };
}
