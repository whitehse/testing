//#include <ev.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <wctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <locale.h>
#include <fcntl.h>

#define EV_STANDALONE 1
#include "ev.c"

//#include <float.h>
//
///* a floor() replacement function, should be independent of ev_tstamp type */
//static ev_tstamp noinline ev_floor (ev_tstamp v) {
//  /* the choice of shift factor is not terribly important */
//#if FLT_RADIX != 2 /* assume FLT_RADIX == 10 */
//  const ev_tstamp shift = sizeof (unsigned long) >= 8 ? 10000000000000000000. : 1000000000.;
//#else
//  const ev_tstamp shift = sizeof (unsigned long) >= 8 ? 18446744073709551616. : 4294967296.;
//#endif
//
//  /* argument too large for an unsigned long? */
//  if (expect_false (v >= shift))
//    {
//      ev_tstamp f;
//
//      if (v == v - 1.)
//        return v; /* very large number */
//
//      f = shift * ev_floor (v * (1. / shift));
//      return f + ev_floor (v - f);
//    }
//
//  /* special treatment for negative args? */
//  if (expect_false (v < 0.))
//    {
//      ev_tstamp f = -ev_floor (-v);
//
//      return f - (f == v ? 0 : 1);
//    }
//
//  /* fits into an unsigned long */
//  return (unsigned long)v;
//}

WINDOW *create_window(int y, int x, int width, int height, int color_id){
    WINDOW* win;
    int i;

    win = newwin(height, width, y, x);
    wbkgd(win, COLOR_PAIR(color_id));
    box(win, 0, 0);
    wrefresh(win);
    //wbkgd(win, A_NORMAL | COLOR_PAIR(color_id) | ' ');
    //wbkgd(win, COLOR_PAIR(color_id) | ' ');
    //bkgd(win, COLOR_PAIR(color_id));

    /* start of shadow */
    attron(COLOR_PAIR(3));

    for(i = (x + 2); i < (x + width + 1); i++){
        move((y + height), i);
        addch(' ');
    }

    for(i = (y + 1); i < (y + height + 1); i++){
        move(i, (x + width));
        addch(' ');
        move(i, (x + width + 1));
        addch(' ');
    }
    attroff(COLOR_PAIR(3));
    /* end of shadow */

    refresh();

    return win;
}

void parse_key(struct ev_loop *loop, struct ev_io *w, int revents){
  int c;
  c = getch();
  while (c != ERR) {
    printw("%c", c);
    c = getch();
  }
}

int main(int argc, char *argv[]) {

  struct ev_loop *loop = EV_DEFAULT;
  WINDOW *win;

  setlocale(LC_ALL, "");

  /* Uncomment to allow for mouse input */
  //printf("\033[?1003h\n");
 
  win = initscr();
  // If no more characters are available on standard input, return ERR
  nodelay(win, TRUE);   

  // Give NCurses control of input
  keypad(stdscr, TRUE);

  // Don't echo input to screen. It will be intercepted by parse_key 
  noecho();
  //raw();
 
  //printw("Hello World!\n\n");

  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  start_color();
  // Main Window
  init_pair(1, COLOR_WHITE, COLOR_BLACK);

  // Subwindow
  init_pair(2, COLOR_WHITE, COLOR_BLUE);

  // Force background of subwindow
  init_pair(3, COLOR_RED, COLOR_YELLOW);

  bkgd(COLOR_PAIR(1));
  refresh();

  create_window(5, 5, 60, 20, 2);
  refresh();

  int val = fcntl(STDIN_FILENO, F_GETFL, 0);
  if (val != -1) {
    fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);
  }

  ev_io *keyboard_watcher;
  keyboard_watcher = malloc(sizeof(ev_io));
  ev_io_init (keyboard_watcher, parse_key, STDIN_FILENO, EV_READ);
  ev_io_start (loop, keyboard_watcher);

  ev_run (loop, 0);
}
