//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerCommodityList, "user/seller-commodity-list");

int SellerCommodityList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string seller_id = content["seller_id"].asString();
    std::string role = content["role"].asString();

    if (role != "user" || seller_id.empty()) {
        log(__func__, "invalid client request, id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // open commodity database and get basic information
        int offset = content["offset"].asInt();
        int num = content["num"].asInt();
        int count = 0;
        Json::Value commodities;
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_STORE, MONGO_QUERY("seller_id" << seller_id), num, offset);
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value info;
            info["cid"] = obj.getStringField("commod_id");
            info["name"] = obj.getStringField("name");
            info["desc"] = obj.getStringField("desc");
            info["price"] = obj.getStringField("price");
            info["rate"] = obj.getStringField("rate");
            info["brand"] = obj.getStringField("brand");
            info["cutoff"] = obj.getStringField("amount");
            info["images"] = getImageNamesFromBson(obj, "images");
            commodities.append(info);
            count++;
        }
        
        out["total"] = count;
        out["data"] = commodities;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int SellerCommodityList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerCommodityList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerCommodityList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerCommodityList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
