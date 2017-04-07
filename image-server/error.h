#ifndef _API_ERROR_H_
#define _API_ERROR_H_

namespace rcar {
    // error code between client and server
    enum {
        APIE_OK = 0,
        APIE_CLIENT_INTERNAL,
        APIE_NO_CONNECTION,
        APIE_INVALID_CLIENT_REQ,
        APIE_INVALID_PARAM,
        APIE_SERVER_INTERNAL,
        APIE_USER_ALREADY_EXIST,
        APIE_SELLER_ALREADY_EXIST,
        APIE_NO_SERVICE,
        APIE_USER_NO_EXIST,
        APIE_SELLER_NO_EXIST,
        APIE_COMMODITY_NO_EXIST,
        APIE_MAINTENANCE_NO_EXIST,
        APIE_PWD_ERROR,
        APIE_CAR_NO_EXIST,
        APIE_NO_FAVORITE,
        APIE_NO_SELLER_FIND,        
        APIE_MAX
    };
    const char* get_error_string(int code);

}; // namespace rcar 
#endif
