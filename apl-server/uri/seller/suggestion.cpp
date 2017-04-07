//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Suggestion, "seller/suggestion");

int Suggestion::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Suggestion::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["id"].asString();
    std::string suggestion = content["id"].asString();

    if (role.empty() || seller_id.empty() || suggestion.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s", seller_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }
        //check password
        BSONObj seller = cursor->next();

        BSONObj  item = BSON("suggestions" << BSON("title" << suggestion)); 
        BSONObjBuilder builder;
        builder.append("$push", item);
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), builder.obj(), true, false);


        // TODO: notify system manager about the suggestions
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Suggestion::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Suggestion::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Suggestion::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
