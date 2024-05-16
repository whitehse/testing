#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/eventfd.h>
#include <liburing.h>
#include <fcntl.h>

#include <ev.h>
#include <libpq-fe.h>
//#include <eve.h>

#define BUFFER_SIZE 1024

void pq_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  ssize_t read;

  int r = PQconsumeInput(PGconn *conn);
/*
  read = recv(w->fd, buffer, BUFFER_SIZE, 0);

  if ( read < 0 ) {
    perror("read error");
    exit(5);
  }

  if (read == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      //return 0;
    } else {
      perror("Something bad happened");
    }
  }
*/
}

int main(int argc, char *argv[]) {
  PGconn* conn;
  int pq_fd;
  struct ev_loop *loop = ev_default_loop(0);
  char *conninfo = malloc(128*sizeof(char));
  sprintf(conninfo, "postgresql://dwhite:%s@qgis.ecolink.coop:5432/qgis", argv[1]);
  printf("The connection string is %s\n", conninfo);
  conn = PQconnectdb(conninfo); 

  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
  }

  int r = PQsendQuery(conn, "select count(*) from cis.bi_srv_loc;");
  printf("PQsendQuery = %d\n", r);

  pq_fd = PQsocket(conn);
  if (pq_fd < 1) {
    perror("There is something wrong with the postgres client file descriptor");
  }

/*
*/
  struct ev_io *w_pq = (struct ev_io*) malloc (sizeof(struct ev_io));
  w_pq->data = conn;
  ev_io_init(w_pq, pq_read_cb, pq_fd, EV_READ | EV_WRITE);
  ev_io_start(loop, w_pq);

  ev_run (loop, 0);
}
