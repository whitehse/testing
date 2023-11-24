#include <stdlib.h> 
#include <math.h>

double simple_interest (double P, double r, int t) {
  // P = Principal
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // returns the accumulated amount (A), or pricipal plus simple interest.
  // Formula (latex): A = P(1 + rt)
  return P * (1 + r*t);
}

double compound_interest (double P, double r, int t, int m) {
  // P = Principal
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of conversions, typically 1
  // returns the accumulated amount (A), or pricipal plus compounded interest.
  // Formula (latex): A = P(1 + \frac{r}{m})^{mt}
  return P * pow(1 + (r/m), m*t);
}

double continuous_interest (double P, double r, int t) {
  // P = Principal
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // returns the accumulated amount (A), or pricipal plus continuous interest.
  // Formula (latex): A = Pe^{rt}
  return P * exp((double)r*t);
}

double effective_rate (double r, int m) {
  // r = interest rate, in decimal form
  // m = the number of conversions
  // returns the effective simple compounded rate that would equal the r compounded interest rate
  // over m periods (i.e. m years)
  // Formula (latex): eff = (1 + \frac{r}{m})^{m} - 1
  return pow(1 + r/m, m) - 1;
}

double present_value_compounded_interest (double A, double r, int t, int m) {
  // A = Accumulated amount
  // r = interest rate, in decimal form
  // t = time, or number of interest periods, such as the number of years for yearly interest
  // m = the number of conversions
  // returns the present value of a future compounded interest accumulated amount, expressed
  // in today's currency
  // Formula (latex): PV = A(1 + \frac{r}{m})^{-mt}
  return A * pow(1 + r/m, -m * t);
}

