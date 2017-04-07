// common db operation
#ifndef _DB_H_
#define _DB_H_

#include "json/json.h"
#include "lib/log.h"
#include "request.h"
#include "stream.h"
#include "error.h"
#include "mongo/client/dbclient.h"
#include "mongo/bson/bsonobj.h"
#include "utils.h"
#include <string>

using namespace mongo;

namespace rcar {
    //it's a table name of store login user/seller info in database 
    static const std::string DB_SESSION = "rcar-base.session";
 
    //it's a table name of store user information in database
    static const std::string DB_USR = "rcar-user.user";
    //it's a table name of store user manager in database 
    static const std::string DB_USRMNG = "rcar-user.usermng";
 
    static const std::string DB_CAR = "rcar-carinfo.cars";
    //it's a table name of store seller information in database
    static const std::string DB_SELLER = "rcar-seller.seller";

    static const std::string DB_ACTIVITY = "rcar-seller.activity";

    //it's a table name of store user message information in database
    static const std::string DB_MSG = "rcar-msg.msg";

    //it's a table name of store commodity information in database
    static const std::string DB_STORE = "rcar-commodity.commodities";

    //it's a table name of store seller'ads info in database 
    static const std::string DB_ADS = "rcar-advertisement.ads";

     //it's a table name of store accusation info in database 
    static const std::string DB_ACCUSATION = "rcar-base.accusation";

    //it's a table name of store service info in database 
    static const std::string DB_ORDER = "rcar-order.order";


    //it's a table name of store user sos info in database 
    static const std::string DB_SOS = "rcar-sos.sos";
    //it's a table name of store coupon info in database 
    static const std::string DB_COUPON = "rcar.coupon";

    //it's a table name of store comment info in database 
    static const std::string DB_COMMENT = "rcar-base.comment";
    static const std::string DB_CITY = "rcar-base.cities";


    bool mongodb_initialize();
    // convert helpers
    mongo::BSONArray getImageNames(const Json::Value &number);
    mongo::BSONArray getImageNames(int number); 

    // converer helpers
    Json::Value getImageNamesFromBson(const mongo::BSONObj &names);
    Json::Value getImageNamesFromBson(const mongo::BSONArray &names);
    Json::Value getImageNamesFromBson(const mongo::BSONObj &root, const std::string &key); 
    mongo::BSONArray getImagesNameFromJson(const Json::Value &names);
    Json::Value getImageNamesFromBson(const mongo::BSONArray &root,  const std::string &key);

    // remove images
    void removeImages(const mongo::BSONObj &root, const std::string &key);

    Json::Value tojson(const mongo::BSONObj &root, const char *key = nullptr); 
    mongo::BSONObj fromjson(const Json::Value &root, const char *key = nullptr); 

}; // namespace rcar 
#endif 
