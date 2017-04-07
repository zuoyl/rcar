//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Accusation, "user/accusation");

int Accusation::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Accusation::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    //get client request's patameter and check
    std::string role = content["role"].asString();
    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;
    std::string user_id = content["user_id"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string description = content["description"].asString();
    std::string time = get_customstring_time();
    std::string accusation_id = OID::gen().toString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        //first, query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__, "user %s does not exist", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        } 

        //insert accusation to db.falseInfo
        BSONObjBuilder builder;
        builder.append("accusation_id", accusation_id);
        builder.append("user_id", user_id);
        builder.append("seller_id", seller_id);		
        builder.append("description", description);
        builder.append("time", time);		
        builder.append("status", "new");
        builder.append("images", getImageNames(content["images"]));
        c.insert(DB_ACCUSATION, builder.obj());
        // notify
        Json::Value msg;
        msg["source"] = "user";
        msg["kind"] = "accusation";
        pushMessageToSeller(seller_id, msg);
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Accusation::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Accusation::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Accusation::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
