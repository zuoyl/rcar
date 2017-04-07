//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "order.h"

namespace user {

DECL_RESOURCE_CLASS(OrderList, "user/order-list");

int OrderList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get seller's name
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || role != "user" || user_id.empty()) {
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int offset = content["offset"].asInt();
        int number = content["num"].asInt();
        int count = 0;
        Json::Value order_list;

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ORDER, MONGO_QUERY("user_id" << user_id),  number, offset);
        while (cursor->more()) {
            BSONObj p = cursor->next();
            Json::Value info;
            info["order_id"] = p.getStringField("order_id");
            info["order_type"] = p.getStringField("order_type");
            info["order_service_type"] = p.getStringField("order_service_type");
            info["user"] = p.getStringField("user_id");
            info["date_time"] = p.getStringField("date_time");
            info["status"] = p.getStringField("status"); 
            info["platenumber"] = p.getStringField("platenumber");
            // images
            info["images"] = getImageNamesFromBson(p, "images");
            info["detail"] = rcar::tojson(p, "detail");
            order_list.append(info);
            count++;
        }
        out["total"] = count;
        out["data"] = order_list;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int OrderList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int OrderList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int OrderList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int OrderList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
