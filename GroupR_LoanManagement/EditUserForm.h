#pragma once

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "UserAdminEditService.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class EditUserForm : public Form
    {
    private:
        AuthenticatedUser^ currentUser;
        int targetUserID;

        TextBox^ fullNameBox;
        TextBox^ emailBox;
        Label^ statusLabel;

        Color backgroundColor = Color::FromArgb(15, 24, 20);
        Color inputColor = Color::FromArgb(31, 44, 39);
        Color accent = Color::FromArgb(34, 197, 94);

        Label^ AddLabel(String^ text, int x, int y, int width)
        {
            Label^ label = gcnew Label();
            label->Text = text;
            label->Location = Point(x, y);
            label->Size = Drawing::Size(width, 27);
            label->ForeColor = Color::White;
            label->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            this->Controls->Add(label);
            return label;
        }

        TextBox^ AddField(int x, int y)
        {
            TextBox^ field = gcnew TextBox();
            field->Location = Point(x, y);
            field->Size = Drawing::Size(460, 34);
            field->Font = gcnew Drawing::Font(
                "Segoe UI", 11.0f);
            field->BackColor = inputColor;
            field->ForeColor = Color::White;
            field->BorderStyle = BorderStyle::FixedSingle;
            this->Controls->Add(field);
            return field;
        }

        void SaveChanges(Object^ sender, EventArgs^ e)
        {
            try
            {
                UserAdminEditService::UpdateDetails(
                    currentUser,
                    targetUserID,
                    fullNameBox->Text,
                    emailBox->Text);

                this->DialogResult =
                    System::Windows::Forms::DialogResult::OK;

                this->Close();
            }
            catch (ArgumentException^ ex)
            {
                statusLabel->Text = ex->Message;
            }
            catch (UnauthorizedAccessException^)
            {
                statusLabel->Text =
                    "Super Administrator authorization required.";
            }
            catch (Exception^)
            {
                statusLabel->Text =
                    "Unable to save changes. Check database connection.";
            }
        }

    public:
        EditUserForm(AuthenticatedUser^ actor, int userID)
        {
            currentUser = actor;
            targetUserID = userID;

            UserEditRecord^ record =
                UserAdminEditService::GetUser(actor, userID);

            this->Text = "Group R - Edit User";
            this->ClientSize = Drawing::Size(540, 465);
            this->StartPosition = FormStartPosition::CenterParent;
            this->BackColor = backgroundColor;
            this->ForeColor = Color::White;
            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->MinimizeBox = false;

            Label^ title = AddLabel(
                "EDIT USER DETAILS", 35, 25, 460);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 20.0f, FontStyle::Bold);
            title->ForeColor = accent;
            title->Height = 45;

            AddLabel("Username: " + record->Username,
                     35, 88, 460);

            AddLabel("Role: " + record->RoleName,
                     35, 122, 460);

            AddLabel("Full Name", 35, 172, 460);
            fullNameBox = AddField(35, 202);
            fullNameBox->Text = record->FullName;
            fullNameBox->MaxLength = 150;

            AddLabel("Email Address (optional)", 35, 258, 460);
            emailBox = AddField(35, 288);
            emailBox->Text = record->Email;
            emailBox->MaxLength = 254;

            Button^ save = gcnew Button();
            save->Text = "SAVE CHANGES";
            save->Location = Point(35, 359);
            save->Size = Drawing::Size(290, 42);
            save->BackColor = accent;
            save->ForeColor = Color::White;
            save->FlatStyle = FlatStyle::Flat;
            save->FlatAppearance->BorderSize = 0;
            save->Click += gcnew EventHandler(
                this, &EditUserForm::SaveChanges);
            this->Controls->Add(save);

            Button^ cancel = gcnew Button();
            cancel->Text = "CANCEL";
            cancel->Location = Point(340, 359);
            cancel->Size = Drawing::Size(155, 42);
            cancel->BackColor = inputColor;
            cancel->ForeColor = Color::White;
            cancel->FlatStyle = FlatStyle::Flat;
            cancel->Click += gcnew EventHandler(
                this, &EditUserForm::CancelChanges);
            this->Controls->Add(cancel);

            statusLabel = AddLabel("", 35, 420, 460);
            statusLabel->ForeColor = Color::OrangeRed;

            this->AcceptButton = save;
            this->CancelButton = cancel;
        }

    private:
        void CancelChanges(Object^ sender, EventArgs^ e)
        {
            this->DialogResult =
                System::Windows::Forms::DialogResult::Cancel;

            this->Close();
        }
    };
}
