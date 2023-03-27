/*
 * Written in 2010-2020 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates the most minimal http server you can make with lws that
 * uses a libuv event loop created outside lws.  It shows how lws can
 * participate in someone else's event loop and clean up after itself.
 *
 * You choose the event loop to work with at runtime, by giving the
 * --uv, --event or --ev switch.  Lws has to have been configured to build the
 * selected event lib support.
 *
 * To keep it simple, it serves stuff from the subdirectory 
 * "./mount-origin" of the directory it was started in.
 * You can change that by changing mount.origin below.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include "eve.h"
#include <ev.h>
#include <sodium.h>

static struct lws_context_creation_info info;
static const struct ops *ops = NULL;
struct lws_context *context;
int lifetime = 5, reported;

enum {
	TEST_STATE_CREATE_LWS_CONTEXT,
	TEST_STATE_DESTROY_LWS_CONTEXT,
	TEST_STATE_EXIT
};

static int sequence = TEST_STATE_CREATE_LWS_CONTEXT;

static struct ev_loop *loop_ev;
static struct ev_timer timer_outer_ev;
static struct ev_signal sighandler_ev;

static void
timer_cb_ev(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
	foreign_timer_service(loop_ev);
}

static void
signal_cb_ev(struct ev_loop *loop, struct ev_signal *watcher, int revents)
{
	signal_cb(watcher->signum);
}

static int BUFFER_SIZE = 1024;
int total_clients = 0;

/* Read client message */
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    char buffer[BUFFER_SIZE];
    ssize_t read;

    if(EV_ERROR & revents) {
        perror("got invalid event");
        return;
    }

    // Receive message from client socket
    read = recv(watcher->fd, buffer, BUFFER_SIZE, 0);

    if(read < 0) {
        perror("read error");
        return;
    }

    if(read == 0) {
        // Stop and free watchet if client socket is closing
        ev_io_stop(loop,watcher);
        free(watcher);
        perror("peer might closing");
        total_clients --; // Decrement total_clients count
        printf("%d client(s) connected.\n", total_clients);
        return;
    } else {
        printf("message:%s\n",buffer);
    }

    // Send message bach to the client
    send(watcher->fd, buffer, read, 0);
    bzero(buffer, read);
}

/* Accept client requests */
void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sd;
    struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

    if(EV_ERROR & revents) {
        perror("got invalid event");
        return;
    }

    // Accept client request
    client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_sd < 0) {
        perror("accept error");
        return;
    }

    total_clients ++; // Increment total_clients count
    printf("Successfully connected with client.\n");
    printf("%d client(s) connected.\n", total_clients);

    // Initialize and start watcher to read client requests
    ev_io_init(w_client, read_cb, client_sd, EV_READ);
    ev_io_start(loop, w_client);
}

static void
foreign_event_loop_init_and_run_libev(void)
{
	//loop_ev = EV_DEFAULT;

	ev_signal_init(&sighandler_ev, signal_cb_ev, SIGINT);
	ev_signal_start(loop_ev, &sighandler_ev);

	ev_timer_init(&timer_outer_ev, timer_cb_ev, 0, 1);
	ev_timer_start(loop_ev, &timer_outer_ev);

	ev_run(loop_ev, 0);
}

static void
foreign_event_loop_stop_libev(void)
{
	ev_break(loop_ev, EVBREAK_ALL);
}

static void
foreign_event_loop_cleanup_libev(void)
{
	/* cleanup the foreign loop assets */

	ev_timer_stop(loop_ev, &timer_outer_ev);
	ev_signal_stop(loop_ev, &sighandler_ev);

	ev_run(loop_ev, 0);
	ev_loop_destroy(loop_ev);
}

const struct ops ops_libev = {
	foreign_event_loop_init_and_run_libev,
	foreign_event_loop_stop_libev,
	foreign_event_loop_cleanup_libev
};

static const struct lws_http_mount mount = {
	/* .mount_next */		NULL,		/* linked-list "next" */
	/* .mountpoint */		"/",		/* mountpoint URL */
	/* .origin */			"./mount-origin", /* serve from dir */
	/* .def */			"index.html",	/* default filename */
	/* .protocol */			NULL,
	/* .cgienv */			NULL,
	/* .extra_mimetypes */		NULL,
	/* .interpret */		NULL,
	/* .cgi_timeout */		0,
	/* .cache_max_age */		0,
	/* .auth_mask */		0,
	/* .cache_reusable */		0,
	/* .cache_revalidate */		0,
	/* .cache_intermediaries */	0,
	/* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
	/* .mountpoint_len */		1,		/* char count */
	/* .basic_auth_login_file */	NULL,
};

void
signal_cb(int signum)
{
	lwsl_notice("Signal %d caught, exiting...\n", signum);

	switch (signum) {
	case SIGTERM:
	case SIGINT:
		break;
	default:
		break;
	}

	lws_context_destroy(context);
}

static int
callback_http(struct lws *wsi, enum lws_callback_reasons reason,
	      void *user, void *in, size_t len)
{
	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
		lwsl_user("LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP: resp %u\n",
				lws_http_client_http_response(wsi));
		break;

	/* because we are protocols[0] ... */
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n",
			 in ? (char *)in : "(null)");
		break;

	/* chunks of chunked content, with header removed */
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
		lwsl_user("RECEIVE_CLIENT_HTTP_READ: read %d\n", (int)len);
		lwsl_hexdump_info(in, len);
		return 0; /* don't passthru */

	/* uninterpreted http content */
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
		{
			char buffer[1024 + LWS_PRE];
			char *px = buffer + LWS_PRE;
			int lenx = sizeof(buffer) - LWS_PRE;

			if (lws_http_client_read(wsi, &px, &lenx) < 0)
				return -1;
		}
		return 0; /* don't passthru */

	case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
		lwsl_user("LWS_CALLBACK_COMPLETED_CLIENT_HTTP %s\n",
			  lws_wsi_tag(wsi));
		break;

	case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
		lwsl_info("%s: closed: %s\n", __func__, lws_wsi_tag(wsi));
		break;

	default:
		break;
	}

	return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
	{ "httptest", callback_http, 0, 0, 0, NULL, 0},
	LWS_PROTOCOL_LIST_TERM
};

static int
do_client_conn(void)
{
	struct lws_client_connect_info i;

	memset(&i, 0, sizeof i); /* otherwise uninitialized garbage */

	i.context		= context;

	i.ssl_connection	= LCCSCF_USE_SSL;
	i.port			= 443;
	i.address		= "warmcat.com";

	i.ssl_connection	|= LCCSCF_H2_QUIRK_OVERFLOWS_TXCR |
				   LCCSCF_H2_QUIRK_NGHTTP2_END_STREAM;
	i.path			= "/";
	i.host			= i.address;
	i.origin		= i.address;
	i.method		= "GET";
	i.local_protocol_name	= protocols[0].name;
#if defined(LWS_WITH_SYS_FAULT_INJECTION)
	i.fi_wsi_name		= "user";
#endif

	if (!lws_client_connect_via_info(&i)) {
		lwsl_err("Client creation failed\n");

		return 1;
	}

	return 0;
}


/* this is called at 1Hz using a foreign loop timer */

void
foreign_timer_service(void *foreign_loop)
{
	void *foreign_loops[1];

	//lwsl_user("Foreign 1Hz timer\n");

	if (sequence == TEST_STATE_EXIT && !context && !reported) {
		/*
		 * at this point the lws_context_destroy() we did earlier
		 * has completed and the entire context is wholly destroyed
		 */
		lwsl_user("lws_destroy_context() done, continuing for 5s\n");
		reported = 1;
	}

	if (--lifetime)
		return;

	switch (sequence++) {
	case TEST_STATE_CREATE_LWS_CONTEXT:
		/* this only has to exist for the duration of create context */
		foreign_loops[0] = foreign_loop;
		info.foreign_loops = foreign_loops;

		context = lws_create_context(&info);
		if (!context) {
			lwsl_err("lws init failed\n");
			return;
		}
		lwsl_user("LWS Context created and will be active for 10s\n");

		do_client_conn();

		lifetime = 11;
		break;

	case TEST_STATE_DESTROY_LWS_CONTEXT:
		/* cleanup the lws part */
		lwsl_user("Destroying lws context and continuing loop for 5s\n");
		lws_context_destroy(context);
		lifetime = 6;
		break;

	case TEST_STATE_EXIT:
		lwsl_user("Deciding to exit foreign loop too\n");
		ops->stop();
		break;
	default:
		break;
	}
}

static void
mytimer_cb_ev(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
        lwsl_user("mytimer_cb_ev was called");
}

int main(int argc, const char **argv)
{
	const char *p;
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	if ((p = lws_cmdline_option(argc, argv, "-d")))
		logs = atoi(p);

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal http server eventlib + foreign loop |"
		  " visit http://localhost:7681\n");

	/*
	 * We prepare the info here, but don't use it until later in the
	 * timer callback, to demonstrate the independence of the foreign loop
	 * and lws.
	 */

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = 7681;
	if ((p = lws_cmdline_option(argc, argv, "-p")))
		info.port = atoi(p);
	info.mounts = &mount;
	info.error_document_404 = "/404.html";
	info.pcontext = &context;
	info.protocols = protocols;
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT |
		LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	if (lws_cmdline_option(argc, argv, "-s")) {
		info.ssl_cert_filepath = "localhost-100y.cert";
		info.ssl_private_key_filepath = "localhost-100y.key";
	}

	/*
	 * We configure lws to use the chosen event loop, and select the
	 * matching event-lib specific code for our demo operations
	 */

	info.options |= LWS_SERVER_OPTION_LIBEV;
	ops = &ops_libev;
	lwsl_notice("%s: using libev loop\n", __func__);

        static struct ev_timer mytimer_ev;

        loop_ev = EV_DEFAULT;

        ev_timer_init (&mytimer_ev, mytimer_cb_ev, 1.5, .7);
        ev_timer_start (loop_ev, &mytimer_ev);

        int PORT_NO = 2000;

        int sd;
        struct sockaddr_in addr;
        int addr_len = sizeof(addr);
        struct ev_io w_accept;

        // Create server socket
        if( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
            perror("socket error");
	    exit(EXIT_FAILURE);
        }

        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT_NO);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
            perror("bind error");
	    exit(EXIT_FAILURE);
        }

        if (listen(sd, 2) < 0) {
            perror("listen error");
	    exit(EXIT_FAILURE);
        }

        // Initialize and start a watcher to accepts client requests
        ev_io_init(&w_accept, accept_cb, sd, EV_READ);
        ev_io_start(loop_ev, &w_accept);

        if (sodium_init() < 0) {
            puts("The sodium library did not initialize. Bailing");
	    abort();
        }

	/* foreign loop specific startup and run */
	ops->init_and_run();

	lws_context_destroy(context);

	/* foreign loop specific cleanup and exit */

	ops->cleanup();

	lwsl_user("%s: exiting...\n", __func__);

	return 0;
}

