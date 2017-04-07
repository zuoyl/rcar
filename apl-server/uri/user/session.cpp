//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Session, "user/session");

int Session::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Session::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string pwd = content["pwd"].asString();
    std::string push_user_id = content["push_user_id"].asString(); 
    std::string push_channel_id = content["push_channel_id"].asString();
    std::string device_type = content["device_type"].asString();
    std::string time = get_customstring_time();

    if (role.empty() || user_id.empty() || pwd.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s, pwd = %s", user_id.c_str(), pwd.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str() );
            return APIE_USER_NO_EXIST;
        }

        //check password
        BSONObj user = cursor->next();
        if (pwd != user.getStringField("pwd")) {
            return APIE_PWD_ERROR;
        }

        // update record in session db
        cursor = c.query(DB_SESSION, MONGO_QUERY("role" << "user" <<"identifier" << user_id));
        if (!cursor->more()) {
            BSONObj p = BSON("role" << role
                    << "identifier" << user_id
                    << "role" << "user" 
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

            c.update(DB_SESSION, MONGO_QUERY("role" << "user" << "identifier" << user_id), p, true, false);
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
}

int Session::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Session::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
	
    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__,"user_id = %s", user_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("role" << role << "user_id" << user_id));

        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        // update user login status in session DB 
        BSONObj p = BSON("$set" << BSON("status" << "unlogin"));
        c.update(DB_SESSION, MONGO_QUERY("role" << role << "user_id" << user_id), p, false, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

int Session::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
