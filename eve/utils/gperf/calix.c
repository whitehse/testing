#include <stdlib.h>
#include <stdio.h>
//#include <calix.h>
#include <string.h>
#include <calix_axos.h>
#include <calix_perceived_severity.h>

int main(int argc, char *argv[]) {
/*
    int i = hash_axos("description", 11);
    printf("description = %d\n", i);
    i = hash_axos("probable-cause", 14);
    printf("probable-cause = %d\n", i);
    i = hash_axos("ont", 3);
    printf("ont = %d\n", i);
    i = hash_axos("on", 2);
    printf("on = %d\n", i);
    i = hash_axos("o", 1);
    printf("o = %d\n", i);
    i = hash_axos("ont-id", 6);
    printf("ont-id = %d\n", i);
    i = hash_axos("ont-id2", 7);
    printf("ont-id2 = %d\n", i);
    i = hash_axos("ont-id3", 7);
    printf("ont-id3 = %d\n", i);
    i = hash_axos("oo", 1);
    printf("oo = %d\n", i);
*/
    //i = hash_perceived_severity("CLEAR", 5);
    //printf("CEARL = %d\n", i);

    struct calix_axos *axos;
    axos = in_word_set_axos ("description", 11);
    printf("Name = %s\n", axos->name);

    struct calix_perceived_severity *severity;
    severity = in_word_set_perceived_severity ("CLEAR", 5);
    printf("Name = %s\n", severity->name);
}
