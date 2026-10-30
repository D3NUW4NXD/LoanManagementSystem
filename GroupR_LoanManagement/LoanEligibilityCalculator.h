#pragma once

#using <System.dll>

using namespace System;

namespace GroupR_LoanManagement
{
    public ref class LoanEligibilityResult
    {
    public:
        Decimal MonthlyIncome;
        Decimal ExistingMonthlyDebt;
        Decimal RequestedAmount;
        Decimal AnnualInterestRate;
        int TermMonths;

        Decimal EstimatedInstallment;
        Decimal TotalMonthlyDebt;
        Decimal DebtToIncomePercent;
        Decimal RemainingMonthlyIncome;

        String^ AssessmentNote;
    };

    public ref class LoanEligibilityCalculator abstract sealed
    {
    public:
        static LoanEligibilityResult^ Calculate(
            Decimal income,
            Decimal existingDebt,
            Decimal amount,
            Decimal annualRate,
            int months)
        {
            if (income <= Decimal::Zero)
                throw gcnew ArgumentException(
                    "Monthly income must be greater than zero.");

            if (existingDebt < Decimal::Zero)
                throw gcnew ArgumentException(
                    "Existing debt cannot be negative.");

            if (amount <= Decimal::Zero)
                throw gcnew ArgumentException(
                    "Loan amount must be greater than zero.");

            if (annualRate < Decimal::Zero ||
                annualRate > Decimal(100))
                throw gcnew ArgumentException(
                    "Interest rate must be between 0 and 100.");

            if (months < 1 || months > 360)
                throw gcnew ArgumentException(
                    "Loan term must be between 1 and 360 months.");

            double principal = Decimal::ToDouble(amount);
            double monthlyRate =
                Decimal::ToDouble(annualRate) / 1200.0;

            double installment = 0.0;

            if (monthlyRate == 0.0)
            {
                installment = principal / months;
            }
            else
            {
                double factor =
                    Math::Pow(1.0 + monthlyRate, months);

                installment =
                    principal * monthlyRate * factor /
                    (factor - 1.0);
            }

            if (Double::IsNaN(installment) ||
                Double::IsInfinity(installment))
                throw gcnew InvalidOperationException(
                    "Unable to calculate installment.");

            LoanEligibilityResult^ result =
                gcnew LoanEligibilityResult();

            result->MonthlyIncome = income;
            result->ExistingMonthlyDebt = existingDebt;
            result->RequestedAmount = amount;
            result->AnnualInterestRate = annualRate;
            result->TermMonths = months;

            result->EstimatedInstallment =
                Decimal::Round(
                    Convert::ToDecimal(installment),
                    2,
                    MidpointRounding::AwayFromZero);

            result->TotalMonthlyDebt =
                existingDebt + result->EstimatedInstallment;

            result->DebtToIncomePercent =
                Decimal::Round(
                    result->TotalMonthlyDebt /
                    income * Decimal(100),
                    2,
                    MidpointRounding::AwayFromZero);

            result->RemainingMonthlyIncome =
                income - result->TotalMonthlyDebt;

            result->AssessmentNote =
                "Preliminary affordability calculation only. "
                "A lending decision requires the approved "
                "eligibility policy, credit review, and "
                "administrator authorization.";

            return result;
        }
    };
}
