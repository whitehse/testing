#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <ev.h>

static int interrupted;

static const struct lws_http_mount mount = {
        .mountpoint			= "/",		    /* mountpoint URL */
        .origin				= "./mount-origin", /* serve from dir */
        .def				= "index.html",	  /* default filename */
        .origin_protocol		= LWSMPRO_FILE,	    /* files in a dir */
        .mountpoint_len			= 1,		        /* char count */
};

void sigint_handler(int sig)
{
        interrupted = 1;
}

int main(int argc, const char **argv)
{
        struct ev_loop *loop = EV_DEFAULT;
        struct lws_client_connect_info connect_info;
        void *foreign_loops[1] = { loop };
        int BUF_SIZE = 4096;

        struct lws_context_creation_info info;
        struct lws_context *context;
        const char *p;
        int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
                        /* for LLL_ verbosity above NOTICE to be built into lws,
                         * lws must have been configured and built with
                         * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
                        /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
                        /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
                        /* | LLL_DEBUG */;

        signal(SIGINT, sigint_handler);

        if ((p = lws_cmdline_option(argc, argv, "-d")))
                logs = atoi(p);

        lws_set_log_level(logs, NULL);
        lwsl_user("LWS minimal http server | visit http://localhost:7681\n");

        memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
        info.port = 7681;
        info.mounts = &mount;
        info.error_document_404 = "/404.html";
        info.options =
                LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE | LWS_SERVER_OPTION_LIBEV;
	info.foreign_loops = foreign_loops;
        //info.uid = 1000;
        //info.gid = 1000;

        if (lws_cmdline_option(argc, argv, "--h2-prior-knowledge"))
                info.options |= LWS_SERVER_OPTION_H2_PRIOR_KNOWLEDGE;

        context = lws_create_context(&info);
        if (!context) {
                lwsl_err("lws init failed\n");
                return 1;
        }

	/*
        while (n >= 0 && !interrupted)
                n = lws_service(context, 0);

	*/

	ev_run(loop, 0);
        lws_context_destroy(context);
        return 0;
}
