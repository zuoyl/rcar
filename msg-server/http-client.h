#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include "json/json.h"

namespace rcar { 
    class HttpClient {
        public:
            HttpClient(){} 
            virtual ~HttpClient(){}
            int post(const std::string &url, const std::string &input);
            int get(const std::string &url, const std::string &input);
    };
}// namespace rcar
#endif //_HTTP_CLIENT_H_
