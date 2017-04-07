//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Session, "seller/session");

int Session::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Session::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["id"].asString();
    std::string pwd = content["pwd"].asString();
    std::string push_user_id = content["push_user_id"].asString(); 
    std::string push_channel_id = content["push_channel_id"].asString();
    std::string device_type = content["device_type"].asString();
    std::string time = get_customstring_time();

    if (role.empty() || seller_id.empty() || pwd.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s, pwd = %s", seller_id.c_str(), pwd.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }

        //check password
        BSONObj seller = cursor->next();
        if (pwd != seller.getStringField("pwd")) {
            return APIE_PWD_ERROR;
        }

        // update record in session db
        cursor = c.query(DB_SESSION, MONGO_QUERY("role" << "seller" <<"identifier" << seller_id));
        if (!cursor->more()) {
            BSONObj p = BSON("role" << role
                    << "identifier" << seller_id
                    << "role" << "seller" 
                    << "push_user_id" << push_user_id
                    << "push_channel_id" << push_channel_id
                    << "device_type" << device_type
                    << "status" << "login"
                    << "time" << time
                    << "online" << 1);
            c.insert(DB_SESSION, p);
        } else {
            BSONObj p = BSON("$set" << BSON("push_user_id" << push_user_id
                    << "push_channel_id" << push_channel_id
                    << "device_type" << device_type
                    << "status" << "login"
                    << "time" << time
                    << "online" << 1));
            c.update(DB_SESSION, MONGO_QUERY("role" << "seller" << "identifier" << seller_id), p, true, false);
        }
 
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Session::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Session::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
	
    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"seller_id = %s", seller_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("role" << role << "seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller %s doesn't exist", seller_id.c_str() );
            return APIE_INVALID_CLIENT_REQ;
        }

        // update user login status in session DB 
        BSONObj p = BSON("$set" << BSON("status" << "unlogin"));
        c.update(DB_SESSION, MONGO_QUERY("role" << role << "seller_id" << seller_id), p, false, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Session::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
