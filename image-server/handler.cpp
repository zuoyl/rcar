//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include "handler.h"

using namespace rcar;
using namespace mongo;

bool rcar::initialize_image_handle() {
    // initialize mongodb
//    mongo::client::Options options;
//    mongo::client::initialize(options);
    return true;
}

// utils

void rcar::dump_http_request(evhttp_request *req) {
    std::string cmdtype;
	struct evkeyvalq headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET: cmdtype = "GET"; break;
        case EVHTTP_REQ_POST: cmdtype = "POST"; break;
        case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
        case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
        case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
        case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
        case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
        case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
        default: cmdtype = "unknown"; break;
	}
    // decode uri
    const char *uri = evhttp_request_get_uri(req);
    char *decoded_uri = evhttp_decode_uri(uri);

    std::cout << "Received a " << cmdtype << " request for " << uri << std::endl;

    struct evkeyvalq params;
    evhttp_parse_query(uri, &params);

    const char* target = evhttp_find_header(&params, "target");
    const char* size = evhttp_find_header(&params, "size");

	buf = evhttp_request_get_input_buffer(req);
    std::cout << "Input data: <<<";
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
		if (n > 0)
			(void) fwrite(cbuf, 1, n, stdout);
	}
    std::cout << ">>>";
}

static std::string get_val(evhttp_request *req, const std::string &key) {
    const char *uri = evhttp_request_get_uri(req);
    char *decoded_uri = evhttp_decode_uri(uri);

    struct evkeyvalq params;
    evhttp_parse_query(uri, &params);

    std::string value;
    const char* v = evhttp_find_header(&params, key.c_str());
    if (v != NULL) 
        value = v;
    return value;
}

std::string rcar::get_image_id(evhttp_request *req) {
    const char *uri = evhttp_request_get_uri(req);
    std::string decoded_uri = evhttp_decode_uri(uri);
    // uri has the following format
    // /rcar/images/100.jpeg?size=24x24&thumbnail=yes&target=seller
    std::size_t pos1 = decoded_uri.find_last_of("/");
    std::size_t pos2 = decoded_uri.find("?"); 
    std::string name = decoded_uri.substr(pos1 + 1, pos2 - pos1 -1);
    return name;
}

std::string rcar::get_image_target(evhttp_request *req) {
    return get_val(req, "target");
}
std::string rcar::get_image_size(evhttp_request *req) {
    return get_val(req, "size");
}

std::string rcar::get_image_thumbnail(evhttp_request *req) {
    return get_val(req, "thumbnail");
}
Json::Value rcar::create_in_json_object(evhttp_request *req) {
    Json::Value val;
    // const json object
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    int size = evbuffer_get_length(buf);
    char *bufptr = new char[size];
    int n = evbuffer_remove(buf, bufptr, size);
    if (n != size) {
        Log::error("failed to remove data from evrequest\n");
        return val;
    }
    Json::Reader reader;
    if (!reader.parse(bufptr, bufptr + n, val)) {
        Log::error("failed to parse request content as json object\n");
        return val;
    }
    return val;
}

Json::Value rcar::create_out_json_object(evhttp_request *req) {
    Json::Value val;
    // const json object
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    int size = evbuffer_get_length(buf);
    char *bufptr = new char[size];
    int n = evbuffer_remove(buf, bufptr, size);
    if (n != size) {
        Log::error("failed to remove data from evrequest\n");
        return val;
    }
    Json::Reader reader;
    if (!reader.parse(bufptr, bufptr + n, val)) {
        Log::error("failed to parse request content as json object\n");
        return val;
    }
 
    return val;
}




