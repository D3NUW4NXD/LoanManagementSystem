#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "GuarantorListService.h"
#include "GuarantorProfileForm.h"
#include "EditGuarantorForm.h"
#include "GuarantorRegistrationForm.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class GuarantorManagementForm : public Form
    {
    // GROUPR_PHASE94_V2
protected:
    virtual void OnShown(System::EventArgs^ e) override
    {
        System::Windows::Forms::Form::OnShown(e);
        GroupRModernTheme::Apply(this);
    }

    private:
        AuthenticatedUser^ currentUser;
        DataTable^ allGuarantors;

        DataGridView^ guarantorsGrid;
        TextBox^ searchBox;
        Label^ countLabel;
        Label^ statusLabel;

        void SearchGuarantors()
        {
            if (allGuarantors == nullptr)
                return;

            String^ query = searchBox->Text->Trim();

            if (query->Length == 0)
            {
                guarantorsGrid->DataSource = allGuarantors;
                countLabel->Text =
                    "TOTAL GUARANTORS: " +
                    allGuarantors->Rows->Count.ToString();
                return;
            }

            DataTable^ filtered = allGuarantors->Clone();

            for each (DataRow^ row in allGuarantors->Rows)
            {
                bool matched = false;

                for each (DataColumn^ column
                    in allGuarantors->Columns)
                {
                    Object^ value = row[column];

                    if (value == nullptr ||
                        value == DBNull::Value)
                        continue;

                    String^ text = Convert::ToString(value);

                    if (text->IndexOf(
                        query,
                        StringComparison::OrdinalIgnoreCase) >= 0)
                    {
                        matched = true;
                        break;
                    }
                }

                if (matched)
                    filtered->ImportRow(row);
            }

            guarantorsGrid->DataSource = filtered;

            countLabel->Text =
                "MATCHING GUARANTORS: " +
                filtered->Rows->Count.ToString();
        }

        void OnRegisterGuarantor(Object^ sender, EventArgs^ e)
        {
            try
            {
                GuarantorRegistrationForm^ form =
                    gcnew GuarantorRegistrationForm(currentUser);

                if (form->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadGuarantors(sender, e);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Guarantor Registration",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnEditGuarantor(Object^ sender, EventArgs^ e)
        {
            if (guarantorsGrid->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Select a guarantor to edit.",
                    "Edit Guarantor",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                return;
            }

            try
            {
                DataGridViewRow^ row =
                    guarantorsGrid->SelectedRows[0];

                Object^ value =
                    row->Cells["GuarantorID"]->Value;

                if (value == nullptr || value == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Unable to read Guarantor ID.");

                int guarantorID = Convert::ToInt32(value);

                EditGuarantorForm^ form =
                    gcnew EditGuarantorForm(
                        currentUser, guarantorID);

                if (form->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadGuarantors(sender, e);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Edit Guarantor",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnViewGuarantorProfile(
            Object^ sender, EventArgs^ e)
        {
            if (guarantorsGrid->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Select a guarantor first.",
                    "Guarantor Profile",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                return;
            }

            try
            {
                DataGridViewRow^ row =
                    guarantorsGrid->SelectedRows[0];

                Object^ value =
                    row->Cells["GuarantorID"]->Value;

                if (value == nullptr ||
                    value == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Unable to read Guarantor ID.");

                int guarantorID = Convert::ToInt32(value);

                GuarantorProfileForm^ form =
                    gcnew GuarantorProfileForm(
                        currentUser, guarantorID);

                form->ShowDialog(this);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Guarantor Profile",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void OnGuarantorDoubleClick(
            Object^ sender,
            DataGridViewCellEventArgs^ e)
        {
            if (e->RowIndex < 0)
                return;

            guarantorsGrid->ClearSelection();
            guarantorsGrid->Rows[e->RowIndex]->Selected = true;

            OnViewGuarantorProfile(
                sender, EventArgs::Empty);
        }
        void OnSearch(Object^ sender, EventArgs^ e)
        {
            SearchGuarantors();
        }

        void LoadGuarantors(Object^ sender, EventArgs^ e)
        {
            try
            {
                allGuarantors =
                    GuarantorListService::GetAllGuarantors(
                        currentUser);

                SearchGuarantors();

                statusLabel->ForeColor =
                    Color::FromArgb(74, 222, 128);

                statusLabel->Text =
                    allGuarantors->Rows->Count == 0
                    ? "No guarantor records found."
                    : "Guarantor records loaded successfully.";
            }
            catch (UnauthorizedAccessException^)
            {
                allGuarantors = nullptr;
                guarantorsGrid->DataSource = nullptr;
                countLabel->Text = "ACCESS DENIED";
                statusLabel->ForeColor = Color::OrangeRed;
                statusLabel->Text =
                    "Please sign in again.";
            }
            catch (Exception^ ex)
            {
                allGuarantors = nullptr;
                guarantorsGrid->DataSource = nullptr;
                countLabel->Text = "UNABLE TO LOAD";
                statusLabel->ForeColor = Color::OrangeRed;
                statusLabel->Text = ex->Message;
            }
        }

    public:
        GuarantorManagementForm(AuthenticatedUser^ actor)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;
            allGuarantors = nullptr;

            this->Text = "Group R - Guarantor Management";
            this->ClientSize = Drawing::Size(1120, 720);
            this->StartPosition =
                FormStartPosition::CenterParent;

            this->BackColor =
                Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;

            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;

            Label^ title = gcnew Label();
            title->Text = "GUARANTOR MANAGEMENT";
            title->Location = Point(35, 28);
            title->Size = Drawing::Size(980, 46);
            title->ForeColor =
                Color::FromArgb(74, 222, 128);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 23.0f, FontStyle::Bold);

            this->Controls->Add(title);

            Label^ subtitle = gcnew Label();
            subtitle->Text =
                "View and search guarantor records";
            subtitle->Location = Point(35, 82);
            subtitle->Size = Drawing::Size(850, 28);
            subtitle->ForeColor = Color::Silver;
            subtitle->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            this->Controls->Add(subtitle);

            searchBox = gcnew TextBox();
            searchBox->Location = Point(35, 140);
            searchBox->Size = Drawing::Size(530, 36);
            searchBox->Font = gcnew Drawing::Font(
                "Segoe UI", 12.0f);
            searchBox->BackColor =
                Color::FromArgb(31, 44, 39);
            searchBox->ForeColor = Color::White;
            searchBox->BorderStyle =
                BorderStyle::FixedSingle;

            searchBox->TextChanged +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::OnSearch);

            this->Controls->Add(searchBox);

            Button^ refreshButton = gcnew Button();
            refreshButton->Text = "REFRESH";
            refreshButton->Location = Point(585, 140);
            refreshButton->Size = Drawing::Size(160, 36);
            refreshButton->BackColor =
                Color::FromArgb(22, 163, 74);
            refreshButton->ForeColor = Color::White;
            refreshButton->FlatStyle = FlatStyle::Flat;
            refreshButton->FlatAppearance->BorderSize = 0;

            refreshButton->Click +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::LoadGuarantors);

            this->Controls->Add(refreshButton);
            Button^ viewGuarantorProfileButton =
                gcnew Button();

            viewGuarantorProfileButton->Text =
                "VIEW PROFILE";

            viewGuarantorProfileButton->Location =
                Point(765, 223);

            viewGuarantorProfileButton->Size =
                Drawing::Size(290, 36);

            viewGuarantorProfileButton->BackColor =
                Color::FromArgb(31, 94, 65);

            viewGuarantorProfileButton->ForeColor =
                Color::White;

            viewGuarantorProfileButton->FlatStyle =
                FlatStyle::Flat;

            viewGuarantorProfileButton->
                FlatAppearance->BorderSize = 0;

            viewGuarantorProfileButton->Click +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::
                        OnViewGuarantorProfile);

            this->Controls->Add(viewGuarantorProfileButton);
            Button^ editGuarantorButton = gcnew Button();
            editGuarantorButton->Text = "EDIT SELECTED";
            editGuarantorButton->Location = Point(765, 181);
            editGuarantorButton->Size = Drawing::Size(290, 36);
            editGuarantorButton->BackColor =
                Color::FromArgb(31, 94, 65);
            editGuarantorButton->ForeColor = Color::White;
            editGuarantorButton->FlatStyle = FlatStyle::Flat;
            editGuarantorButton->FlatAppearance->BorderSize = 0;

            editGuarantorButton->Click +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::OnEditGuarantor);

            this->Controls->Add(editGuarantorButton);
            Button^ registerGuarantorButton = gcnew Button();
            registerGuarantorButton->Text = "+ REGISTER GUARANTOR";
            registerGuarantorButton->Location = Point(765, 140);
            registerGuarantorButton->Size = Drawing::Size(290, 36);
            registerGuarantorButton->BackColor =
                Color::FromArgb(22, 163, 74);
            registerGuarantorButton->ForeColor = Color::White;
            registerGuarantorButton->FlatStyle = FlatStyle::Flat;
            registerGuarantorButton->FlatAppearance->BorderSize = 0;

            registerGuarantorButton->Click +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::OnRegisterGuarantor);

            this->Controls->Add(registerGuarantorButton);

            guarantorsGrid = gcnew DataGridView();
            guarantorsGrid->Location = Point(35, 280);
            guarantorsGrid->Size = Drawing::Size(1050, 338);

            guarantorsGrid->ReadOnly = true;
            guarantorsGrid->AllowUserToAddRows = false;
            guarantorsGrid->AllowUserToDeleteRows = false;
            guarantorsGrid->RowHeadersVisible = false;
            guarantorsGrid->MultiSelect = false;

            guarantorsGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;

            guarantorsGrid->AutoGenerateColumns = true;
            guarantorsGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::DisplayedCells;

            guarantorsGrid->BackgroundColor =
                Color::FromArgb(31, 44, 39);
            guarantorsGrid->BorderStyle =
                BorderStyle::None;
            guarantorsGrid->GridColor =
                Color::FromArgb(55, 75, 60);

            guarantorsGrid->EnableHeadersVisualStyles = false;
            guarantorsGrid->ColumnHeadersHeight = 40;
            guarantorsGrid->RowTemplate->Height = 34;

            guarantorsGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(26, 65, 43);

            guarantorsGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            guarantorsGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(31, 44, 39);

            guarantorsGrid->DefaultCellStyle->ForeColor =
                Color::White;

            guarantorsGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(22, 101, 52);

            guarantorsGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            guarantorsGrid->AlternatingRowsDefaultCellStyle->BackColor =
                Color::FromArgb(37, 54, 45);

            this->Controls->Add(guarantorsGrid);
            guarantorsGrid->CellDoubleClick +=
                gcnew DataGridViewCellEventHandler(
                    this,
                    &GuarantorManagementForm::
                        OnGuarantorDoubleClick);

            countLabel = gcnew Label();
            countLabel->Text = "Loading guarantors...";
            countLabel->Location = Point(35, 637);
            countLabel->Size = Drawing::Size(600, 28);
            countLabel->ForeColor = Color::White;
            countLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f, FontStyle::Bold);
            this->Controls->Add(countLabel);

            statusLabel = gcnew Label();
            statusLabel->Location = Point(35, 673);
            statusLabel->Size = Drawing::Size(1045, 30);
            statusLabel->ForeColor =
                Color::FromArgb(74, 222, 128);
            statusLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);
            this->Controls->Add(statusLabel);

            this->Shown +=
                gcnew EventHandler(
                    this,
                    &GuarantorManagementForm::LoadGuarantors);
        }
    };
}



