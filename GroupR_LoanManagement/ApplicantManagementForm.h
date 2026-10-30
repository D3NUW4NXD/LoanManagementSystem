#pragma once
#include "GroupRModernTheme.h"

#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Data.dll>
#using <System.Xml.dll>

#include "ApplicantService.h"
#include "ApplicantProfileForm.h"
#include "EditApplicantForm.h"
#include "ApplicantRegistrationForm.h"

using namespace System;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Windows::Forms;

namespace GroupR_LoanManagement
{
    public ref class ApplicantManagementForm : public Form
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
        DataTable^ allApplicants;

        DataGridView^ applicantsGrid;
        TextBox^ searchBox;
        Label^ countLabel;
        Label^ statusLabel;

        void FormatApplicantGrid()
        {
            if (applicantsGrid->Columns->Count == 0)
                return;

            // Keep full data accessible through View Profile.
            array<String^>^ hiddenColumns = gcnew array<String^>
            {
                "DateOfBirth",
                "Email",
                "Address",
                "Occupation",
                "Employer",
                "ExistingMonthlyDebt",
                "CreditScore",
                "CreatedByUserID",
                "CreatedAt"
            };

            for each (String^ name in hiddenColumns)
            {
                if (applicantsGrid->Columns->Contains(name))
                    applicantsGrid->Columns[name]->Visible = false;
            }

            if (applicantsGrid->Columns->Contains("ApplicantID"))
            {
                DataGridViewColumn^ column =
                    applicantsGrid->Columns["ApplicantID"];

                column->HeaderText = "ID";
                column->FillWeight = 45.0f;
            }

            if (applicantsGrid->Columns->Contains("ApplicantCode"))
            {
                DataGridViewColumn^ column =
                    applicantsGrid->Columns["ApplicantCode"];

                column->HeaderText = "APPLICANT CODE";
                column->FillWeight = 135.0f;
            }

            if (applicantsGrid->Columns->Contains("FullName"))
            {
                DataGridViewColumn^ column =
                    applicantsGrid->Columns["FullName"];

                column->HeaderText = "FULL NAME";
                column->FillWeight = 180.0f;
            }

            if (applicantsGrid->Columns->Contains("NIC"))
            {
                applicantsGrid->Columns["NIC"]->HeaderText = "NIC";
                applicantsGrid->Columns["NIC"]->FillWeight = 100.0f;
            }

            if (applicantsGrid->Columns->Contains("Phone"))
            {
                applicantsGrid->Columns["Phone"]->HeaderText = "PHONE";
                applicantsGrid->Columns["Phone"]->FillWeight = 110.0f;
            }

            if (applicantsGrid->Columns->Contains("MonthlyIncome"))
            {
                DataGridViewColumn^ column =
                    applicantsGrid->Columns["MonthlyIncome"];

                column->HeaderText = "MONTHLY INCOME (LKR)";
                column->FillWeight = 140.0f;
                column->DefaultCellStyle->Format = "N2";
                column->DefaultCellStyle->Alignment =
                    DataGridViewContentAlignment::MiddleRight;
            }

            applicantsGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;

            applicantsGrid->ClearSelection();
        }

        void ApplySearch()
        {
            if (allApplicants == nullptr)
                return;

            String^ query = searchBox->Text->Trim();

            if (query->Length == 0)
            {
                applicantsGrid->DataSource = allApplicants;
                FormatApplicantGrid();

                countLabel->Text =
                    "Total applicants: " +
                    allApplicants->Rows->Count.ToString();

                return;
            }

            DataTable^ filtered = allApplicants->Clone();

            for each (DataRow^ row in allApplicants->Rows)
            {
                bool matched = false;

                for each (DataColumn^ column
                    in allApplicants->Columns)
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

            applicantsGrid->DataSource = filtered;
            FormatApplicantGrid();

            countLabel->Text =
                "Matching applicants: " +
                filtered->Rows->Count.ToString();
        }

        void LoadApplicants(Object^ sender, EventArgs^ e)
        {
            try
            {
                allApplicants =
                    ApplicantService::GetAllApplicants(currentUser);

                ApplySearch();

                statusLabel->ForeColor =
                    Color::FromArgb(74, 222, 128);

                statusLabel->Text =
                    allApplicants->Rows->Count == 0
                    ? "No applicants yet. Use + REGISTER APPLICANT."
                    : "Applicant records loaded successfully.";
            }
            catch (UnauthorizedAccessException^)
            {
                statusLabel->ForeColor = Color::OrangeRed;
                statusLabel->Text =
                    "Access denied. Please sign in again.";
            }
            catch (Exception^)
            {
                statusLabel->ForeColor = Color::OrangeRed;
                statusLabel->Text =
                    "Unable to load applicant records.";
            }
        }

                void OnRegisterApplicant(Object^ sender, EventArgs^ e)
        {
            try
            {
                ApplicantRegistrationForm^ form =
                    gcnew ApplicantRegistrationForm(currentUser);

                if (form->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadApplicants(nullptr, EventArgs::Empty);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Applicant Management",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
                void OnEditApplicant(Object^ sender, EventArgs^ e)
        {
            if (applicantsGrid->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Please select an applicant first.",
                    "Select Applicant",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                return;
            }

            try
            {
                DataGridViewRow^ selected =
                    applicantsGrid->SelectedRows[0];

                Object^ value =
                    selected->Cells["ApplicantID"]->Value;

                if (value == nullptr || value == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Could not read the Applicant ID.");

                int applicantID = Convert::ToInt32(value);

                EditApplicantForm^ form =
                    gcnew EditApplicantForm(
                        currentUser, applicantID);

                if (form->ShowDialog(this) ==
                    System::Windows::Forms::DialogResult::OK)
                {
                    LoadApplicants(nullptr, EventArgs::Empty);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Edit Applicant",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
                void OnViewApplicantProfile(Object^ sender, EventArgs^ e)
        {
            if (applicantsGrid->SelectedRows->Count == 0)
            {
                MessageBox::Show(
                    "Select an applicant to view their profile.",
                    "Applicant Profile",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                return;
            }

            try
            {
                DataGridViewRow^ row =
                    applicantsGrid->SelectedRows[0];

                Object^ value =
                    row->Cells["ApplicantID"]->Value;

                if (value == nullptr || value == DBNull::Value)
                    throw gcnew InvalidOperationException(
                        "Unable to read Applicant ID.");

                int applicantID = Convert::ToInt32(value);

                ApplicantProfileForm^ profile =
                    gcnew ApplicantProfileForm(
                        currentUser, applicantID);

                profile->ShowDialog(this);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Applicant Profile",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }
        void OnApplicantDoubleClick(
            Object^ sender,
            DataGridViewCellEventArgs^ e)
        {
            if (e->RowIndex < 0)
                return;

            OnViewApplicantProfile(sender, EventArgs::Empty);
        }
        void OnSearch(Object^ sender, EventArgs^ e)
        {
            ApplySearch();
        }

    public:
        ApplicantManagementForm(AuthenticatedUser^ actor)
        {
            if (actor == nullptr)
                throw gcnew UnauthorizedAccessException(
                    "Authentication required.");

            currentUser = actor;
            allApplicants = nullptr;

            this->Text = "Group R - Applicant Management";
            this->ClientSize = Drawing::Size(1120, 720);
            this->StartPosition =
                FormStartPosition::CenterParent;

            this->BackColor = Color::FromArgb(15, 24, 20);
            this->ForeColor = Color::White;

            this->FormBorderStyle =
                System::Windows::Forms::FormBorderStyle::FixedSingle;

            this->MaximizeBox = false;

            Label^ title = gcnew Label();
            title->Text = "APPLICANT MANAGEMENT";
            title->Location = Point(35, 28);
            title->Size = Drawing::Size(850, 45);
            title->ForeColor =
                Color::FromArgb(74, 222, 128);
            title->Font = gcnew Drawing::Font(
                "Segoe UI", 23.0f, FontStyle::Bold);

            this->Controls->Add(title);

            Label^ subtitle = gcnew Label();
            subtitle->Text =
                "View and search loan applicant records";
            subtitle->Location = Point(35, 80);
            subtitle->Size = Drawing::Size(800, 25);
            subtitle->ForeColor = Color::Silver;
            subtitle->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            this->Controls->Add(subtitle);

            searchBox = gcnew TextBox();
            searchBox->Location = Point(35, 135);
            searchBox->Size = Drawing::Size(460, 36);
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
                    &ApplicantManagementForm::OnSearch);

            this->Controls->Add(searchBox);

            Button^ refreshButton = gcnew Button();
            refreshButton->Text = "REFRESH";
            refreshButton->Location = Point(515, 135);
            refreshButton->Size = Drawing::Size(150, 36);
            refreshButton->BackColor =
                Color::FromArgb(22, 163, 74);
            refreshButton->ForeColor = Color::White;
            refreshButton->FlatStyle = FlatStyle::Flat;
            refreshButton->FlatAppearance->BorderSize = 0;

            refreshButton->Click +=
                gcnew EventHandler(
                    this,
                    &ApplicantManagementForm::LoadApplicants);

            this->Controls->Add(refreshButton);
            Button^ viewProfileButton = gcnew Button();
            viewProfileButton->Text = "VIEW PROFILE";
            viewProfileButton->Location = Point(685, 176);
            viewProfileButton->Size = Drawing::Size(185, 36);
            viewProfileButton->BackColor =
                Color::FromArgb(31, 94, 65);
            viewProfileButton->ForeColor = Color::White;
            viewProfileButton->FlatStyle = FlatStyle::Flat;
            viewProfileButton->FlatAppearance->BorderSize = 0;

            viewProfileButton->Click +=
                gcnew EventHandler(
                    this,
                    &ApplicantManagementForm::OnViewApplicantProfile);

            this->Controls->Add(viewProfileButton);
            Button^ editApplicantButton = gcnew Button();
            editApplicantButton->Text = "EDIT SELECTED";
            editApplicantButton->Location = Point(915, 135);
            editApplicantButton->Size = Drawing::Size(170, 36);
            editApplicantButton->BackColor =
                Color::FromArgb(31, 94, 65);
            editApplicantButton->ForeColor = Color::White;
            editApplicantButton->FlatStyle = FlatStyle::Flat;
            editApplicantButton->FlatAppearance->BorderSize = 0;

            editApplicantButton->Click +=
                gcnew EventHandler(
                    this,
                    &ApplicantManagementForm::OnEditApplicant);

            this->Controls->Add(editApplicantButton);
            Button^ registerApplicantButton = gcnew Button();
            registerApplicantButton->Text = "+ REGISTER APPLICANT";
            registerApplicantButton->Location = Point(685, 135);
            registerApplicantButton->Size = Drawing::Size(215, 36);
            registerApplicantButton->BackColor =
                Color::FromArgb(22, 163, 74);
            registerApplicantButton->ForeColor = Color::White;
            registerApplicantButton->FlatStyle = FlatStyle::Flat;
            registerApplicantButton->FlatAppearance->BorderSize = 0;

            registerApplicantButton->Click +=
                gcnew EventHandler(
                    this,
                    &ApplicantManagementForm::OnRegisterApplicant);

            this->Controls->Add(registerApplicantButton);

            applicantsGrid = gcnew DataGridView();
            applicantsGrid->Location = Point(35, 245);
            applicantsGrid->Size = Drawing::Size(1050, 385);

            applicantsGrid->BackgroundColor =
                Color::FromArgb(31, 44, 39);

            applicantsGrid->BorderStyle =
                BorderStyle::None;

            applicantsGrid->GridColor =
                Color::FromArgb(55, 75, 60);

            applicantsGrid->ReadOnly = true;
            applicantsGrid->AllowUserToAddRows = false;
            applicantsGrid->AllowUserToDeleteRows = false;
            applicantsGrid->RowHeadersVisible = false;

            applicantsGrid->SelectionMode =
                DataGridViewSelectionMode::FullRowSelect;

            applicantsGrid->AutoSizeColumnsMode =
                DataGridViewAutoSizeColumnsMode::Fill;

            applicantsGrid->EnableHeadersVisualStyles = false;

            applicantsGrid->ColumnHeadersDefaultCellStyle->BackColor =
                Color::FromArgb(26, 65, 43);

            applicantsGrid->ColumnHeadersDefaultCellStyle->ForeColor =
                Color::White;

            applicantsGrid->DefaultCellStyle->BackColor =
                Color::FromArgb(31, 44, 39);

            applicantsGrid->DefaultCellStyle->ForeColor =
                Color::White;

            applicantsGrid->DefaultCellStyle->SelectionBackColor =
                Color::FromArgb(22, 101, 52);

            applicantsGrid->DefaultCellStyle->SelectionForeColor =
                Color::White;

            
            applicantsGrid->RowTemplate->Height = 34;
            applicantsGrid->ColumnHeadersHeight = 40;
            applicantsGrid->MultiSelect = false;
            applicantsGrid->AutoGenerateColumns = true;

            applicantsGrid->AlternatingRowsDefaultCellStyle->BackColor =
                Color::FromArgb(37, 54, 45);

            applicantsGrid->ColumnHeadersDefaultCellStyle->Font =
                gcnew Drawing::Font(
                    "Segoe UI", 9.0f, FontStyle::Bold);

            applicantsGrid->DefaultCellStyle->Font =
                gcnew Drawing::Font("Segoe UI", 10.0f);
            this->Controls->Add(applicantsGrid);
            applicantsGrid->CellDoubleClick +=
                gcnew DataGridViewCellEventHandler(
                    this,
                    &ApplicantManagementForm::OnApplicantDoubleClick);


            countLabel = gcnew Label();
            countLabel->Text = "Loading applicants...";
            countLabel->Location = Point(35, 646);
            countLabel->Size = Drawing::Size(500, 28);
            countLabel->ForeColor = Color::White;
            countLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            this->Controls->Add(countLabel);

            statusLabel = gcnew Label();
            statusLabel->Location = Point(35, 680);
            statusLabel->Size = Drawing::Size(1000, 28);
            statusLabel->ForeColor =
                Color::FromArgb(74, 222, 128);
            statusLabel->Font = gcnew Drawing::Font(
                "Segoe UI", 10.0f);

            this->Controls->Add(statusLabel);

            this->Shown +=
                gcnew EventHandler(
                    this,
                    &ApplicantManagementForm::LoadApplicants);
        }
    };
}





