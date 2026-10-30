#pragma once

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "PasswordResetService.h"
#include "UserAdminEditService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class ResetPasswordForm : public Form
    {
    private:
        AuthenticatedUser^ currentUser;
        int targetUserID;

        TextBox^ passwordBox;
        TextBox^ confirmBox;
        Label^ statusLabel;
        Button^ resetButton;

        Label^ MakeLabel(String^ text, int x, int y)
        {
            Label^ result = gcnew Label();
            result->Text = text;
            result->Location = Point(x, y);
            result->Size = Drawing::Size(460, 28);
            result->ForeColor = Color::White;
            result->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);
            this->Controls->Add(result);
            return result;
        }

        TextBox^ MakePasswordBox(int x, int y)
        {
            TextBox^ result = gcnew TextBox();
            result->Location = Point(x, y);
            result->Size = Drawing::Size(460, 35);
            result->BackColor = Color::FromArgb(31, 44, 39);
            result->ForeColor = Color::White;
            result->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            result->BorderStyle = BorderStyle::FixedSingle;
            result->UseSystemPasswordChar = true;
            result->MaxLength = 128;
            this->Controls->Add(result);
            return result;
        }

        void OnReset(Object^ sender, EventArgs^ e)
        {
            String^ password = passwordBox->Text;
            String^ confirmation = confirmBox->Text;

            statusLabel->Text = String::Empty;

            if (password->Length < 16 || password->Length > 128)
            {
                statusLabel->Text =
                    "Use a password of 16 to 128 characters.";
                return;
            }

            if (!String::Equals(
                password,
                confirmation,
                StringComparison::Ordinal))
            {
                statusLabel->Text = "Passwords do not match.";
                return;
            }

            if (MessageBox::Show(
                "Reset the selected account's password?",
                "Confirm Password Reset",
                MessageBoxButtons::YesNo,
                MessageBoxIcon::Warning
            ) != System::Windows::Forms::DialogResult::Yes)
            {
                return;
            }

            resetButton->Enabled = false;

            try
            {
                PasswordResetService::ResetPassword(
                    currentUser,
                    targetUserID,
                    password);

                passwordBox->Clear();
                confirmBox->Clear();

                MessageBox::Show(
                    "Password updated successfully.",
                    "Group R",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;
                this->Close();
            }
            catch (UnauthorizedAccessException^)
            {
                statusLabel->Text = "Access denied.";
            }
            catch (ArgumentException^ ex)
            {
                statusLabel->Text = ex->Message;
            }
            catch (InvalidOperationException^ ex)
            {
                statusLabel->Text = ex->Message;
            }
            catch (Exception^)
            {
                statusLabel->Text =
                    "Unable to reset password. Check database.";
            }
            finally
            {
                resetButton->Enabled = true;
            }
        }

        void OnCancel(Object^ sender, EventArgs^ e)
        {
            passwordBox->Clear();
            confirmBox->Clear();

            this->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;
            this->Close();
        }

    public:
        ResetPasswordForm(
            AuthenticatedUser^ actor,
            int userID)
        {
            currentUser = actor;
            targetUserID = userID;

            // Reuse the existing protected user lookup.
            UserEditRecord^ account =
                UserAdminEditService::GetUser(actor, userID);

            this->Text = "Group R - Reset Password";
            this->ClientSize = Drawing::Size(540, 460);
            this->StartPosition =
                FormStartPosition::CenterParent;
            this->BackColor = Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->MinimizeBox = false;

            Label^ heading = MakeLabel(
                "RESET PASSWORD", 35, 25);
            heading->Size = Drawing::Size(460, 45);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 20.0f, FontStyle::Bold);
            heading->ForeColor = Color::FromArgb(34, 197, 94);

            MakeLabel(
                "Username: " + account->Username,
                35, 92);

            MakeLabel(
                "Role: " + account->RoleName,
                35, 125);

            MakeLabel(
                "New Password (16-128 characters)",
                35, 175);
            passwordBox = MakePasswordBox(35, 205);

            MakeLabel(
                "Confirm New Password",
                35, 260);
            confirmBox = MakePasswordBox(35, 290);

            resetButton = gcnew Button();
            resetButton->Text = "RESET PASSWORD";
            resetButton->Location = Point(35, 355);
            resetButton->Size = Drawing::Size(285, 43);
            resetButton->BackColor =
                Color::FromArgb(22, 163, 74);
            resetButton->ForeColor = Color::White;
            resetButton->FlatStyle = FlatStyle::Flat;
            resetButton->FlatAppearance->BorderSize = 0;
            resetButton->Click += gcnew EventHandler(
                this, &ResetPasswordForm::OnReset);
            this->Controls->Add(resetButton);

            Button^ cancelButton = gcnew Button();
            cancelButton->Text = "CANCEL";
            cancelButton->Location = Point(335, 355);
            cancelButton->Size = Drawing::Size(170, 43);
            cancelButton->BackColor =
                Color::FromArgb(31, 44, 39);
            cancelButton->ForeColor = Color::White;
            cancelButton->FlatStyle = FlatStyle::Flat;
            cancelButton->Click += gcnew EventHandler(
                this, &ResetPasswordForm::OnCancel);
            this->Controls->Add(cancelButton);

            statusLabel = MakeLabel("", 35, 415);
            statusLabel->ForeColor = Color::OrangeRed;

            this->AcceptButton = resetButton;
            this->CancelButton = cancelButton;
        }
    };
}
