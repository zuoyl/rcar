//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(CommodityList, "seller/commodity-list");

int CommodityList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    // get seller's name
    std::string seller_id = content["seller_id"].asString();
    if (seller_id.empty()) {
        error(__func__, "invalid client request, id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller doesn't exist", seller_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        
        // open commodity database and get basic information
        int offset = content["offset"].asInt();
        int num = content["num"].asInt();
        Json::Value com_list;
        cursor = c.query(DB_STORE, MONGO_QUERY("seller_id" << seller_id), num, offset);
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value info;
            info["commodity_id"] = obj.getStringField("commodity_id");
            info["name"] = obj.getStringField("name");
            info["detail"] = obj.getStringField("desc");
            info["price"] = obj.getStringField("price");
            info["rate"] = obj.getStringField("rate");
            info["brand"] = obj.getStringField("brand");
            info["cutoff"] = obj.getStringField("amount");
            info["images"] = getImageNamesFromBson(obj, "images");
            com_list.append(info);
        }
        out["data"] = com_list;
        out["status"] = APIE_OK;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int CommodityList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int CommodityList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int CommodityList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int CommodityList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
