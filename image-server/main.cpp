#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>


#include <assert.h>
#include "lib/sys-config.h"
#include "lib/log.h"
#include "error.h"

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

#include "handler.h"
#include "context.h"
#include "mongo/client/init.h"

char uri_root[512];

using namespace rcar;


static ImageContext _context; 

static bool initialize_log() {
    SystemConfig *config = rcar::SystemConfig::getInstance();
    std::string log_file = config->getLogFile() ;
    int log_level = !config->getLogLevel();
    return Log::initialize(log_file, log_level);
}

static void httpImagesCallback(struct evhttp_request *req, void *arg) {
    // TransactionManager is not thread safe, it will be improved later
    int method = evhttp_request_get_command(req);
   // rcar::dump_http_request(req);
    if (method == EVHTTP_REQ_GET)
        rcar::handleImageDownload(req, _context);
    else if (method == EVHTTP_REQ_POST)
        rcar::handleImageUpload(req, _context);

}

int main(int argc, char **argv) {
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;
	int port;
    rcar::SystemConfig *config;

    mongo::client::Options options;
    mongo::client::initialize(options);

    if (!rcar::initialize_image_handle()) {
        std::cout << "failed to initialize image handler" << std::endl;
        goto failure;
    }


    // initialize signal
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		goto failure;


    // get general configuration
    config = rcar::SystemConfig::getInstance();
    if (!config->create(argc, argv, "rcar-image-server")) {
        std::cout << "failed to create configuration" << std::endl;
        goto failure;
    }
    

	port  = config->getPort();
    _context.setMongo(config->getDBServer());

	// initialize logs
	if (!initialize_log()) {
		std::cout << "failed to initialize log" << std::endl;
		// SystemConfig::destroy_instance();
		goto failure;
	}
	base = event_base_new();
	if (!base) {
        std::cout << "Couldn't create an event_base: exiting" << std::endl;
		goto failure;
	}

	/* Create a new evhttp object to handle requests. */
    http = evhttp_new(base);
	if (!http) {
        std::cout << "couldn't create evhttp. Exiting" << std::endl;
		goto failure;
	}

    /* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "127.0.0.1", port);
	if (!handle) {
        std::cout <<  "couldn't bind to port " << port << " Exiting" << std::endl;
		goto failure;
	}

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			goto failure;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
            std::cout << "Weird address family " << ss.ss_family << std::endl;
			goto failure;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
            std::cout << "Listening on " <<  addr << ":" <<  got_port << std::endl;
			evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d",addr,got_port);
		} else {
            std::cout << "evutil_inet_ntop failed" << std::endl;
			goto failure;
		}
	}


    evhttp_set_max_body_size(http, 1024*1024*8);
	evhttp_set_gencb(http, httpImagesCallback, NULL);

//    daemon(0, 0);

	event_base_dispatch(base);
    Log::hl_info("exit server...\n");

failure:
    if (http)
        evhttp_free(http);
    if (base) 
        event_base_free(base);
    Log::finalize();
    mongo::client::shutdown();



	return 0;
}
