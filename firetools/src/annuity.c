#include <stdlib.h> 
#include <math.h>

double future_annuity_value_end_of_period (double PP, double r, int t, int m) {
  // PP = periodic paypment
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // returns the future value of the annuity (investment) account
  // Formula (latex): FV = PP\frac{\left[(1 + \frac{r}{m})^{mt} - 1\right]}{\frac{r}{m}}
  return PP * (pow(1 + r/m, m*t) - 1) / (r/m);
}

double future_annuity_value_beginning_of_period (double PP, double r, int t, int m) {
  // PP = periodic paypment
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // returns the future value of the annuity (investment) account
  // Formula (latex): FV = PP(1 - \frac{2}{m})\frac{\left[(1 + \frac{r}{m})^{mt} - 1\right]}{\frac{r}{m}}
  return PP * (1 + (r/m)) * (pow(1 + r/m, m*t) - 1) / (r/m);
}

double present_value_of_annuity (double PP, double r, int t, int m) {
  // PP = periodic paypment
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // returns the future value of the annuity (investment) account
  // Formula (latex): PV = PP\frac{\left[1 - (1 + \frac{r}{m})^{-mt}\right]}{\frac{r}{m}}
  return PP * (1 - pow(1 + r/m, -m*t)) / (r/m);
}

double annuity_PMT (double FV, double r, int t, int m) {
  // PP = periodic paypment
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of payments per time period (e.g. 26, for 26 payments in a year)
  // returns the payment amount to reach a future value of the annuity
  // Formula (latex): PMT = \frac{\frac{r}{m}S}{(1 + \frac{r}{m})^{mt} - 1}
  return (r/m*FV) / (pow(1 + r/m, m*t) - 1);
}
