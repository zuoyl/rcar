//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerCommodity, "user/seller-commodity");

int SellerCommodity::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    std::string role = content["role"].asString();
    // get seller's name
    std::string commodity_id = content["commodity_id"].asString();
    if (commodity_id.empty()) {
        log(__func__, "invalid client request, id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_STORE, MONGO_QUERY("commod_id" << commodity_id));
       if (cursor->more()) {
           BSONObj obj = cursor->next();
           Json::Value info;
           out["commodity_id"] = obj.getStringField("commod_id");
           out["name"] = obj.getStringField("name");
           out["desc"] = obj.getStringField("desc");
           out["price"] = obj.getStringField("price");
           out["rate"] = obj.getStringField("rate");
           out["brand"] = obj.getStringField("brand");
           out["cutoff"] = obj.getStringField("amount");
           out["images"] = getImageNamesFromBson(obj, "images");
       }
       else {
           return APIE_INVALID_CLIENT_REQ;
       }
       return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int SellerCommodity::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerCommodity::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerCommodity::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerCommodity::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
