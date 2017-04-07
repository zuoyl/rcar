//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Password, "seller/pwd");

int Password::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Password::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string pwd = content["pwd"].asString();
   
    if (role.empty() || seller_id.empty() || pwd.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__, "seller_id = %s, pwd = %s", seller_id.c_str(), pwd.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__, "seller is not exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }

        // update record
        BSONObj p = BSON("$set" << BSON("pwd" << pwd));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), p, false, false);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
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
