#ifndef _BAIDU_ACCESS_PLACE_API_H_
#define _BAIDU_ACCESS_PLACE_API_H_

#include "json/json.h"
#include <string>

namespace rcar { namespace baidu {
    class Place {
       public:
            Place(){}
            ~Place(){}
            int search(
                    const std::string &region,
                    const std::string &name, 
                    const std::string &tag, 
                    const std::string &filer,
                    const std::string &sort_name,
                    Json::Value &val);
            int search_in_bound(
                    const std::string &bound,
                    const std::string &name, 
                    const std::string &tag, 
                    const std::string &filer,
                    const std::string &sort_name,
                    Json::Value &val);
            int search_nearby(
                    const std::string &location,
                    const std::string &radius,
                    const std::string &name, 
                    const std::string &tag, 
                    const std::string &filer,
                    const std::string &sort_name,
                    Json::Value &val);
    };

}// namespace baidu
}// namespace rcar
#endif
