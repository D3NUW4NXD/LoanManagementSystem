#pragma once

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "AuthenticationService.h"
#include "DashboardForm.h"
#include "RegisterForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class LoginForm : public Form
    {
    private:
        Panel^ leftPane;
        Panel^ rightPane;
        Panel^ loginCard;
        TextBox^ usernameBox;
        TextBox^ passwordBox;
        Button^ signInButton;
        Label^ statusLabel;

        Color bg = Color::FromArgb(13, 22, 18);
        Color green = Color::FromArgb(56, 211, 126);
        Color muted = Color::FromArgb(158, 180, 165);

        Label^ AddText(
            Control^ parent, String^ content,
            int x, int y, int w, int h,
            float size, bool bold, Color color)
        {
            Label^ label = gcnew Label();
            label->Text = content;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(w, h);
            label->Font = gcnew Drawing::Font(
                "Segoe UI", size,
                bold ? FontStyle::Bold : FontStyle::Regular);
            label->ForeColor = color;
            label->BackColor = Color::Transparent;
            label->AutoEllipsis = true;
            parent->Controls->Add(label);
            return label;
        }

        TextBox^ Input(int y, bool password)
        {
            TextBox^ box = gcnew TextBox();
            box->Location = Point(36, y);
            box->Size = Drawing::Size(408, 43);
            box->AutoSize = false;
            box->Font = gcnew Drawing::Font("Segoe UI", 12);
            box->BackColor = Color::FromArgb(29, 44, 36);
            box->ForeColor = Color::White;
            box->BorderStyle = BorderStyle::FixedSingle;
            box->UseSystemPasswordChar = password;
            loginCard->Controls->Add(box);
            return box;
        }

        void ArrangeCard(Object^ sender, EventArgs^ e)
        {
            if (rightPane != nullptr && loginCard != nullptr)
            {
                loginCard->Left = Math::Max(
                    18, (rightPane->ClientSize.Width - loginCard->Width) / 2);
                loginCard->Top = Math::Max(
                    28, (rightPane->ClientSize.Height - loginCard->Height) / 2);
            }
        }

        void TogglePassword(Object^ sender, EventArgs^ e)
        {
            CheckBox^ toggle = safe_cast<CheckBox^>(sender);
            passwordBox->UseSystemPasswordChar = !toggle->Checked;
        }

        void OpenRegistration(Object^ sender, EventArgs^ e)
        {
            RegisterForm^ form = gcnew RegisterForm();

            try
            {
                form->ShowDialog(this);
            }
            finally
            {
                delete form;
            }
        }

        void SignIn(Object^ sender, EventArgs^ e)
        {
            signInButton->Enabled = false;
            statusLabel->Text = "Verifying account...";

            try
            {
                AuthenticatedUser^ user =
                    AuthenticationService::Login(
                        usernameBox->Text, passwordBox->Text);

                passwordBox->Clear();

                if (user == nullptr)
                {
                    statusLabel->Text =
                        "Invalid credentials or account awaiting activation.";
                    return;
                }

                if (user->RoleID < 1 || user->RoleID > 3)
                {
                    statusLabel->Text = "Unsupported account role.";
                    return;
                }

                statusLabel->Text = "";

                DashboardForm^ dashboard = gcnew DashboardForm(user);
                Hide();

                try
                {
                    dashboard->ShowDialog(this);
                }
                finally
                {
                    delete dashboard;
                    usernameBox->Clear();
                    passwordBox->Clear();
                    Show();
                    usernameBox->Focus();
                }
            }
            catch (Exception^) {

                passwordBox->Clear();
                statusLabel->Text =
                    "Unable to sign in. Please check the connection.";
            }
            finally
            {
                signInButton->Enabled = true;
            }
        }

    public:
        LoginForm()
        {
            Text = "Group R | Sign In";
            ClientSize = Drawing::Size(1120, 700);
            MinimumSize = Drawing::Size(980, 660);
            StartPosition = FormStartPosition::CenterScreen;
            BackColor = bg;
            ForeColor = Color::White;
            AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;

            rightPane = gcnew Panel();
            rightPane->Dock = DockStyle::Fill;
            rightPane->BackColor = bg;

            leftPane = gcnew Panel();
            leftPane->Dock = DockStyle::Left;
            leftPane->Width = 445;
            leftPane->BackColor = Color::FromArgb(20, 39, 28);

            Panel^ accent = gcnew Panel();
            accent->Location = Point(42, 70);
            accent->Size = Drawing::Size(48, 5);
            accent->BackColor = green;
            leftPane->Controls->Add(accent);

            AddText(leftPane, "GROUP R.", 42, 99, 350, 66,
                33, true, Color::White);

            AddText(leftPane, "LOAN MANAGEMENT SYSTEM", 44, 174, 355, 30,
                11, true, green);

            AddText(leftPane, "Financial operations,", 42, 278, 370, 54,
                23, true, Color::White);

            AddText(leftPane, "simplified.", 42, 326, 370, 55,
                26, true, green);

            AddText(leftPane,
                "An organized workspace for loan applications,",
                44, 409, 370, 28, 10, false, muted);

            AddText(leftPane,
                "account management and financial reporting.",
                44, 437, 370, 28, 10, false, muted);

            Label^ footer = AddText(leftPane,
                "GROUP R  /  SECURE STAFF ACCESS",
                44, 619, 370, 26, 9, true, muted);
            footer->Anchor = AnchorStyles::Left | AnchorStyles::Bottom;

            loginCard = gcnew Panel();
            loginCard->Size = Drawing::Size(480, 570);
            loginCard->BackColor = bg;

            AddText(loginCard, "Welcome back", 36, 30, 420, 54,
                26, true, Color::White);

            AddText(loginCard, "Sign in to your Group R workspace",
                37, 91, 415, 27, 10, false, muted);

            AddText(loginCard, "USERNAME", 36, 156, 380, 24,
                9, true, muted);
            usernameBox = Input(185, false);
            usernameBox->MaxLength = 100;

            AddText(loginCard, "PASSWORD", 36, 256, 380, 24,
                9, true, muted);
            passwordBox = Input(285, true);
            passwordBox->MaxLength = 128;

            CheckBox^ showPassword = gcnew CheckBox();
            showPassword->Text = "Show password";
            showPassword->Location = Point(36, 344);
            showPassword->Size = Drawing::Size(190, 27);
            showPassword->ForeColor = muted;
            showPassword->CheckedChanged +=
                gcnew EventHandler(this, &LoginForm::TogglePassword);
            loginCard->Controls->Add(showPassword);

            signInButton = gcnew Button();
            signInButton->Text = "SIGN IN";
            signInButton->Location = Point(36, 393);
            signInButton->Size = Drawing::Size(408, 49);
            signInButton->BackColor = Color::FromArgb(27, 174, 91);
            signInButton->ForeColor = Color::White;
            signInButton->Font = gcnew Drawing::Font(
                "Segoe UI", 11, FontStyle::Bold);
            signInButton->FlatStyle = FlatStyle::Flat;
            signInButton->FlatAppearance->BorderSize = 0;
            signInButton->FlatAppearance->MouseOverBackColor =
                Color::FromArgb(37, 194, 111);
            signInButton->Cursor = Cursors::Hand;
            signInButton->Click +=
                gcnew EventHandler(this, &LoginForm::SignIn);
            loginCard->Controls->Add(signInButton);

            Button^ createButton = gcnew Button();
            createButton->Text = "CREATE AN ACCOUNT";
            createButton->Location = Point(36, 457);
            createButton->Size = Drawing::Size(408, 44);
            createButton->BackColor = Color::FromArgb(30, 48, 37);
            createButton->ForeColor = green;
            createButton->Font = gcnew Drawing::Font(
                "Segoe UI", 10, FontStyle::Bold);
            createButton->FlatStyle = FlatStyle::Flat;
            createButton->FlatAppearance->BorderSize = 0;
            createButton->Cursor = Cursors::Hand;
            createButton->Click +=
                gcnew EventHandler(this, &LoginForm::OpenRegistration);
            loginCard->Controls->Add(createButton);

            statusLabel = AddText(loginCard, "", 36, 517,
                415, 45, 9, false,
                Color::FromArgb(250, 128, 128));

            rightPane->Controls->Add(loginCard);
            rightPane->Resize +=
                gcnew EventHandler(this, &LoginForm::ArrangeCard);

            Controls->Add(rightPane);
            Controls->Add(leftPane);

            ArrangeCard(nullptr, EventArgs::Empty);

            AcceptButton = signInButton;
        }
    };
}
