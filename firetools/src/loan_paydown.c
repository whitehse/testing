#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/queue.h>
#include <monetary.h>

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

int buf_len = 16;
char simple_locale[16];

// Convert double to currency string using strfmon.
char *currency(double amount) {
    char *string = malloc(16);
    strfmon(string, buf_len-1, "%n", amount);
    return string;
}

// Calculate a loan payment, given a loan amount, interest rate, and loan length in months
double calc_loan_payment (double p/*loanAmount*/, double interestRate, int n/*months*/) {
    double i = interestRate / 1200;
    double loan_payment = p*(i*pow(i+1,n)) / (pow(i+1,n)-1);
    return loan_payment;
}

// Calculate the length of a loan, given the loan amount, interest rate, and the monthly payment
double calc_loan_months (double loanAmount, double interestRate, double monthlyPayment) {
    // n=lg(M/(M−P*i))/Lg(i+1)
    double i = interestRate / 1200;
    double months = log(monthlyPayment/(monthlyPayment-loanAmount*i)) / log(i+1);
    return months;
}

// Calculate the interest rate of a loan, given the loan amount, monthly payment, and loan length in months
double calc_interest_rate (double p /*loanAmount*/, double a /*monthlyPayment*/, int n /*months*/) {
    // https://math.stackexchange.com/questions/724469/finding-the-mortgage-interest-rate
    // Props to user Claude Leibovici on StackExchange for this formula.
    // This is an approximation. 1 pass probably converges on the correct value. Do two
    // passes as a security blanket.
    double ir;
    double k = p/a;

    ir = (2*(n-k)*(2*k*(n+2)+(n-1)*n)) / (k*k*(n+2)*(n+3)+2*k*n*(n*n+n-2)+(1-n)*n*n);

    for (int i=0; i<2; i++) {
        double f1 = p * (ir*pow(ir+1,n) / (pow(ir+1,n) - 1)) - a;
        double f2 = p * (pow(ir+1,n-1)*(pow(ir+1,n+1)-n*ir-ir-1) / pow(pow(ir+1,n)-1,2));
        ir = ir - f1/f2;
    }

    return ir * 1200;
}

// Calculate the loan amount, give the interest rate, monthly payment, and loan length in months
double calc_loan_amount (double interestRate, double m/*monthlyPayment*/, int n/*months*/) {
    double i = interestRate / 1200;
    double loan_amount = (m*(pow(i+1,n)-1)) / (i*pow(i+1,n));
    return loan_amount;
}

// Given a loan amount, interest rate, and loan length in months, calculate the monthly payment
// and return as a loan payment schedule struct.
void calc_loan_payment_schedule (double p/*loanAmount*/, double interestRate, int n/*months*/,
        struct loan_schedule_tailq_head *loan_payment_schedule_head) {

    double payment_amt = calc_loan_payment(p, interestRate, n);
    double interest_paid = 0;
    double principal_paid = 0;
    double balance = p;

    struct loan_schedule_tailq *payment_entry;
    for (int i=0; i<n; i++) {
        payment_entry = malloc(sizeof(*payment_entry));
        payment_entry->payment.payment = payment_amt;
        double interest_amount = balance * interestRate/100 / 12;
        double principal_amount = payment_amt - interest_amount;
        interest_paid += interest_amount;
        principal_paid += principal_amount;
        balance = balance - principal_amount;
        payment_entry->payment.payment_interest = interest_amount;
        payment_entry->payment.payment_principal = principal_amount;
        payment_entry->payment.interest_paid += interest_paid;
        payment_entry->payment.principal_paid += principal_paid;
        payment_entry->payment.balance = balance;
        TAILQ_INSERT_TAIL(loan_payment_schedule_head, payment_entry, loan_schedule_entries);
    }

    return;
}

/*
void main() {
    struct loan_schedule_tailq_head *loan_schedule_head;
    loan_schedule_head = malloc(sizeof(*loan_schedule_head));
    TAILQ_INIT(loan_schedule_head);
    calc_loan_payment_schedule (119000, 4.75, 360, loan_schedule_head);
    struct loan_schedule_tailq *payment_entry;
    int i=1;
    TAILQ_FOREACH(payment_entry, loan_schedule_head, loan_schedule_entries) {
		printf("Payment %i is %.2f\n", i, payment_entry->payment.payment);
        printf("  principal: %.2f\n", payment_entry->payment.payment_principal);
        printf("  interest: %.2f\n", payment_entry->payment.payment_interest);
        printf("  total pricipal paid: %.2f\n", payment_entry->payment.principal_paid);
        printf("  total interest paid: %.2f\n", payment_entry->payment.interest_paid);
        printf("  balance: %.2f\n", payment_entry->payment.balance);
        printf("\n");
        i++;
	}
}
*/
