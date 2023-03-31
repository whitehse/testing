#include <stdlib.h>
#include <expat.h>

struct gperf_callbacks {
  XML_StartElementHandler start_element_handler;
  XML_EndElementHandler end_element_handler;
  XML_CharacterDataHandler char_data_handler;
};

struct calix_axos_document {
    int depth;
    struct gperf_callbacks callbacks[20];
};

struct calix_axos_parse {
    char* name;
    int number;
    XML_StartElementHandler start_handler;
    XML_EndElementHandler end_handler;
    XML_CharacterDataHandler data_handler;
    //struct gperf_callbacks;
};

static unsigned int
hash_axos (register const char *str, register size_t len);

struct calix_axos_parse *
in_word_set_axos (register const char *str, register size_t len);
