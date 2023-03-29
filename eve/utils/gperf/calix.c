#include <stdlib.h>
#include <stdio.h>
#include <calix.h>
#include <string.h>
//#include <calix_gperf.h>

int main(int argc, char *argv[]) {
    int i = hash("description", 11);
    printf("description = %d\n", i);
    i = hash("probable-cause", 14);
    printf("probable-cause = %d\n", i);
    i = hash("ont", 3);
    printf("ont = %d\n", i);
    i = hash("on", 2);
    printf("on = %d\n", i);
    i = hash("o", 1);
    printf("o = %d\n", i);
    i = hash("ont-id", 6);
    printf("ont-id = %d\n", i);
    i = hash("ont-id2", 7);
    printf("ont-id2 = %d\n", i);
    i = hash("ont-id3", 7);
    printf("ont-id3 = %d\n", i);
    i = hash("oo", 1);
    printf("oo = %d\n", i);
}
