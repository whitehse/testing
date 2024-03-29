/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013-2020 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

#define ASSH_PV
#define ASSH_ABI_UNSAFE  /* do not warn */

#include "test.h"

#include <assh/assh_session.h>
#include <assh/assh_context.h>
#include <assh/assh_algo.h>
#include <assh/assh_kex.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

enum action_e {
  ACTION_NO_FILTER = 2,
  ACTION_ORDER = 4,
  ACTION_TABLE = 8,
};

static enum action_e action = 0;
static uint_fast8_t safety_weight = 50;
static assh_safety_t min_safety = 0;

static char std(enum assh_algo_spec_e s)
{
  switch (s & 7)
    {
    case ASSH_ALGO_STD_IETF:
      return 'I';
    case ASSH_ALGO_STD_DRAFT:
      return 'D';
    case ASSH_ALGO_STD_PRIVATE:
      return 'P';
    default:
      return ' ';
    }
}

static const char *class_names[] = ASSH_ALGO_CLASS_NAMES;

static void show_table()
{
  printf("  Class      Name                                 Implem    Std Speed Safety\n"
	          "----------------------------------------------------------------------------");

  assh_algo_id_t i;
  const struct assh_algo_s *a;
  enum assh_algo_class_e cl = ASSH_ALGO_ANY;

  for (i = 0; (a = assh_algo_table[i]) != NULL; i++)
    {
      const struct assh_algo_name_s *n = a->names;

      if (cl != a->class_)
	{
	  putchar('\n');
	  cl = a->class_;
	}

      printf("  %-10s %s%-36s%s %-12s %c  %3u   %3u\n",
	      class_names[a->class_], "\x1b[1m",
	      n->name, "\x1b[m", a->implem, std(n->spec), a->speed, a->safety);

      if (a->variant != NULL)
	printf("    Variant:   %-40s\n", a->variant);

      for (n++; n->spec; n++)
	printf("    Alias :  %-40s %c\n", n->name, std(n->spec));

      if (!assh_algo_supported(a))
	printf("    Not supported on this platform\n");
    }
}

static void show_order()
{
  struct assh_context_s *c;

  if (assh_context_create(&c, ASSH_SERVER,
                          NULL, NULL, NULL, NULL))
    TEST_FAIL("context init");

  if (assh_kex_set_order(c, safety_weight) ||
      assh_algo_register_default(c, min_safety) != ASSH_OK)
    TEST_FAIL("algo regiter");

  if (!(action & ACTION_NO_FILTER))
    assh_algo_filter_variants(c);

  assh_algo_sort(c);

  printf("  Spd Saf Score Algorithm                                Implem        Variant\n"
	          "-------------------------------------------------------------------------------\n");

  assh_algo_id_t i;
  const struct assh_algo_s *a;
  enum assh_algo_class_e cl = ASSH_ALGO_ANY;

  for (i = 0; i < c->algo_cnt; i++)
    {
      a = c->algos[i];
      const struct assh_algo_name_s *n = a->names;

      if (cl != a->class_)
	{
	  cl = a->class_;
	  printf("%s:\n", class_names[cl]);
	}

      printf("  %3u %3u %5u %-40s %-13s %s\n",
              a->speed, a->safety, ASSH_ALGO_SCORE(a, safety_weight),
              n->name, a->implem, a->variant ? a->variant : "");
    }

  assh_context_release(c);
}

static void usage()
{
  printf("usage: algo_list [options]\n");

  printf(	  "Options:\n\n"

	  "    -t         show table of supported algorithms\n\n"

	  "    -w 0-99    set safety weight (50), filter and reorder algorithms\n"
	  "    -a 0-99    set minimum safety (0)\n"
	  "    -V         do not remove variant duplicates\n\n"
	  "    -h         show help\n\n"
	  );

  exit(1);
}

int main(int argc, char **argv)
{
  if (assh_deps_init())
    return -1;

  int opt;

  while ((opt = getopt(argc, argv, "w:a:Vth")) != -1)
    {
      switch (opt)
	{
	case 't':
	  if (action & ACTION_ORDER)
	    usage();
	  action = ACTION_TABLE;
	  break;
	case 'w':
	  if (action & ACTION_TABLE)
	    usage();
	  safety_weight = atoi(optarg);
	  action |= ACTION_ORDER;
	  break;
	case 'a':
	  if (action & ACTION_TABLE)
	    usage();
	  min_safety = atoi(optarg);
	  action |= ACTION_ORDER;
	  break;
	case 'V':
	  if (action & ACTION_TABLE)
	    usage();
	  action |= ACTION_NO_FILTER | ACTION_ORDER;
	  break;
	case 'h':
	  usage();
	default:
	  return 1;
	}
    }

  if (action & ACTION_ORDER)
    show_order();
  else
    show_table();

  return 0;
}
