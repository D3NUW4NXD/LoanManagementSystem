#pragma once

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>

#include "UserCreationService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class CreateUserForm : public Form
    {
    private:
        AuthenticatedUser^ currentUser;

        TextBox^ usernameBox;
        TextBox^ fullNameBox;
        TextBox^ emailBox;
        TextBox^ passwordBox;
        TextBox^ confirmBox;

        ComboBox^ roleBox;
        Label^ statusLabel;
        Button^ saveButton;

        Color bgColor = Color::FromArgb(15, 24, 20);
        Color fieldColor = Color::FromArgb(31, 44, 39);
        Color accent = Color::FromArgb(34, 197, 94);

        Label^ AddLabel(String^ text, int x, int y)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(480, 25);
            label->ForeColor = Color::White;
            label->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f
            );

            this->Controls->Add(label);
            return label;
        }

        TextBox^ AddTextBox(int x, int y, int width)
        {
            TextBox^ field = gcnew TextBox();

            field->Location = Point(x, y);
            field->Size = Drawing::Size(width, 32);
            field->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f
            );

            field->BackColor = fieldColor;
            field->ForeColor = Color::White;
            field->BorderStyle = BorderStyle::FixedSingle;

            this->Controls->Add(field);
            return field;
        }

        void OnCancel(Object^ sender, EventArgs^ e)
        {
            this->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;

            this->Close();
        }

        void OnSave(Object^ sender, EventArgs^ e)
        {
            String^ username = usernameBox->Text->Trim();
            String^ fullName = fullNameBox->Text->Trim();
            String^ email = emailBox->Text->Trim();
            String^ password = passwordBox->Text;
            String^ confirmation = confirmBox->Text;

            statusLabel->ForeColor = Color::OrangeRed;

            if (String::IsNullOrWhiteSpace(username) ||
                String::IsNullOrWhiteSpace(fullName))
            {
                statusLabel->Text =
                    "Username and full name are required.";
                return;
            }

            if (username->Length > 100 ||
                fullName->Length > 150)
            {
                statusLabel->Text =
                    "Username or full name is too long.";
                return;
            }

            if (email->Length > 254)
            {
                statusLabel->Text = "Email address is too long.";
                return;
            }

            if (roleBox->SelectedIndex < 0)
            {
                statusLabel->Text = "Select an account role.";
                return;
            }

            if (password->Length < 16)
            {
                statusLabel->Text =
                    "Password must have at least 16 characters.";
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

            int roleID =
                (roleBox->SelectedIndex == 0) ? 2 : 3;

            saveButton->Enabled = false;

            try
            {
                int newUserID =
                    UserCreationService::CreateUser(
                        currentUser,
                        username,
                        fullName,
                        email,
                        password,
                        roleID
                    );

                passwordBox->Clear();
                confirmBox->Clear();
                password = nullptr;
                confirmation = nullptr;

                MessageBox::Show(
                    "User account created successfully.\n" +
                    "User ID: " + newUserID.ToString() +
                    "\nUsername: " + username,
                    "Account Created",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information
                );

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;

                this->Close();
            }
            catch (UnauthorizedAccessException^)
            {
                statusLabel->Text =
                    "Access denied. Super Administrator required.";
            }
            catch (InvalidOperationException^ ex)
            {
                statusLabel->Text = ex->Message;
            }
            catch (ArgumentException^ ex)
            {
                statusLabel->Text = ex->Message;
            }
            catch (Exception^)
            {
                statusLabel->Text =
                    "Account creation failed. Check database connection.";
            }
            finally
            {
                saveButton->Enabled = true;
            }
        }

    public:
        CreateUserForm(AuthenticatedUser^ user)
        {
            if (user == nullptr || user->RoleID != 1)
            {
                throw gcnew UnauthorizedAccessException(
                    "Super Administrator required."
                );
            }

            currentUser = user;

            this->Text = "Group R - Create User";
            this->ClientSize = Drawing::Size(610, 665);
            this->StartPosition =
                FormStartPosition::CenterParent;

            this->BackColor = bgColor;
            this->ForeColor = Color::White;

            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;

            this->MaximizeBox = false;
            this->MinimizeBox = false;

            Label^ heading = gcnew Label();
            heading->Text = "CREATE NEW USER";
            heading->Location = Point(35, 22);
            heading->Size = Drawing::Size(540, 45);
            heading->Font = gcnew Drawing::Font(
                "Segoe UI", 20.0f, FontStyle::Bold
            );
            heading->ForeColor = accent;
            this->Controls->Add(heading);

            AddLabel("Username", 35, 90);
            usernameBox = AddTextBox(35, 119, 535);
            usernameBox->MaxLength = 100;

            AddLabel("Full Name", 35, 170);
            fullNameBox = AddTextBox(35, 199, 535);
            fullNameBox->MaxLength = 150;

            AddLabel("Email Address (optional)", 35, 250);
            emailBox = AddTextBox(35, 279, 535);
            emailBox->MaxLength = 254;

            AddLabel("Account Role", 35, 330);

            roleBox = gcnew ComboBox();
            roleBox->Location = Point(35, 359);
            roleBox->Size = Drawing::Size(535, 32);
            roleBox->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f
            );
            roleBox->BackColor = fieldColor;
            roleBox->ForeColor = Color::White;
            roleBox->DropDownStyle =
                ComboBoxStyle::DropDownList;

            roleBox->Items->Add("Administrator");
            roleBox->Items->Add("Staff");
            roleBox->SelectedIndex = 1;

            this->Controls->Add(roleBox);

            AddLabel("Password (at least 16 characters)", 35, 410);
            passwordBox = AddTextBox(35, 439, 535);
            passwordBox->UseSystemPasswordChar = true;
            passwordBox->MaxLength = 128;

            AddLabel("Confirm Password", 35, 490);
            confirmBox = AddTextBox(35, 519, 535);
            confirmBox->UseSystemPasswordChar = true;
            confirmBox->MaxLength = 128;

            saveButton = gcnew Button();
            saveButton->Text = "CREATE ACCOUNT";
            saveButton->Location = Point(35, 572);
            saveButton->Size = Drawing::Size(340, 43);
            saveButton->BackColor = accent;
            saveButton->ForeColor = Color::White;
            saveButton->FlatStyle = FlatStyle::Flat;
            saveButton->FlatAppearance->BorderSize = 0;
            saveButton->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f, FontStyle::Bold
            );

            saveButton->Click += gcnew EventHandler(
                this, &CreateUserForm::OnSave
            );

            this->Controls->Add(saveButton);

            Button^ cancelButton = gcnew Button();
            cancelButton->Text = "CANCEL";
            cancelButton->Location = Point(395, 572);
            cancelButton->Size = Drawing::Size(175, 43);
            cancelButton->BackColor = fieldColor;
            cancelButton->ForeColor = Color::White;
            cancelButton->FlatStyle = FlatStyle::Flat;

            cancelButton->Click += gcnew EventHandler(
                this, &CreateUserForm::OnCancel
            );

            this->Controls->Add(cancelButton);

            statusLabel = gcnew Label();
            statusLabel->Location = Point(35, 625);
            statusLabel->Size = Drawing::Size(540, 27);
            statusLabel->ForeColor = Color::OrangeRed;
            statusLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 9.0f
            );

            this->Controls->Add(statusLabel);

            this->AcceptButton = saveButton;
            this->CancelButton = cancelButton;
        }
    };
}
