#include "request.h"
#include "stream.h"
#include "json/json.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "lib/log.h"
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>

using namespace rcar;

Request::Request(struct evhttp_request *req) {
    assert(req != NULL);
    m_req = req;
    // construct json in object
    int method = evhttp_request_get_command(req);
    if (method == EVHTTP_REQ_GET || method == EVHTTP_REQ_HEAD ||
            method == EVHTTP_REQ_DELETE) {
        const char *uri = evhttp_request_get_uri(req);
        std::string decoded_uri = evhttp_decode_uri(uri); 

        struct evkeyvalq params;
        evhttp_parse_query(uri, &params);

        std::string value;
        const char* v = evhttp_find_header(&params, "q");
        if (v != nullptr)  {
            Json::Reader reader;
            if (!reader.parse(v, v + strlen(v), m_in_json)) {
                 throw RequestException("failed to parse request content as json object\n");
            }
        }

    } else {
        struct evbuffer *buf = evhttp_request_get_input_buffer(req);
        int size = evbuffer_get_length(buf);
        char *bufptr = new char[size];
        int n = evbuffer_remove(buf, bufptr, size);
        if (n != size) {
            throw RequestException("failed to remove data from evrequest\n");
        }
        Json::Reader reader;
        if (!reader.parse(bufptr, bufptr + n, m_in_json)) {
            throw RequestException("failed to parse request content as json object\n");
        }
        delete []bufptr;
    }

}

Request::~Request(void) {
    assert(m_req != NULL);
}

int Request::get_method() {
    int method = evhttp_request_get_command(m_req);
    if (method == EVHTTP_REQ_POST) return HTTPMethodPost;
    if (method == EVHTTP_REQ_GET) return HTTPMethodGet;
    if (method == EVHTTP_REQ_DELETE) return HTTPMethodDelete;
    if (method == EVHTTP_REQ_PUT) return HTTPMethodPut;
    
    return HTTPMethodUnknown;
}


char* Request::get_env(const std::string &name) {
    return get_env(name.c_str());
}

char* Request::get_env(const char *name) {
    struct evkeyvalq *headers;
    struct evkeyval *header;

    if (!name) return NULL;

    headers = evhttp_request_get_input_headers(m_req);
    for (header = headers->tqh_first; header != NULL;
         header = header->next.tqe_next) {
        if (strcmp(name, header->key))
            return header->value;
    }
    return NULL;
}

const std::string Request::getResourceName() {
    char *uri = (char*)evhttp_request_get_uri(m_req);
    std::string decoded_uri = evhttp_decode_uri(uri);
    // formate
    // http://127.0.0.1/rcar/api/v1.0/user/xxx
    std::size_t pos1 =  decoded_uri.find("api/v1.0/");
    pos1 += strlen("api/v1.0");
    std::size_t pos2 =  decoded_uri.find("?");
    std::string name = decoded_uri.substr(pos1 + 1, pos2 - pos1 - 1);
    return name;
}
int Request::get_content_length() {
    struct evbuffer *buf = evhttp_request_get_input_buffer(m_req);
    return evbuffer_get_length(buf);
}

void Request::finish(int result) {
    m_out_json["api_result"] = result;
    evbuffer* buf = evbuffer_new();
    std::string val = m_out_json.toStyledString();
    evbuffer_add(buf, val.c_str(), val.size()); 

    // set content type
    evhttp_add_header(evhttp_request_get_output_headers(m_req),
            "Content-Type", "text/json");
    evhttp_send_reply(m_req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

const Json::Value& Request::getInObject() {
   return m_in_json;
}


