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
#include "request.h"
#include "resource-mgr.h"
#include "lib/log.h"
#include "stream.h"
#include "error.h"

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif
#include "mongo/client/dbclient.h"

char uri_root[512];

static bool initialize_log() {
    std::string log_file;
    int log_level;

    rcar::SystemConfig *config = rcar::SystemConfig::getInstance();
    log_file =  config->getLogFile();
    if (!log_file.empty()) 
        log_file = "./rcar.log";
    log_level = config->getLogLevel();
        
    return rcar::Log::initialize(log_file, log_level);
}


int main(int argc, char **argv) {
#if 0 
    // initialize mongodb
    mongo::client::Options option;
    mongo::client::initialize(option);
    mongo::DBClientConnection conn;
    conn.connect("localhost");
#endif
    // initialize signal
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#if 0
	if (argc < 2) {
		syntax();
		return 1;
	}
#endif
    // get general configuration
    rcar::SystemConfig *config = rcar::SystemConfig::getInstance();
    if (!config->create(argc, argv, "rcar-apl-server")) {
        std::cout << "failed to create configuration" << std::endl;
        return 0;
    }
	int port = config->getPort();

	// initialize logs
	if (!initialize_log()) {
		std::cout << "failed to initialize log" << std::endl;
		// SystemConfig::destroy_instance();
		return 0;
	}
	struct event_base *base = event_base_new();
	if (!base) {
        std::cout << "Couldn't create an event_base: exiting" << std::endl;
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	struct evhttp *http = evhttp_new(base);
	if (!http) {
        std::cout << "couldn't create evhttp. Exiting" << std::endl;
		return 1;
	}

    /* Now we tell the evhttp what port to listen on */
	struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, "127.0.0.1", port);
	if (!handle) {
        std::cout <<  "couldn't bind to port " << port << " Exiting" << std::endl;
		return 1;
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
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
            std::cout << "Weird address family " << ss.ss_family << std::endl;
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
            std::cout << "Listening on " <<  addr << ":" <<  got_port << std::endl;
			evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d",addr,got_port);
		} else {
            std::cout << "evutil_inet_ntop failed" << std::endl;
			return 1;
		}
	}

    // initialize transaction manager
    rcar::ResourceManager *mgr = rcar::ResourceManager::getInstance();
    mgr->initialize(http);
    mgr->dumpAllResources();

    // daemon
    //daemon(0, 0);

	event_base_dispatch(base);
    rcar::Log::hl_info("exit server...\n");

    // SystemConfig::destroy_instance();
    // release log resource
    rcar::Log::finalize();



	return 0;
}
