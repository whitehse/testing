/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -s 10 -t calix_axos.gperf  */
/* Computed positions: -k'4-5,10' */

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
int return_calix (struct calix_axos *axos);
#line 12 "calix_axos.gperf"
struct calix_axos;

struct calix { char *name; int number; void* data; };

#define TOTAL_KEYWORDS 46
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 24
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 77
/* maximum key range = 76, duplicates = 0 */

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
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78,  0, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78,  5, 20, 35,
      15,  5, 25,  0, 78,  0, 78, 78, 78, 20,
      40, 10, 30,  5,  5,  5,  0, 20, 35, 78,
      78, 78, 78, 78, 78, 78, 78, 78
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
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
      case 2:
        break;
    }
  return hval;
}

struct calix_axos *
in_word_set_axos (register const char *str, register size_t len)
{
  static struct calix_axos wordlist[] =
    {
      {""}, {""},
#line 22 "calix_axos.gperf"
      {"id",                       7,NULL},
      {""},
#line 34 "calix_axos.gperf"
      {"port",                     19,NULL},
      {""},
#line 35 "calix_axos.gperf"
      {"ont-id",                   20,NULL},
      {""}, {""},
#line 23 "calix_axos.gperf"
      {"name",                     8,NULL},
      {""},
#line 19 "calix_axos.gperf"
      {"detail",                   4,NULL},
      {""},
#line 30 "calix_axos.gperf"
      {"category",                 15,NULL},
#line 58 "calix_axos.gperf"
      {"omci-comm-fail",           43,NULL},
      {""},
#line 21 "calix_axos.gperf"
      {"instance-id",              6,NULL},
#line 33 "calix_axos.gperf"
      {"address",                  18,NULL},
#line 32 "calix_axos.gperf"
      {"repair-action",            17,NULL},
      {""}, {""},
#line 45 "calix_axos.gperf"
      {"ont-arrival",              30,NULL},
#line 53 "calix_axos.gperf"
      {"low-rx-opt-pwr-ne",        38,NULL},
#line 37 "calix_axos.gperf"
      {"phys-pon-slot",            22,NULL},
#line 36 "calix_axos.gperf"
      {"phys-pon-shelf",           21,NULL},
#line 59 "calix_axos.gperf"
      {"ont-sw-mismatch",          44,NULL},
#line 57 "calix_axos.gperf"
      {"loss-of-pon",              42,NULL},
#line 50 "calix_axos.gperf"
      {"ont-eth-down",             35,NULL},
#line 46 "calix_axos.gperf"
      {"ont-departure",            31,NULL},
#line 52 "calix_axos.gperf"
      {"ont-dying-gasp",           37,NULL},
#line 26 "calix_axos.gperf"
      {"alarm",                    11,NULL},
      {""}, {""},
#line 54 "calix_axos.gperf"
      {"ntp-server-reachability",  39,NULL},
#line 41 "calix_axos.gperf"
      {"vendor-id",                26,NULL},
      {""},
#line 49 "calix_axos.gperf"
      {"user-logout",              34,NULL},
#line 47 "calix_axos.gperf"
      {"notification",             32,NULL},
#line 39 "calix_axos.gperf"
      {"serial-number",            24,NULL},
      {""},
#line 29 "calix_axos.gperf"
      {"alarm-type",               14,NULL},
      {""},
#line 61 "calix_axos.gperf"
      {"ont-download-fail",        46,NULL},
#line 60 "calix_axos.gperf"
      {"ont-download-start",       45,NULL},
#line 40 "calix_axos.gperf"
      {"equipment-type",           25,NULL},
      {""}, {""},
#line 43 "calix_axos.gperf"
      {"ont-ds-sdber",             28,NULL},
#line 38 "calix_axos.gperf"
      {"phys-pon-port",            23,NULL},
#line 42 "calix_axos.gperf"
      {"eventTime",                27,NULL},
#line 51 "calix_axos.gperf"
      {"xmlns",                    36,NULL},
#line 31 "calix_axos.gperf"
      {"hw-failure-event",         16,NULL},
#line 44 "calix_axos.gperf"
      {"ont-us-sdber",             29,NULL},
#line 25 "calix_axos.gperf"
      {"prev-severity",            10,NULL},
      {""},
#line 48 "calix_axos.gperf"
      {"user-login",               33,NULL},
      {""},
#line 56 "calix_axos.gperf"
      {"dhcp-lease-termination",   41,NULL},
#line 24 "calix_axos.gperf"
      {"perceived-severity",       9,NULL},
#line 55 "calix_axos.gperf"
      {"dhcp-lease-establishment", 40,NULL},
      {""},
#line 17 "calix_axos.gperf"
      {"description",              2,NULL},
#line 20 "calix_axos.gperf"
      {"device-sequence-number",   5,NULL},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 16 "calix_axos.gperf"
      {"ont-missing",              1,NULL},
#line 27 "calix_axos.gperf"
      {"service-impacting",        12,NULL},
      {""},
#line 18 "calix_axos.gperf"
      {"probable-cause",           3,NULL},
      {""}, {""},
#line 28 "calix_axos.gperf"
      {"service-affecting",        13,NULL}
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
