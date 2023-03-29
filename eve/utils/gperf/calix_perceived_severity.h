#include <stdlib.h>
struct calix_perceived_severity {
  char *name;
  int number;
  void* data;
};
static unsigned int
hash_perceived_severity (register const char *str, register size_t len);
struct calix_perceived_severity *
in_word_set_perceived_severity (register const char *str, register size_t len);
