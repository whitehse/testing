#include <stdlib.h>

struct calix_axos {
    char *name;
    int number;
    void* data;
};

static unsigned int
hash_axos (register const char *str, register size_t len);

struct calix_axos *
in_word_set_axos (register const char *str, register size_t len);

