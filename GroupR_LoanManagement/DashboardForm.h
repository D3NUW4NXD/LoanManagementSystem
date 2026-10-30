#include "AuditLogViewerForm.h"
#include "ReportsDashboardForm.h"
#pragma once
// GROUPR_PREMIUM_UI_V1

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "AuthenticationService.h"
#include "DatabaseConnection.h"
#include "ApplicantManagementForm.h"
#include "GuarantorManagementForm.h"
#include "LoanManagementForm.h"
#include "RepaymentScheduleForm.h"
#include "PaymentManagementForm.h"
#include "UserManagementForm.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Reflection;

namespace GroupR_LoanManagement
{
    public ref class DashboardForm : public Form
    {
    private:
        AuthenticatedUser^ currentUser;
        Panel^ sidebar;
        Panel^ workspace;
        Panel^ mainBody;
        FlowLayoutPanel^ bodyFlow;
        Panel^ welcomeCard;
        TableLayoutPanel^ metricsLayout;
        Panel^ actionsCard;
        Label^ applicantsValue;
        Label^ loansValue;
        Label^ pendingValue;
        Label^ overdueValue;
        Label^ connectionLabel;

        Color bg = Color::FromArgb(12, 21, 17);
        Color sideBg = Color::FromArgb(19, 34, 26);
        Color cardBg = Color::FromArgb(25, 41, 32);
        Color green = Color::FromArgb(56, 211, 126);
        Color white = Color::FromArgb(239, 246, 241);
        Color muted = Color::FromArgb(160, 181, 167);

        System::Drawing::Font^ FontOf(
            float size, bool bold)
        {
            return gcnew System::Drawing::Font(
                "Segoe UI", size,
                bold ? FontStyle::Bold : FontStyle::Regular);
        }

        Label^ TextLabel(
            String^ value, int x, int y,
            int w, int h, float size,
            Color color, bool bold)
        {
            Label^ result = gcnew Label();
            result->Text = value;
            result->Location = Point(x, y);
            result->Size = System::Drawing::Size(w, h);
            result->Font = FontOf(size, bold);
            result->ForeColor = color;
            result->BackColor = Color::Transparent;
            result->AutoEllipsis = true;
            return result;
        }

        Button^ NavButton(
            String^ caption, String^ key,
            bool enabled)
        {
            Button^ b = gcnew Button();
            b->Text = "   " + caption;
            b->Tag = key;
            b->Width = 226;
            b->Height = 47;
            b->TextAlign =
                ContentAlignment::MiddleLeft;
            b->FlatStyle = FlatStyle::Flat;
            b->FlatAppearance->BorderSize = 0;
            b->BackColor = (key == "dashboard") ? Color::FromArgb(34, 75, 49) : sideBg;
            b->ForeColor = (key == "dashboard") ? green : (enabled ? white : muted);
            b->Font = FontOf(10, false);
            b->Margin =
                System::Windows::Forms::Padding(
                    8, 2, 8, 2);
            b->Cursor = enabled ?
                Cursors::Hand : Cursors::Default;
            b->Enabled = enabled;

            b->Click += gcnew EventHandler(
                this, &DashboardForm::Navigate);

            return b;
        }

        Button^ ActionButton(
            String^ title, String^ key)
        {
            Button^ b = gcnew Button();
            b->Text = title;
            b->Tag = key;
            b->Size = System::Drawing::Size(185, 49);
            b->Margin =
                System::Windows::Forms::Padding(
                    7, 9, 7, 7);
            b->BackColor =
                Color::FromArgb(29, 127, 74);
            b->ForeColor = white;
            b->FlatStyle = FlatStyle::Flat;
            b->FlatAppearance->BorderSize = 0;
            b->Font = FontOf(9, true);
            b->Cursor = Cursors::Hand;
            b->Click += gcnew EventHandler(
                this, &DashboardForm::Navigate);
            return b;
        }

        Panel^ MetricCard(
            String^ caption, Label^% number)
        {
            Panel^ p = gcnew Panel();
            p->Dock = DockStyle::Fill;
            p->BackColor = cardBg;
            p->Margin =
                System::Windows::Forms::Padding(6);

            p->Controls->Add(
                TextLabel(caption, 18, 19,
                          185, 24, 9, muted, false));

            number = TextLabel(
                "—", 18, 49,
                185, 56, 27, green, true);

            p->Controls->Add(number);

            Panel^ metricAccent = gcnew Panel();
            metricAccent->Dock = DockStyle::Top;
            metricAccent->Height = 3;
            metricAccent->BackColor = green;
            p->Controls->Add(metricAccent);

            p->Controls->Add(
                TextLabel("FROM DATABASE",
                    18, 108, 185, 20, 8, muted, false));
            return p;
        }

        void AddSidebar()
        {
            sidebar = gcnew Panel();
            sidebar->Dock = DockStyle::Left;
            sidebar->Width = 242;
            sidebar->BackColor = sideBg;

            Panel^ branding = gcnew Panel();
            branding->Dock = DockStyle::Top;
            branding->Height = 92;

            branding->Controls->Add(
                TextLabel("GROUP R.", 23, 17,
                          190, 35, 21, green, true));

            branding->Controls->Add(
                TextLabel("FINANCIAL WORKSPACE", 24, 54,
                          195, 22, 9, muted, true));

            Panel^ identity = gcnew Panel();
            identity->Dock = DockStyle::Top;
            identity->Height = 95;
            identity->BackColor =
                Color::FromArgb(23, 41, 30);

            identity->Controls->Add(
                TextLabel(
                    currentUser->FullName,
                    22, 15, 195, 32,
                    11, white, true));

            identity->Controls->Add(
                TextLabel(
                    currentUser->RoleName,
                    22, 49, 195, 22,
                    9, green, false));

            FlowLayoutPanel^ navigation =
                gcnew FlowLayoutPanel();

            navigation->Dock = DockStyle::Fill;
            navigation->FlowDirection =
                FlowDirection::TopDown;
            navigation->WrapContents = false;
            navigation->AutoScroll = true;
            navigation->BackColor = sideBg;
            navigation->Padding =
                System::Windows::Forms::Padding(
                    0, 12, 0, 12);

            navigation->Controls->Add(
                NavButton("DASHBOARD", "dashboard", true));
            navigation->Controls->Add(
                NavButton("APPLICANTS", "applicants", true));
            navigation->Controls->Add(
                NavButton("GUARANTORS", "guarantors", true));
            navigation->Controls->Add(
                NavButton("FINANCIAL WORKSPACE", "loans", true));
            navigation->Controls->Add(
                NavButton("ELIGIBILITY", "eligibility", true));
            navigation->Controls->Add(
                NavButton("REPAYMENT SCHEDULE", "repayments", true));

            // These modules are not yet implemented.
            navigation->Controls->Add(
                NavButton("PAYMENTS", "payments", true));
            navigation->Controls->Add(
                NavButton("REPORTS", "reports", true));
            navigation->Controls->Add(
                NavButton("AUDIT LOGS", "audit", (currentUser != nullptr && (currentUser->RoleID == 1 || currentUser->RoleID == 2))));

            if (currentUser->RoleID == 1)
            {
                navigation->Controls->Add(
                    NavButton("USER MANAGEMENT", "users", true));
            }

            Panel^ bottom = gcnew Panel();
            bottom->Dock = DockStyle::Bottom;
            bottom->Height = 86;
            bottom->BackColor = sideBg;

            Button^ logout = gcnew Button();
            logout->Text = "LOG OUT";
            logout->Location = Point(17, 14);
            logout->Size =
                System::Drawing::Size(205, 45);
            logout->BackColor =
                Color::FromArgb(106, 43, 48);
            logout->ForeColor = white;
            logout->FlatStyle = FlatStyle::Flat;
            logout->FlatAppearance->BorderSize = 0;
            logout->Font = FontOf(10, true);
            logout->Cursor = Cursors::Hand;
            logout->Click += gcnew EventHandler(
                this, &DashboardForm::LogoutClicked);

            bottom->Controls->Add(logout);

            sidebar->Controls->Add(navigation);
            sidebar->Controls->Add(bottom);
            sidebar->Controls->Add(identity);
            sidebar->Controls->Add(branding);

            this->Controls->Add(sidebar);
        }

        void AddHeader()
        {
            Panel^ header = gcnew Panel();
            header->Dock = DockStyle::Top;
            header->Height = 91;
            header->BackColor =
                Color::FromArgb(21, 33, 26);

            header->Controls->Add(
                TextLabel("Overview",
                    27, 17, 440, 37,
                    19, white, true));

            header->Controls->Add(
                TextLabel(
                    "Your financial operations workspace",
                    28, 55, 450, 25,
                    9, muted, false));

            Label^ dateLabel = TextLabel(
                DateTime::Now.ToString("dd MMM yyyy"),
                0, 29, 190, 24,
                10, muted, false);

            dateLabel->Dock = DockStyle::Right;
            dateLabel->TextAlign =
                ContentAlignment::MiddleCenter;

            header->Controls->Add(dateLabel);
            Panel^ headerAccent = gcnew Panel();
            headerAccent->Dock = DockStyle::Bottom;
            headerAccent->Height = 2;
            headerAccent->BackColor =
                Color::FromArgb(37, 99, 64);
            header->Controls->Add(headerAccent);

            workspace->Controls->Add(header);
        }

        void AddBody()
        {
            mainBody = gcnew Panel();
            mainBody->Dock = DockStyle::Fill;
            mainBody->BackColor = bg;
            mainBody->Padding =
                System::Windows::Forms::Padding(22);

            bodyFlow = gcnew FlowLayoutPanel();
            bodyFlow->Dock = DockStyle::Fill;
            bodyFlow->FlowDirection =
                FlowDirection::TopDown;
            bodyFlow->WrapContents = false;
            bodyFlow->AutoScroll = true;
            bodyFlow->BackColor = bg;

            welcomeCard = gcnew Panel();
            welcomeCard->Height = 125;
            welcomeCard->BackColor =
                Color::FromArgb(24, 62, 41);
            welcomeCard->Margin =
                System::Windows::Forms::Padding(
                    0, 2, 0, 16);

            welcomeCard->Controls->Add(
                TextLabel(
                    "Good to see you, " +
                    currentUser->FullName,
                    25, 20, 750, 40,
                    18, white, true));

            welcomeCard->Controls->Add(
                TextLabel(
                    "Track applications, decisions "
                    "and repayments in one secure workspace.",
                    26, 69, 750, 30,
                    10, muted, false));

            Panel^ welcomeAccent = gcnew Panel();
            welcomeAccent->Dock = DockStyle::Left;
            welcomeAccent->Width = 5;
            welcomeAccent->BackColor = green;
            welcomeCard->Controls->Add(welcomeAccent);

            metricsLayout = gcnew TableLayoutPanel();
            metricsLayout->Height = 148;
            metricsLayout->ColumnCount = 4;
            metricsLayout->RowCount = 1;
            metricsLayout->Margin =
                System::Windows::Forms::Padding(
                    0, 0, 0, 18);

            for (int i = 0; i < 4; i++)
            {
                metricsLayout->ColumnStyles->Add(
                    gcnew ColumnStyle(
                        SizeType::Percent, 25.0f));
            }

            metricsLayout->Controls->Add(
                MetricCard("TOTAL APPLICANTS",
                           applicantsValue), 0, 0);

            metricsLayout->Controls->Add(
                MetricCard("TOTAL LOANS",
                           loansValue), 1, 0);

            metricsLayout->Controls->Add(
                MetricCard("PENDING APPROVALS",
                           pendingValue), 2, 0);

            metricsLayout->Controls->Add(
                MetricCard("OVERDUE INSTALLMENTS",
                           overdueValue), 3, 0);

            actionsCard = gcnew Panel();
            actionsCard->Height = 230;
            actionsCard->BackColor = cardBg;
            actionsCard->Margin =
                System::Windows::Forms::Padding(
                    0, 0, 0, 16);

            actionsCard->Controls->Add(
                TextLabel(
                    "QUICK ACTIONS",
                    21, 17, 370, 30,
                    13, white, true));

            FlowLayoutPanel^ actions =
                gcnew FlowLayoutPanel();
            actions->Location = Point(16, 78);
            actions->Height = 139;
            actions->Anchor =
                (AnchorStyles)(
                    (int)AnchorStyles::Top |
                    (int)AnchorStyles::Left |
                    (int)AnchorStyles::Right);

            actions->AutoScroll = true;
            actions->WrapContents = true;

            actions->Controls->Add(
                ActionButton("APPLICANTS", "applicants"));
            actions->Controls->Add(
                ActionButton("GUARANTORS", "guarantors"));
            actions->Controls->Add(
                ActionButton("LOANS", "loans"));
            actions->Controls->Add(
                ActionButton("REPAYMENTS", "repayments"));

            actions->Controls->Add(
                ActionButton("PAYMENTS", "payments"));

            actions->Controls->Add(
                ActionButton("REPORTS", "reports"));

            actionsCard->Controls->Add(
                TextLabel(
                    "Jump directly into your daily operations",
                    21, 48, 480, 23,
                    9, muted, false));

            actionsCard->Controls->Add(actions);

            Panel^ systemCard = gcnew Panel();
            systemCard->Name = "SystemCard";
            systemCard->Height = 88;
            systemCard->BackColor = cardBg;
            systemCard->Margin =
                System::Windows::Forms::Padding(0);

            systemCard->Controls->Add(
                TextLabel(
                    "DATABASE & SYSTEM STATUS",
                    20, 12, 330, 26,
                    12, white, true));

            connectionLabel = TextLabel(
                "Checking database connection...",
                21, 44, 740, 26,
                10, muted, false);

            systemCard->Controls->Add(connectionLabel);

            bodyFlow->Controls->Add(welcomeCard);
            bodyFlow->Controls->Add(metricsLayout);
            bodyFlow->Controls->Add(actionsCard);
            bodyFlow->Controls->Add(systemCard);

            mainBody->Controls->Add(bodyFlow);
            workspace->Controls->Add(mainBody);

            mainBody->Resize +=
                gcnew EventHandler(
                    this, &DashboardForm::BodyResized);

            BodyResized(nullptr, EventArgs::Empty);
        }

        void BodyResized(
            Object^ sender, EventArgs^ e)
        {
            if (bodyFlow == nullptr)
                return;

            int width = Math::Max(
                700, mainBody->ClientSize.Width - 50);

            for each (Control^ c in bodyFlow->Controls)
                c->Width = width;

            if (actionsCard != nullptr)
            {
                for each (Control^ c
                    in actionsCard->Controls)
                {
                    FlowLayoutPanel^ f =
                        dynamic_cast<FlowLayoutPanel^>(c);

                    if (f != nullptr)
                        f->Width = width - 32;
                }
            }
        }

        int CountRows(
            SqlConnection^ con, String^ sql)
        {
            SqlCommand^ cmd = gcnew SqlCommand(sql, con);
            return Convert::ToInt32(cmd->ExecuteScalar());
        }

        void RefreshMetrics()
        {
            SqlConnection^ con =
                DatabaseConnection::CreateConnection();

            try
            {
                con->Open();

                applicantsValue->Text =
                    CountRows(con,
                        "SELECT COUNT(*) FROM dbo.Applicants"
                    ).ToString();

                loansValue->Text =
                    CountRows(con,
                        "SELECT COUNT(*) FROM dbo.Loans"
                    ).ToString();

                pendingValue->Text =
                    CountRows(con,
                        "SELECT COUNT(*) FROM dbo.Loans "
                        "WHERE LoanStatus=N'Pending'"
                    ).ToString();

                overdueValue->Text =
                    CountRows(con,
                        "SELECT COUNT(*) "
                        "FROM dbo.RepaymentSchedule "
                        "WHERE DueDate < CAST(GETDATE() AS date) "
                        "AND PaymentStatus <> N'Paid'"
                    ).ToString();

                connectionLabel->Text =
                    "Database connected  |  " +
                    DateTime::Now.ToString(
                        "dd MMM yyyy  HH:mm");
                connectionLabel->ForeColor = green;
            }
            catch (Exception^ ex)
            {
                applicantsValue->Text = "—";
                loansValue->Text = "—";
                pendingValue->Text = "—";
                overdueValue->Text = "—";

                connectionLabel->Text =
                    "Database unavailable: " + ex->Message;
                connectionLabel->ForeColor =
                    Color::FromArgb(237, 133, 133);
            }
            finally
            {
                delete con;
            }
        }

        void OpenExistingModule(Type^ targetType)
        {
            if (targetType == nullptr)
                throw gcnew InvalidOperationException(
                    "Module type unavailable.");

            Form^ module = nullptr;

            array<Type^>^ parameterTypes =
                gcnew array<Type^>(1);
            parameterTypes[0] =
                AuthenticatedUser::typeid;

            ConstructorInfo^ ctor =
                targetType->GetConstructor(
                    parameterTypes);

            if (ctor != nullptr)
            {
                array<Object^>^ args =
                    gcnew array<Object^>(1);
                args[0] = currentUser;

                module = dynamic_cast<Form^>(
                    ctor->Invoke(args));
            }
            else
            {
                ctor = targetType->GetConstructor(
                    Type::EmptyTypes);

                if (ctor != nullptr)
                    module = dynamic_cast<Form^>(
                        ctor->Invoke(
                            gcnew array<Object^>(0)));
            }

            if (module == nullptr)
                throw gcnew InvalidOperationException(
                    "No compatible constructor was found "
                    "for " + targetType->Name +
                    ". The existing module was not changed.");

            try
            {
                module->StartPosition =
                    FormStartPosition::CenterParent;

                module->ShowDialog(this);
            }
            finally
            {
                delete module;
            }

            RefreshMetrics();
        }

        void Navigate(Object^ sender, EventArgs^ e)
        {
            Button^ button =
                dynamic_cast<Button^>(sender);

            if (button == nullptr)
                return;

            String^ key =
                safe_cast<String^>(button->Tag);

            try
            {
                if (key == "dashboard")
                {
                    RefreshMetrics();
                }
                else if (key == "applicants")
                {
                    OpenExistingModule(
                        ApplicantManagementForm::typeid);
                }
                else if (key == "guarantors")
                {
                    OpenExistingModule(
                        GuarantorManagementForm::typeid);
                }
                else if (key == "loans" ||
                         key == "eligibility")
                {
                    OpenExistingModule(
                        LoanManagementForm::typeid);
                }
                else if (key == "repayments")
                {
                    OpenExistingModule(
                        RepaymentScheduleForm::typeid);
                }
                else if (key == "payments")
                {
                    OpenExistingModule(
                        PaymentManagementForm::typeid);
                }
                else if (key == "reports")
            {
                OpenExistingModule(ReportsDashboardForm::typeid);
            }
            else if (key == "audit")
            {
                OpenExistingModule(AuditLogViewerForm::typeid);
            }
            else if (key == "users" &&
                         currentUser->RoleID == 1)
                {
                    OpenExistingModule(
                        UserManagementForm::typeid);
                }
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(
                    ex->Message,
                    "Module Navigation",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
            }
        }

        void LogoutClicked(
            Object^ sender, EventArgs^ e)
        {
            System::Windows::Forms::DialogResult choice =
                MessageBox::Show(
                    "Do you want to log out?",
                    "Confirm Logout",
                    MessageBoxButtons::YesNo,
                    MessageBoxIcon::Question);

            if (choice ==
                System::Windows::Forms::DialogResult::Yes)
            {
                // Close the authenticated dashboard.
                // The application's login flow determines
                // whether the login window is shown again.
                this->Close();
            }
        }

        void DashboardShown(
            Object^ sender, EventArgs^ e)
        {
            RefreshMetrics();
        }

    public:
        DashboardForm(AuthenticatedUser^ user)
        {
            if (user == nullptr)
                throw gcnew ArgumentNullException("user");

            currentUser = user;

            this->Text =
                "Group R | Financial Operations Dashboard";
            this->BackColor = bg;
            this->ForeColor = white;
            this->ClientSize =
                System::Drawing::Size(1270, 800);
            this->MinimumSize =
                System::Drawing::Size(1020, 650);
            this->StartPosition =
                FormStartPosition::CenterScreen;

            workspace = gcnew Panel();
            workspace->Dock = DockStyle::Fill;
            workspace->BackColor = bg;

            this->Controls->Add(workspace);

            AddSidebar();
            AddHeader();
            AddBody();

            this->Shown += gcnew EventHandler(
                this, &DashboardForm::DashboardShown);
        }
    };
}



