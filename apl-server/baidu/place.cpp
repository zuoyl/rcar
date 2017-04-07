#include "place.h"
#include "lib/log.h"
#include "error.h"
#include <iostream>

using namespace rcar;
using namespace baidu;

static const std::string SERVICE_POI_SEARCH = 
    "http://api.map.baidu.com/place/v2/search";
static const std::string SERVICE_POI_DETAIL = 
    "http://api.map.baidu.com/place/v2/detail";
static const std::string SERVICE_POI_EVENTSEARCH = 
    "http://api.map.baidu.com/place/v2/eventsearch";
static const std::string SERVICE_POI_EVENTDETAIL = 
    "http://api.map.baidu.com/place/v2/eventdetail";
static const std::string SERVICE_AK = 
    "XMO5it3FZR1yzS0InCvKCGcd";
static const std::string SERVICE_OUTPUT_FORMAT = "json";
static const std::string SERVICE_SCOPE = "2"; // detail
static const std::string SERVICE_PAGE_SIZE = "10";

//using namespace boost::network;
 
int Place::search(const std::string &region,
        const std::string &name, 
        const std::string &tag, 
        const std::string &filter,
        const std::string &sort_name,
        Json::Value &val)
{
#if 0
    try {
        http::client client;
        uri::uri base_uri(SERVICE_POI_SEARCH);
        uri::uri search;
        search << base_uri;
        search << uri::query("region", uri::encoded(region));
        search << uri::query("query", uri::encoded(name));
        search << uri::query("tag", tag);
        search << uri::query("output", SERVICE_OUTPUT_FORMAT);
        search << uri::query("scope", SERVICE_SCOPE);
        search << uri::query("filter", filter);
        search << uri::query("page_size", SERVICE_PAGE_SIZE);
        search << uri::query("ak", SERVICE_AK);

        http::client::request request(search);
        http::client::response response = client.get(request);

        Json::Reader reader;
        if (!reader.parse(response.body(), val)) {
            Log::error("can not parse response from baidu\n");
            return APIE_SERVER_INTERNAL;
        }
        return APIE_OK;
    }
    catch (std::exception &e) {
        Log::error("excetion occured when calling baidu place api:%s\n", 
                e.what());
        return APIE_SERVER_INTERNAL;
    }
#endif
    return APIE_OK;
}
int Place::search_in_bound(
        const std::string &bound,
        const std::string &name, 
        const std::string &tag, 
        const std::string &filter,
        const std::string &sort_name,
        Json::Value &val)
{
#if 0
    try {
        http::client client;
        uri::uri base_uri(SERVICE_POI_SEARCH);
        uri::uri search;
        search << base_uri;
        search << uri::query("bounds", bound);
        search << uri::query("query", uri::encoded(name));
        search << uri::query("tag", tag);
        search << uri::query("output", SERVICE_OUTPUT_FORMAT);
        search << uri::query("scope", SERVICE_SCOPE);
        search << uri::query("filter", filter);
        search << uri::query("page_size", SERVICE_PAGE_SIZE);
        search << uri::query("ak", SERVICE_AK);

        http::client::request request(search);
        http::client::response response = client.get(request);

        Json::Reader reader;
        if (!reader.parse(response.body(), val)) {
            Log::error("can not parse response from baidu\n");
            return APIE_SERVER_INTERNAL;
        }
        return APIE_OK;
    }
    catch (std::exception &e) {
        Log::error("excetion occured when calling baidu place api:%s\n", 
                e.what());
        return APIE_SERVER_INTERNAL;
    }
#endif 
    return APIE_OK;
}
int Place::search_nearby(
        const std::string &location,
        const std::string &radius,
        const std::string &name, 
        const std::string &tag, 
        const std::string &filter,
        const std::string &sort_name,
        Json::Value &val)
{
#if 0
    try {
        http::client client;
        uri::uri base_uri(SERVICE_POI_SEARCH);
        uri::uri search;
        search << base_uri;
        search << uri::query("location", location);
        search << uri::query("radius", radius);
        search << uri::query("query", uri::encoded(name));
        search << uri::query("tag", tag);
        search << uri::query("output", SERVICE_OUTPUT_FORMAT);
        search << uri::query("scope", SERVICE_SCOPE);
        search << uri::query("filter", filter);
        search << uri::query("page_size", SERVICE_PAGE_SIZE);
        search << uri::query("ak", SERVICE_AK);

        http::client::request request(search);
        http::client::response response = client.get(request);

        Json::Reader reader;
        if (!reader.parse(response.body(), val)) {
            Log::error("can not parse response from baidu\n");
            return APIE_SERVER_INTERNAL;
        }
        return APIE_OK;
    }
    catch (std::exception &e) {
        Log::error("excetion occured when calling baidu place api:%s\n", 
                e.what());
        return APIE_SERVER_INTERNAL;
    }
#endif
    return APIE_OK;
}
