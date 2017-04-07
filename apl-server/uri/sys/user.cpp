//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(User, "sys/user");

int User::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &user = req.getOutObject();

    // get client request of user's id
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "sys")
        return APIE_INVALID_CLIENT_REQ;

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //get user's detail information
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));

        if (!cursor->more()) {
            log(__func__,"%s is not registered", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        BSONObj p = cursor->next();
        user["user_name"] = p.getStringField("user_name");
        user["sex"] = p.getStringField("sex");
        user["email"] = p.getStringField("email");
        user["home_city"] = p.getStringField("home_city");            
        user["home_addr"] = p.getStringField("home_addr");
        user["images"] = get_images(p, "images");
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
 
    return APIE_OK;

}
int User::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int User::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int User::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int User::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
