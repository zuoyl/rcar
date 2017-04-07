#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "json/json.h"
#include <stdexcept>

struct evhttp_request;

namespace rcar {
    class RequestException : public std::runtime_error {
        public:
            explicit RequestException(const std::string &except):
                std::runtime_error(except) {}
    };
 
    class Request {
        public:
            enum { HTTPMethodGet, HTTPMethodPost, HTTPMethodDelete, 
                   HTTPMethodPut, HTTPMethodPatch, HTTPMethodUnknown};
        public:
            Request(struct evhttp_request *req);
            virtual ~Request();
            int get_method();
            int get_req_id();
            int get_role();
            char* get_env(const std::string &name);
            char* get_env(const char* name);
            const std::string getResourceName();
            const Json::Value& getInObject();
            Json::Value& getOutObject() { return m_out_json; }
            void finish(int ret);
        private:
            int get_content_length();
        private:
            struct evhttp_request *m_req;
            int m_content_size;
            Json::Value m_out_json;
            Json::Value m_in_json;
    };

}; // namespace 
#endif
