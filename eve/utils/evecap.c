#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

/*
$ extcap_example.py --extcap-interfaces --extcap-version=4.0

        extcap {version=1.0}{help=Some help url}
        interface {value=example1}{display=Example interface 1 for extcap}
        interface {value=example2}{display=Example interface 2 for extcap}

$ extcap_example.py --extcap-interface IFACE --extcap-dlts

        dlt {number=147}{name=USER1}{display=Demo Implementation for Extcap}

$ extcap_example.py --extcap-interface <iface> --extcap-config

        arg {number=0}{call=--delay}{display=Time delay}{tooltip=Time delay between packages}{type=integer}{range=1,15}{required=true}
        arg {number=1}{call=--message}{display=Message}{tooltip=Package message content}{placeholder=Please enter a message here ...}{type=string}
        arg {number=2}{call=--verify}{display=Verify}{tooltip=Verify package content}{type=boolflag}
        arg {number=3}{call=--remote}{display=Remote Channel}{tooltip=Remote Channel Selector}{type=selector}
        arg {number=4}{call=--server}{display=IP address for log server}{type=string}{validation=\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b}
        value {arg=3}{value=if1}{display=Remote1}{default=true}
        value {arg=3}{value=if2}{display=Remote2}{default=false}

        arg {number=0}{call=--channel}{display=Wi-Fi Channel}{type=integer}{required=true}
        arg {number=1}{call=--chanflags}{display=Channel Flags}{type=radio}
        arg {number=2}{call=--interface}{display=Interface}{type=selector}
        value {arg=0}{range=1,11}
        value {arg=1}{value=ht40p}{display=HT40+}
        value {arg=1}{value=ht40m}{display=HT40-}
        value {arg=1}{value=ht20}{display=HT20}
        value {arg=2}{value=wlan0}{display=wlan0}

extcapbin --extcap-interface IFACE [params] --capture [--extcap-capture-filter CFILTER] --fifo FIFO
*/

/* Flag set by ‘--verbose’. */
static int verbose_flag;
static int interface_flag;
static int interfaces_flag;
static int config_flag;
static int dlts_flag;
static int capture_flag;
static int capture_filter_flag;
static int fifo_flag;
static char interface[1000];
static char capture_filter[1000];
static char fifo[1000];

int main (int argc, char **argv) {
  int c;
  interface_flag=0;
  capture_flag=0;
  capture_filter_flag=0;
  fifo_flag=0;

  while (1) {
      static struct option long_options[] = {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          {"brief",   no_argument,       &verbose_flag, 0},
	  {"extcap-interfaces", no_argument, &interfaces_flag, 1},
	  {"extcap-dlts", no_argument,   &dlts_flag, 1},
	  {"extcap-config", no_argument,   &config_flag, 1},
	  {"capture", no_argument,   &capture_flag, 1},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"add",     no_argument,       0, 'a'},
          {"append",  no_argument,       0, 'b'},
          {"delete",  required_argument, 0, 'd'},
          {"create",  required_argument, 0, 'c'},
          {"file",    required_argument, 0, 'f'},
          {"extcap-version", required_argument, 0, 'v'},
          {"extcap-interface", required_argument, 0, 'i'},
          {"extcap-capture-filter", required_argument, 0, 'p'},
          {"fifo", required_argument, 0, 'o'},
          {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "abc:d:f:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c) {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'a':
          puts ("option -a\n");
          break;

        case 'b':
          puts ("option -b\n");
          break;

        case 'c':
          printf ("option -c with value `%s'\n", optarg);
          break;

        case 'd':
          printf ("option -d with value `%s'\n", optarg);
          break;

        case 'f':
          printf ("option -f with value `%s'\n", optarg);
          break;

        case 'v':
          //printf ("option -v with value `%s'\n", optarg);
          break;

        case 'i':
	  strcpy(interface, optarg);
	  interface_flag = 1;
          break;

        case 'p':
	  strcpy(capture_filter, optarg);
	  capture_filter_flag = 1;
          break;

        case 'o':
	  strcpy(fifo, optarg);
	  fifo_flag = 1;
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
  }

  /* Instead of reporting ‘--verbose’
     and ‘--brief’ as they are encountered,
     we report the final status resulting from them. */
  if (verbose_flag)
    puts ("verbose flag is set");

  if (interfaces_flag) {
    //puts ("extcap-interfaces flag is set");
    puts("extcap {version=1.0}{help=Some help url}");
    puts("interface {value=eno2}{display=evecap: eno2}");
    puts("interface {value=example2}{display=Example interface 2 for extcap}");
  }

  if (interface_flag && dlts_flag) {
    printf("dlt {number=1}{name=DLT_EN10MB}{display=%s}\n", interface);
  }

  if (interface_flag && config_flag) {
    /*
    puts("arg {number=0}{call=--delay}{display=Time delay}{tooltip=Time delay between packages}{type=integer}{range=1,15}{required=true}");
    puts("arg {number=1}{call=--message}{display=Message}{tooltip=Package message content}{placeholder=Please enter a message here ...}{type=string}");
    puts("arg {number=2}{call=--verify}{display=Verify}{tooltip=Verify package content}{type=boolflag}");
    puts("arg {number=3}{call=--remote}{display=Remote Channel}{tooltip=Remote Channel Selector}{type=selector}");
    puts("arg {number=4}{call=--server}{display=IP address for log server}{type=string}{validation=\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b}");
    puts("value {arg=3}{value=if1}{display=Remote1}{default=true}");
    puts("value {arg=3}{value=if2}{display=Remote2}{default=false}");
    */
  }

  if (interface_flag && capture_flag && fifo_flag) {
  }

  /* Print any remaining command line arguments (not options). */
  if (optind < argc) {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
  }
  exit (0);
}
