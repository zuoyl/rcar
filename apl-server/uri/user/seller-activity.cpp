//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerActivity, "user/seller-activity");

int SellerActivity::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int SellerActivity::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerActivity::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get parameters
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string activity_id = content["activity_id"].asString();

    if (role.empty() || user_id.empty() || role != "user" || activity_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    log("user_id = %s" , user_id.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log("user %s does not exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        cursor = c.query(DB_ACTIVITY, MONGO_QUERY("activity_id" << activity_id));
        if (cursor->more()) {
            BSONObj p = cursor->next();
            int count = p.getIntField("total");
            if (!p.hasField("users")) {
                BSONObj obj = BSON("total" << (count + 1) 
                              << "$set" << BSON("users" << BSON_ARRAY(activity_id)));
                c.update(DB_ACTIVITY, MONGO_QUERY("activity_id" << activity_id), obj);
            } else {
                BSONObj obj = BSON("total" << (count + 1) 
                              << "$push" << BSON("users" << BSON_ARRAY(activity_id)));
                c.update(DB_ACTIVITY, MONGO_QUERY("activity_id" << activity_id), obj);
            }
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int SellerActivity::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerActivity::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
