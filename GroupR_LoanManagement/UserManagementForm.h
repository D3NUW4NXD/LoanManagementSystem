#include "EditUserForm.h"
#include "UserAdminEditService.h"
#pragma once
#include "ResetPasswordForm.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>

#include "UserManagementService.h"
#include "UserCreationService.h"
#include "CreateUserForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace GroupR_LoanManagement
{
    public ref class UserManagementForm : public Form
    {
    private:
        AuthenticatedUser^ currentUser;

        DataGridView^ usersGrid;
        TextBox^ searchBox;
        Label^ countLabel;
        Label^ messageLabel;

        Color bg = Color::FromArgb(15, 24, 20);
        Color panelColor = Color::FromArgb(31, 44, 39);
        Color green = Color::FromArgb(34, 197, 94);

        Label^ MakeLabel(String^ value, int x, int y,
            int width, int height, int size, bool bold)
        {
            Label^ label = gcnew Label();
            label->Text = value;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(width, height);
            label->ForeColor = Color::White;
            label->Font = gcnew Drawing::Font(
                "Segoe UI", (float)size,
                bold ? FontStyle::Bold : FontStyle::Regular
            );
            return label;
        }

        Button^ MakeButton(String^ text, int x, int y,
            int width, int height)
        {
            Button^ button = gcnew Button();
            button->Text = text;
            button->Location = Point(x, y);
            button->Size = Drawing::Size(width, height);
            button->BackColor = green;
            button->ForeColor = Color::White;
            button->FlatStyle = FlatStyle::Flat;
            button->FlatAppearance->BorderSize = 0;
            button->Font = gcnew Drawing::Font(
                "Segoe UI", 10, FontStyle::Bold
            );
            return button;
        }

        void LoadUsers()
        {
            List<UserSummary^>^ users =
                UserManagementService::GetAllUsers(currentUser);

            usersGrid->Rows->Clear();

            String^ filter = searchBox->Text->Trim();

            int visibleCount = 0;

            for each (UserSummary^ user in users)
            {
                if (!String::IsNullOrWhiteSpace(filter))
                {
                    bool matches =
                        user->Username->IndexOf(
                            filter,
                            StringComparison::OrdinalIgnoreCase
                        ) >= 0 ||
                        user->FullName->IndexOf(
                            filter,
                            StringComparison::OrdinalIgnoreCase
                        ) >= 0;

                    if (!matches)
                        continue;
                }

                usersGrid->Rows->Add(
                    user->UserID,
                    user->Username,
                    user->FullName,
                    user->RoleName,
                    user->IsActive ? "Active" : "Inactive"
                );

                visibleCount++;
            }

            countLabel->Text =
                "Showing " + visibleCount.ToString() +
                " of " + users->Count.ToString() + " users";
        }

        void OnSearch(Object^ sender, EventArgs^ e)
        {
            try
            {
                LoadUsers();
            }
            catch (Exception^)
            {
                messageLabel->Text = "Unable to search users.";
            }
        }

        void OnRefresh(Object^ sender, EventArgs^ e)
        {
            try
            {
                LoadUsers();
                messageLabel->Text = "User list refreshed.";
            }
            catch (Exception^)
            {
                messageLabel->Text = "Unable to load users.";
            }
        }

        int SelectedUserID()
        {
            if (usersGrid->SelectedRows->Count == 0)
                throw gcnew InvalidOperationException(
                    "Select a user from the table first.");

            DataGridViewRow^ row =
                usersGrid->SelectedRows[0];

            if (row->Cells["UserID"]->Value == nullptr)
                throw gcnew InvalidOperationException(
                    "Invalid user selection.");

            return Convert::ToInt32(
                row->Cells["UserID"]->Value);
        }

        void OnEditSelectedUser(Object^ sender, EventArgs^ e)
        {
            try
            {
                int userID = SelectedUserID();

                EditUserForm^ form =
                    gcnew EditUserForm(currentUser, userID);

                if (form->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadUsers();
                    messageLabel->Text =
                        "User details updated successfully.";
                }
            }
            catch (InvalidOperationException^ ex)
            {
                messageLabel->Text = ex->Message;
            }
            catch (UnauthorizedAccessException^)
            {
                messageLabel->Text =
                    "Access denied.";
            }
            catch (Exception^)
            {
                messageLabel->Text =
                    "Unable to edit the selected account.";
            }
        }

        void OnChangeAccountStatus(Object^ sender, EventArgs^ e)
        {
            try
            {
                int userID = SelectedUserID();

                UserEditRecord^ user =
                    UserAdminEditService::GetUser(
                        currentUser, userID);

                bool newStatus = !user->IsActive;
                String^ action =
                    newStatus ? "activate" : "deactivate";

                String^ question =
                    "Are you sure you want to " + action +
                    " account '" + user->Username + "'?";

                if (MessageBox::Show(
                    question,
                    "Confirm Account Status",
                    MessageBoxButtons::YesNo,
                    MessageBoxIcon::Question
                ) != System::Windows::Forms::DialogResult::Yes)
                {
                    return;
                }

                UserAdminEditService::SetActive(
                    currentUser,
                    userID,
                    newStatus);

                LoadUsers();

                messageLabel->Text =
                    "Account status updated successfully.";
            }
            catch (InvalidOperationException^ ex)
            {
                messageLabel->Text = ex->Message;
            }
            catch (UnauthorizedAccessException^)
            {
                messageLabel->Text = "Access denied.";
            }
            catch (Exception^)
            {
                messageLabel->Text =
                    "Unable to update account status.";
            }
        }
        void OnResetSelectedPassword(Object^ sender, EventArgs^ e)
        {
            try
            {
                int selectedID = SelectedUserID();

                ResetPasswordForm^ resetForm =
                    gcnew ResetPasswordForm(
                        currentUser,
                        selectedID);

                if (resetForm->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadUsers();

                    messageLabel->Text =
                        "Password reset completed successfully.";
                }
            }
            catch (InvalidOperationException^ ex)
            {
                messageLabel->Text = ex->Message;
            }
            catch (UnauthorizedAccessException^)
            {
                messageLabel->Text = "Access denied.";
            }
            catch (Exception^)
            {
                messageLabel->Text =
                    "Unable to open Password Reset.";
            }
        }
        void OnCreateUser(Object^ sender, EventArgs^ e)
        {
            try
            {
                CreateUserForm^ form =
                    gcnew CreateUserForm(currentUser);

                System::Windows::Forms::DialogResult result =
                    form->ShowDialog(this);

                if (result ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadUsers();
                    messageLabel->Text =
                        "New user created successfully.";
                }
            }
            catch (Exception^)
            {
                messageLabel->Text =
                    "Unable to complete user creation.";
            }
        }

    public:
        UserManagementForm(AuthenticatedUser^ user)
        {
            if (user == nullptr || user->RoleID != 1)
                throw gcnew UnauthorizedAccessException(
                    "Super Administrator access required."
                );

            currentUser = user;

            this->Text = "Group R - User Management";
            this->ClientSize = Drawing::Size(1000, 700);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = bg;
            this->ForeColor = Color::White;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;

            this->Controls->Add(MakeLabel(
                "USER MANAGEMENT",
                35, 25, 600, 45, 23, true
            ));

            this->Controls->Add(MakeLabel(
                "Manage system accounts and access roles",
                35, 78, 650, 30, 11, false
            ));

            searchBox = gcnew TextBox();
            searchBox->Location = Point(35, 140);
            searchBox->Size = Drawing::Size(360, 35);
            searchBox->Font = gcnew Drawing::Font(
                "Segoe UI", 12
            );
            searchBox->BackColor = panelColor;
            searchBox->ForeColor = Color::White;
            searchBox->BorderStyle = BorderStyle::FixedSingle;
            searchBox->TextChanged += gcnew EventHandler(
                this, &UserManagementForm::OnSearch
            );
            this->Controls->Add(searchBox);

            Button^ refreshButton =
                MakeButton("REFRESH", 420, 140, 130, 35);
            refreshButton->Click += gcnew EventHandler(
                this, &UserManagementForm::OnRefresh
            );
            this->Controls->Add(refreshButton);

            Button^ createButton =
                MakeButton("+ CREATE USER", 755, 140, 200, 35);
            createButton->Click += gcnew EventHandler(
                this, &UserManagementForm::OnCreateUser
            );
            this->Controls->Add(createButton);

            usersGrid = gcnew DataGridView();

            // GROUPR_USER_UI_V1
            usersGrid->BackgroundColor = Color::FromArgb(17, 29, 23);
            usersGrid->GridColor = Color::FromArgb(43, 66, 51);
            usersGrid->BorderStyle = BorderStyle::None;
            usersGrid->CellBorderStyle =
                DataGridViewCellBorderStyle::SingleHorizontal;
            usersGrid->EnableHeadersVisualStyles = false;
            usersGrid->ColumnHeadersBorderStyle =
                DataGridViewHeaderBorderStyle::None;
            usersGrid->ColumnHeadersHeight = 48;
            usersGrid->ColumnHeadersHeightSizeMode =
                DataGridViewColumnHeadersHeightSizeMode::DisableResizing;
            usersGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(26, 57, 38);
            usersGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::FromArgb(92, 230, 149);
            usersGrid->ColumnHeadersDefaultCellStyle->Font =
                gcnew System::Drawing::Font(
                    "Segoe UI", 10, FontStyle::Bold);
            usersGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(19, 34, 26);
            usersGrid->DefaultCellStyle->ForeColor = Color::White;
            usersGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(36, 104, 65);
            usersGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;
            usersGrid->DefaultCellStyle->Font =
                gcnew System::Drawing::Font("Segoe UI", 10);
            usersGrid->DefaultCellStyle->Padding =
                System::Windows::Forms::Padding(8, 5, 8, 5);
            usersGrid->AlternatingRowsDefaultCellStyle->BackColor =
                Color::FromArgb(24, 43, 32);
            usersGrid->RowTemplate->Height = 42;
            usersGrid->RowHeadersVisible = false;
            usersGrid->Location = Point(35, 205);
            usersGrid->Size = Drawing::Size(920, 350);
            usersGrid->BackgroundColor = panelColor;
            usersGrid->GridColor = Color::FromArgb(55, 75, 60);
            usersGrid->BorderStyle = BorderStyle::None;
            usersGrid->AllowUserToAddRows = false;
            usersGrid->AllowUserToDeleteRows = false;
            usersGrid->ReadOnly = true;
            usersGrid->RowHeadersVisible = false;
            usersGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;
            usersGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;
            usersGrid->EnableHeadersVisualStyles = false;

            usersGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(26, 65, 43);
            usersGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            usersGrid->DefaultCellStyle->BackColor = panelColor;
            usersGrid->DefaultCellStyle->ForeColor = Color::White;
            usersGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(22, 101, 52);
            usersGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            usersGrid->Columns->Add("UserID", "ID");
            usersGrid->Columns->Add("Username", "Username");
            usersGrid->Columns->Add("FullName", "Full Name");
            usersGrid->Columns->Add("Role", "Role");
            usersGrid->Columns->Add("Status", "Status");

            this->Controls->Add(usersGrid);
            Button^ editButton =
                MakeButton("EDIT USER", 35, 565, 170, 38);

            editButton->Click += gcnew EventHandler(
                this, &UserManagementForm::OnEditSelectedUser);

            this->Controls->Add(editButton);

            Button^ statusButton =
                MakeButton("APPROVE / DEACTIVATE ACCOUNT",
                           220, 565, 230, 38);

            statusButton->Click += gcnew EventHandler(
                this, &UserManagementForm::OnChangeAccountStatus);

            this->Controls->Add(statusButton);
            Button^ resetPasswordButton =
                MakeButton(
                    "RESET PASSWORD",
                    465, 565, 205, 38);

            resetPasswordButton->Click +=
                gcnew EventHandler(
                    this,
                    &UserManagementForm::OnResetSelectedPassword);

            this->Controls->Add(resetPasswordButton);


            countLabel = MakeLabel(
                "Loading users...", 35, 614, 500, 30, 10, false
            );
            this->Controls->Add(countLabel);

            messageLabel = MakeLabel(
                "", 35, 645, 900, 30, 10, false
            );
            messageLabel->ForeColor = green;
            this->Controls->Add(messageLabel);

            // Load data after the form is shown.
            this->Shown += gcnew EventHandler(
                this, &UserManagementForm::OnRefresh
            );
        }
    };
}



