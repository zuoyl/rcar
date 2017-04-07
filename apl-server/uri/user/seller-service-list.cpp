//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerServiceList, "user/seller-service-list");

int SellerServiceList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();

    if (role.empty() || seller_id.empty() || role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log("role = %s, seller_id = %s", role.c_str(), seller_id.c_str());

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // get seller's service information
        int count = 0;
        Json::Value services(Json::arrayValue);

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        while (cursor->more()) {
            BSONObj seller = cursor->next();
            if (seller.hasField("services")) {
                BSONObj p = seller.getObjectField("services");
                for (BSONObj::iterator i = p.begin(); i.more();) {
                    BSONObj service = i.next().Obj();
                    Json::Value info;
                    info["service_id"] = service.getStringField("service_id");
                    info["title"] = service.getStringField("title");
                    info["url"] = service.getStringField("url");
                    info["type"] = service.getStringField("type");			
                    info["desc"] = service.getStringField("desc");
                    info["price"] = service.getStringField("price");
                    info["images"] = getImageNamesFromBson(service, "images");
                    services.append(info);
                    count++;
                }
            }
        }
        out["total"] = count;
        out["data"] = services;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int SellerServiceList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerServiceList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerServiceList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerServiceList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
