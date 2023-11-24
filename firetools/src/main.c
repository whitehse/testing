#include <stdio.h>
#include <interest.c>
#include <firetools_config.h>

int main() {
  // printf() displays the string inside quotation
  //printf("Hello, World!\n");
  printf("Project Name = %s\n", project_name);
  printf("Project Version = %s\n", project_version);
  double i = simple_interest(100000.0, .05, 5);
  printf("The amount of interest is %f\n", i);
  return 1;
}
