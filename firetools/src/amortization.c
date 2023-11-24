#include <stdlib.h> 
#include <math.h>

double amortization_PMT (double P, double r, int t, int m) {
  // P = Principal
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // returns the future value of the annuity (investment) account
  // Formula (latex): PMT = P\frac{\frac{r}{m}}{\left[1 - (1 + \frac{r}{m})^{-mt}\right]} \]
  return P * (r/m) / (1 - pow(1 + r/m, -m*t));
}

double amortization_n (double P, double PMT, double r, int m) {
  // P = Principal
  // PMT = Payment
  // r = interest rate, in decimal form
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // Formula (latex): N = \frac{\log\frac{PMT}{PMT - P\frac{r}{m}}}{\log(\frac{r}{m} + 1)}
  return log(PMT/(PMT - P*r/m)) / log(r/m + 1);
}

double amortization_r (double P, double PMT, int m, int n) {
  // https://math.stackexchange.com/questions/724469/finding-the-mortgage-interest-rate
  // P = Principal
  // PMT = Payment
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // This is an approximation. 1 pass seems to converge on the correct value.
  // This assumes interest is compounded "m"ly

  double r;
  double k = P/PMT;

  r = (2*(n-k)*(2*k*(n+2)+(n-1)*n)) / (k*k*(n+2)*(n+3)+2*k*n*(n*n+n-2)+(1-n)*n*n);

  // Do two passes for good measure
  for (int i=0; i<2; i++) {
    double f1 = P * (r*pow(r+1,n) / (pow(r+1,n) - 1)) - PMT;
    double f2 = P * (pow(r+1,n-1)*(pow(r+1,n+1)-n*r-r-1) / pow(pow(r+1,n)-1,2));
    r = r - f1/f2;
  }
  return r * m;
}

double amortization_P (double PMT, double r, int m, int n/*months*/) {
  // PMT = Payment
  // r = interest rate, in decimal form
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // n = number of payments
  // Formula (latex): P = PMT\frac{(\frac{r}{m}+1)^n-1}{(\frac{r}{m}+1)^n}
  return PMT*(pow(r/m+1,n)-1) / ((r/m)*pow(r/m+1,n));
}

double amortization_P_after_n_payments (double PMT, double r, int m, int n, int payed) {
  // PMT = Payment
  // r = interest rate, in decimal form
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // n = number of total payments
  // payed = number of payments made
  // returns the remaining principal after n payments have been made
  // Formula (latex): P_{new} = PMT\left[\frac{1 - (1 + \frac{r}{m})^{n-payed}}{\frac{r}{m}}\right]
  return PMT*(1 - pow(1 + r/m, -n+payed)) / (r/m);
}
