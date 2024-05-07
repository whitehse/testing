#include <ev.h>
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

void parse_key(struct ev_loop *loop, struct ev_io *w, int revents){
  int c = getch();
  printw("%d", c);
}

int main(int argc, char *argv[]) {

  struct ev_loop *loop = EV_DEFAULT;

  setlocale(LC_ALL, "");
  printf("\033[?1003h\n");
 
  initscr();
  keypad(stdscr, TRUE);
  noecho();
  //raw();
 
  printw("Hello World!");
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
