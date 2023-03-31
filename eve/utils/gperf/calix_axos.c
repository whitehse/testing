/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -s 10 -t calix_axos.gperf  */
/* Computed positions: -k'1,5,10' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "calix_axos.gperf"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <calix_axos.h>
#include <expat.h>
#include <states.h>
int return_calix (struct calix_axos_parse *axos_parser);
#line 14 "calix_axos.gperf"
struct calix_axos_parse;

#define TOTAL_KEYWORDS 80
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 47
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 167
/* maximum key range = 165, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_axos (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168,   0, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168,  30, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
      168, 168, 168, 168, 168, 168, 168,   0,   0,  65,
        5,   0,  20,  70,   0,  35, 168, 168,  55,  15,
        0,   0,  20,   5,  40,  15,  50,  30,  85,  30,
       15, 168, 168, 168, 168, 168, 168, 168
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
      case 8:
      case 7:
      case 6:
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct calix_axos_parse *
in_word_set_axos (register const char *str, register size_t len)
{
  static struct calix_axos_parse wordlist[] =
    {
      {""}, {""}, {""},
#line 66 "calix_axos.gperf"
      {"ont",                      51,NULL,NULL,NULL},
#line 23 "calix_axos.gperf"
      {"name",                     8,NULL,NULL,NULL},
      {""}, {""},
#line 33 "calix_axos.gperf"
      {"address",                  18,NULL,NULL,NULL},
      {""},
#line 63 "calix_axos.gperf"
      {"data",                     48,NULL,NULL,NULL},
#line 69 "calix_axos.gperf"
      {"oper-state",               54,NULL,NULL,NULL},
#line 45 "calix_axos.gperf"
      {"ont-arrival",              30,NULL,NULL,NULL},
#line 50 "calix_axos.gperf"
      {"ont-eth-down",             35,NULL,NULL,NULL},
      {""},
#line 58 "calix_axos.gperf"
      {"omci-comm-fail",           43,NULL,NULL,NULL},
      {""},
#line 31 "calix_axos.gperf"
      {"hw-failure-event",         16,NULL,NULL,NULL},
#line 43 "calix_axos.gperf"
      {"ont-ds-sdber",             28,NULL,NULL,NULL},
      {""},
#line 52 "calix_axos.gperf"
      {"ont-dying-gasp",           37,NULL,NULL,NULL},
#line 26 "calix_axos.gperf"
      {"alarm",                    11,NULL,NULL,NULL},
#line 65 "calix_axos.gperf"
      {"system",                   50,NULL,NULL,NULL},
#line 61 "calix_axos.gperf"
      {"ont-download-fail",        46,NULL,NULL,NULL},
#line 60 "calix_axos.gperf"
      {"ont-download-start",       45,NULL,NULL,NULL},
#line 34 "calix_axos.gperf"
      {"port",                     19,NULL,NULL,NULL},
#line 29 "calix_axos.gperf"
      {"alarm-type",               14,NULL,NULL,NULL},
#line 16 "calix_axos.gperf"
      {"ont-missing",              1,NULL,NULL,NULL},
#line 56 "calix_axos.gperf"
      {"dhcp-lease-termination",   41,NULL,NULL,NULL},
      {""},
#line 55 "calix_axos.gperf"
      {"dhcp-lease-establishment", 40,NULL,NULL,NULL},
#line 92 "calix_axos.gperf"
      {"message-id",               77,NULL,NULL,NULL},
      {""},
#line 78 "calix_axos.gperf"
      {"onu-mac-addr",             63,NULL,NULL,NULL},
      {""},
#line 40 "calix_axos.gperf"
      {"equipment-type",           25,NULL,NULL,NULL},
#line 51 "calix_axos.gperf"
      {"xmlns",                    36,NULL,NULL,NULL},
      {""},
#line 22 "calix_axos.gperf"
      {"id",                       7,NULL,NULL,NULL},
#line 24 "calix_axos.gperf"
      {"perceived-severity",       9,NULL,NULL,NULL},
      {""},
#line 48 "calix_axos.gperf"
      {"user-login",               33,NULL,NULL,NULL},
#line 35 "calix_axos.gperf"
      {"ont-id",                   20,NULL,NULL,NULL},
#line 44 "calix_axos.gperf"
      {"ont-us-sdber",             29,NULL,NULL,NULL},
#line 39 "calix_axos.gperf"
      {"serial-number",            24,NULL,NULL,NULL},
      {""},
#line 59 "calix_axos.gperf"
      {"ont-sw-mismatch",          44,NULL,NULL,NULL},
#line 19 "calix_axos.gperf"
      {"detail",                   4,NULL,NULL,NULL},
#line 79 "calix_axos.gperf"
      {"mta-mac-addr",             64,NULL,NULL,NULL},
#line 37 "calix_axos.gperf"
      {"phys-pon-slot",            22,NULL,NULL,NULL},
#line 36 "calix_axos.gperf"
      {"phys-pon-shelf",           21,NULL,NULL,NULL},
      {""},
#line 64 "calix_axos.gperf"
      {"status",                   49,NULL,NULL,NULL},
      {""},
#line 38 "calix_axos.gperf"
      {"phys-pon-port",            23,NULL,NULL,NULL},
      {""}, {""},
#line 17 "calix_axos.gperf"
      {"description",              2,NULL,NULL,NULL},
#line 76 "calix_axos.gperf"
      {"rg-config-version",        61,NULL,NULL,NULL},
      {""},
#line 42 "calix_axos.gperf"
      {"eventTime",                27,NULL,NULL,NULL},
      {""}, {""},
#line 81 "calix_axos.gperf"
      {"product-code",             66,NULL,NULL,NULL},
#line 89 "calix_axos.gperf"
      {"ds-sdber-rate",            74,NULL,NULL,NULL},
#line 82 "calix_axos.gperf"
      {"linked-by",                67,NULL,NULL,NULL},
#line 70 "calix_axos.gperf"
      {"linked-pon",               55,NULL,NULL,NULL},
#line 57 "calix_axos.gperf"
      {"loss-of-pon",              42,NULL,NULL,NULL},
#line 47 "calix_axos.gperf"
      {"notification",             32,NULL,NULL,NULL},
#line 46 "calix_axos.gperf"
      {"ont-departure",            31,NULL,NULL,NULL},
#line 72 "calix_axos.gperf"
      {"clei",                     57,NULL,NULL,NULL},
      {""},
#line 49 "calix_axos.gperf"
      {"user-logout",              34,NULL,NULL,NULL},
#line 84 "calix_axos.gperf"
      {"up-time",                  69,NULL,NULL,NULL},
#line 25 "calix_axos.gperf"
      {"prev-severity",            10,NULL,NULL,NULL},
      {""},
#line 71 "calix_axos.gperf"
      {"model",                    56,NULL,NULL,NULL},
      {""},
#line 91 "calix_axos.gperf"
      {"ont-counters",             76,NULL,NULL,NULL},
#line 54 "calix_axos.gperf"
      {"ntp-server-reachability",  39,NULL,NULL,NULL},
      {""}, {""},
#line 21 "calix_axos.gperf"
      {"instance-id",              6,NULL,NULL,NULL},
#line 27 "calix_axos.gperf"
      {"service-impacting",        12,NULL,NULL,NULL},
      {""}, {""}, {""},
#line 85 "calix_axos.gperf"
      {"opt-signal-level",         70,NULL,NULL,NULL},
#line 28 "calix_axos.gperf"
      {"service-affecting",        13,NULL,NULL,NULL},
#line 88 "calix_axos.gperf"
      {"us-sdber-rate",            73,NULL,NULL,NULL},
#line 62 "calix_axos.gperf"
      {"rpc-reply",                47,NULL,NULL,NULL},
      {""},
#line 67 "calix_axos.gperf"
      {"vendor",                   52,NULL,NULL,NULL},
#line 94 "calix_axos.gperf"
      {"http://www.calix.com/ns/exa/base",                79,NULL,calix_doc_tag_end,NULL},
      {""},
#line 41 "calix_axos.gperf"
      {"vendor-id",                26,NULL,NULL,NULL},
      {""},
#line 74 "calix_axos.gperf"
      {"alt-version",              59,NULL,NULL,NULL},
#line 20 "calix_axos.gperf"
      {"device-sequence-number",   5,NULL,NULL,NULL},
#line 68 "calix_axos.gperf"
      {"upgrade-class",            53,NULL,NULL,NULL},
#line 18 "calix_axos.gperf"
      {"probable-cause",           3,NULL,NULL,NULL},
      {""}, {""},
#line 80 "calix_axos.gperf"
      {"mfg-serial-number",        65,NULL,NULL,NULL},
#line 87 "calix_axos.gperf"
      {"ne-opt-signal",            72,NULL,NULL,NULL},
      {""}, {""}, {""},
#line 95 "calix_axos.gperf"
      {"http://www.calix.com/ns/exa/gpon-interface-base", 80,NULL,NULL,calix_doc_char_data},
      {""}, {""}, {""}, {""},
#line 73 "calix_axos.gperf"
      {"curr-version",             58,NULL,NULL,NULL},
      {""},
#line 77 "calix_axos.gperf"
      {"curr-committed",           62,NULL,NULL,NULL},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 83 "calix_axos.gperf"
      {"range-length",             68,NULL,NULL,NULL},
#line 90 "calix_axos.gperf"
      {"counters",                 75,NULL,NULL,NULL},
#line 93 "calix_axos.gperf"
      {"urn:ietf:params:xml:ns:netconf:base:1.0",         78,calix_doc_tag_start,NULL,NULL},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 32 "calix_axos.gperf"
      {"repair-action",            17,NULL,NULL,NULL},
#line 75 "calix_axos.gperf"
      {"voip-config-version",      60,NULL,NULL,NULL},
      {""}, {""}, {""},
#line 30 "calix_axos.gperf"
      {"category",                 15,NULL,NULL,NULL},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 53 "calix_axos.gperf"
      {"low-rx-opt-pwr-ne",        38,NULL,NULL,NULL},
      {""}, {""}, {""}, {""},
#line 86 "calix_axos.gperf"
      {"tx-opt-level",             71,NULL,NULL,NULL}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash_axos (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 96 "calix_axos.gperf"

static void XMLCALL
calix_doc_tag_start(void *user_data, const XML_Char *name, const XML_Char **atts) {
}

static void XMLCALL
calix_doc_tag_end(void *user_data, const XML_Char *name) {
}

static void
calix_doc_char_data(void* user_data, const char* val, int len) {
}
