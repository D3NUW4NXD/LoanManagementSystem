#pragma once

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "RegistrationService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class RegisterForm : public Form
    {
    private:
        TextBox^ nameBox;
        TextBox^ usernameBox;
        TextBox^ emailBox;
        TextBox^ passwordBox;
        TextBox^ confirmBox;
        Button^ submitButton;
        Label^ statusLabel;

        Color bg = Color::FromArgb(13, 22, 18);
        Color panel = Color::FromArgb(24, 37, 30);
        Color green = Color::FromArgb(56, 211, 126);
        Color muted = Color::FromArgb(157, 177, 165);

        void AddLabel(String^ text, int y)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(46, y);
            label->Size = Drawing::Size(510, 24);
            label->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            label->ForeColor = muted;
            Controls->Add(label);
        }

        TextBox^ AddField(int y, bool password)
        {
            TextBox^ box = gcnew TextBox();
            box->Location = Point(46, y);
            box->Size = Drawing::Size(508, 38);
            box->AutoSize = false;
            box->Font = gcnew Drawing::Font("Segoe UI", 12);
            box->BackColor = panel;
            box->ForeColor = Color::White;
            box->BorderStyle = BorderStyle::FixedSingle;
            box->UseSystemPasswordChar = password;
            Controls->Add(box);
            return box;
        }

        void TogglePasswords(Object^ sender, EventArgs^ e)
        {
            CheckBox^ toggle = safe_cast<CheckBox^>(sender);
            passwordBox->UseSystemPasswordChar = !toggle->Checked;
            confirmBox->UseSystemPasswordChar = !toggle->Checked;
        }

        void Submit(Object^ sender, EventArgs^ e)
        {
            statusLabel->Text = "";

            if (!String::Equals(
                passwordBox->Text, confirmBox->Text,
                StringComparison::Ordinal))
            {
                statusLabel->Text = "Passwords do not match.";
                return;
            }

            submitButton->Enabled = false;

            try
            {
                RegistrationService::Register(
                    usernameBox->Text,
                    nameBox->Text,
                    emailBox->Text,
                    passwordBox->Text);

                passwordBox->Clear();
                confirmBox->Clear();

                MessageBox::Show(
                    "Your registration has been submitted.\n\n"
                    "A Super Administrator must activate your Staff account "
                    "before you can sign in.",
                    "Registration Submitted",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);

                DialogResult = System::Windows::Forms::DialogResult::OK;
                Close();
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
                statusLabel->Text = "Unable to register. Check the database connection.";
            }
            finally
            {
                submitButton->Enabled = true;
            }
        }

    public:
        RegisterForm()
        {
            Text = "Group R | Create Account";
            ClientSize = Drawing::Size(600, 700);
            StartPosition = FormStartPosition::CenterParent;
            BackColor = bg;
            ForeColor = Color::White;
            FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            MaximizeBox = false;
            MinimizeBox = false;
            AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;

            Label^ heading = gcnew Label();
            heading->Text = "Create your account";
            heading->Location = Point(46, 30);
            heading->Size = Drawing::Size(510, 46);
            heading->Font = gcnew Drawing::Font("Segoe UI", 23, FontStyle::Bold);
            heading->ForeColor = Color::White;
            Controls->Add(heading);

            Label^ subtitle = gcnew Label();
            subtitle->Text = "Join the Group R staff workspace";
            subtitle->Location = Point(48, 84);
            subtitle->Size = Drawing::Size(510, 24);
            subtitle->Font = gcnew Drawing::Font("Segoe UI", 10);
            subtitle->ForeColor = muted;
            Controls->Add(subtitle);

            AddLabel("FULL NAME", 135);
            nameBox = AddField(161, false);
            nameBox->MaxLength = 150;

            AddLabel("USERNAME", 217);
            usernameBox = AddField(243, false);
            usernameBox->MaxLength = 40;

            AddLabel("EMAIL ADDRESS", 299);
            emailBox = AddField(325, false);
            emailBox->MaxLength = 254;

            AddLabel("PASSWORD (16+ CHARACTERS)", 381);
            passwordBox = AddField(407, true);
            passwordBox->MaxLength = 128;

            AddLabel("CONFIRM PASSWORD", 463);
            confirmBox = AddField(489, true);
            confirmBox->MaxLength = 128;

            CheckBox^ showPassword = gcnew CheckBox();
            showPassword->Text = "Show passwords";
            showPassword->Location = Point(46, 540);
            showPassword->Size = Drawing::Size(200, 26);
            showPassword->ForeColor = muted;
            showPassword->CheckedChanged +=
                gcnew EventHandler(this, &RegisterForm::TogglePasswords);
            Controls->Add(showPassword);

            submitButton = gcnew Button();
            submitButton->Text = "CREATE ACCOUNT";
            submitButton->Location = Point(46, 575);
            submitButton->Size = Drawing::Size(508, 48);
            submitButton->BackColor = Color::FromArgb(28, 176, 91);
            submitButton->ForeColor = Color::White;
            submitButton->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
            submitButton->FlatStyle = FlatStyle::Flat;
            submitButton->FlatAppearance->BorderSize = 0;
            submitButton->Cursor = Cursors::Hand;
            submitButton->Click += gcnew EventHandler(this, &RegisterForm::Submit);
            Controls->Add(submitButton);

            statusLabel = gcnew Label();
            statusLabel->Location = Point(46, 631);
            statusLabel->Size = Drawing::Size(510, 36);
            statusLabel->ForeColor = Color::FromArgb(248, 113, 113);
            statusLabel->Font = gcnew Drawing::Font("Segoe UI", 9);
            Controls->Add(statusLabel);

            AcceptButton = submitButton;
        }
    };
}
