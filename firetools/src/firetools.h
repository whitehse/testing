double amortization_PMT (double P, double r, int t, int m);
double amortization_n (double P, double PMT, double r, int m);
double amortization_r (double P, double PMT, int m, int n);
double amortization_P (double PMT, double r, int m, int n/*months*/);
double amortization_P_after_n_payments (double PMT, double r, int m, int n, int payed);
double future_annuity_value_end_of_period (double PP, double r, int t, int m);
double future_annuity_value_beginning_of_period (double PP, double r, int t, int m);
double present_value_of_annuity (double PP, double r, int t, int m);
double annuity_PMT (double FV, double r, int t, int m);
double simple_interest (double P, double r, int t);
double compound_interest (double P, double r, int t, int m);
double continuous_interest (double P, double r, int t);
double effective_rate (double r, int m);
double present_value_compounded_interest (double A, double r, int t, int m);

/*
// Create struct type for a loan.
struct loan {
    double loan_amount;
    double current_interest_rate;
    int months;
    double monthly_payment;
    double total_payment;
    double total_interest_paid_to_date;
    double total_interest_paid_in_future;
    char fixed_or_variable;
};

// Create struct type for a loan payment.
struct loan_payment {
    double payment;
    double payment_interest;
    double payment_principal;
    double interest_paid;
    double principal_paid;
    double balance;
    time_t payment_date;
};

struct loan_schedule_tailq {
    struct loan_payment payment;
    TAILQ_ENTRY(loan_schedule_tailq) loan_schedule_entries;
};

TAILQ_HEAD(loan_schedule_tailq_head, loan_schedule_tailq);
*/

// Convert double to currency string using strfmon.
char *currency(double amount);

// Calculate a loan payment, given a loan amount, interest rate, and loan length in months
double calc_loan_payment (double p/*loanAmount*/, double interestRate, int n/*months*/);

// Calculate the length of a loan, given the loan amount, interest rate, and the monthly payment
double calc_loan_months (double loanAmount, double interestRate, double monthlyPayment);

// Calculate the interest rate of a loan, given the loan amount, monthly payment, and loan length in months
double calc_interest_rate (double p /*loanAmount*/, double a /*monthlyPayment*/, int n /*months*/);

// Calculate the loan amount, give the interest rate, monthly payment, and loan length in months
double calc_loan_amount (double interestRate, double m/*monthlyPayment*/, int n/*months*/);

// Given a loan amount, interest rate, and loan length in months, calculate the monthly payment
// and return as a loan payment schedule struct.
void calc_loan_payment_schedule (double p/*loanAmount*/, double interestRate, int n/*months*/,
        struct loan_schedule_tailq_head *loan_payment_schedule_head);
