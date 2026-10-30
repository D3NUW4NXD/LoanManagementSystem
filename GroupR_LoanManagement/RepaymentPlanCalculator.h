#pragma once

#using <System.dll>

using namespace System;
using namespace System::Collections::Generic;

namespace GroupR_LoanManagement
{
    public ref class RepaymentScheduleRow
    {
    public:
        int InstallmentNumber;
        DateTime DueDate;

        Decimal OpeningBalance;
        Decimal MonthlyPayment;
        Decimal PrincipalAmount;
        Decimal InterestAmount;
        Decimal ClosingBalance;
    };

    public ref class RepaymentPlanCalculator abstract sealed
    {
    private:
        static Decimal Money(Decimal amount)
        {
            return Decimal::Round(
                amount, 2,
                MidpointRounding::AwayFromZero);
        }

    public:
        static List<RepaymentScheduleRow^>^ Generate(
            Decimal principal,
            Decimal annualInterestRate,
            int termMonths,
            DateTime firstDueDate)
        {
            if (principal <= Decimal::Zero)
                throw gcnew ArgumentOutOfRangeException(
                    "principal",
                    "Loan principal must be positive.");

            if (annualInterestRate < Decimal::Zero)
                throw gcnew ArgumentOutOfRangeException(
                    "annualInterestRate",
                    "Interest rate cannot be negative.");

            if (termMonths < 1 || termMonths > 600)
                throw gcnew ArgumentOutOfRangeException(
                    "termMonths",
                    "Term must be between 1 and 600 months.");

            principal = Money(principal);

            if (principal <= Decimal::Zero)
                throw gcnew ArgumentOutOfRangeException(
                    "principal",
                    "Principal is too small after rounding.");

            Decimal monthlyRate =
                annualInterestRate / Decimal(1200);

            Decimal standardPayment;

            if (annualInterestRate == Decimal::Zero)
            {
                standardPayment =
                    Money(principal / Decimal(termMonths));
            }
            else
            {
                double rate =
                    Convert::ToDouble(annualInterestRate)
                    / 1200.0;

                double factor = Math::Pow(
                    1.0 + rate, -termMonths);

                double payment =
                    Convert::ToDouble(principal)
                    * rate / (1.0 - factor);

                if (Double::IsNaN(payment) ||
                    Double::IsInfinity(payment))
                    throw gcnew InvalidOperationException(
                        "Unable to calculate installment.");

                standardPayment = Money(
                    Convert::ToDecimal(payment));
            }

            List<RepaymentScheduleRow^>^ rows =
                gcnew List<RepaymentScheduleRow^>();

            Decimal balance = principal;

            for (int month = 1;
                 month <= termMonths;
                 month++)
            {
                RepaymentScheduleRow^ row =
                    gcnew RepaymentScheduleRow();

                row->InstallmentNumber = month;
                row->DueDate =
                    firstDueDate.AddMonths(month - 1);

                row->OpeningBalance = balance;

                Decimal interest =
                    Money(balance * monthlyRate);

                Decimal principalPart =
                    standardPayment - interest;

                // Final month pays off the exact balance.
                if (month == termMonths)
                {
                    principalPart = balance;
                }
                else
                {
                    if (principalPart <= Decimal::Zero)
                        throw gcnew InvalidOperationException(
                            "Installment is too small "
                            "to reduce the loan principal.");

                    if (principalPart > balance)
                        principalPart = balance;
                }

                row->InterestAmount = interest;
                row->PrincipalAmount = principalPart;

                row->MonthlyPayment =
                    principalPart + interest;

                balance -= principalPart;

                row->ClosingBalance = balance;

                rows->Add(row);

                if (balance == Decimal::Zero)
                    break;
            }

            if (balance != Decimal::Zero)
                throw gcnew InvalidOperationException(
                    "Repayment schedule did not settle "
                    "the full principal.");

            return rows;
        }
    };
}
