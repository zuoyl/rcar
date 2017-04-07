//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Password, "user/pwd");

int Password::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Password::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string pwd = content["pwd"].asString();
   
    if (role.empty() || user_id.empty() || pwd.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s, pwd = %s", user_id.c_str(), pwd.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s no exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        // update password
        BSONObj p = BSON("$set" << BSON("pwd" << pwd));
        c.update(DB_USR, MONGO_QUERY("user_id" << user_id), p, false, false);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Password::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Password::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Password::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
