//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(OrderReplies, "user/order-replies");


int OrderReplies::Get(const ResourceContext &context, Request &req) {
    //get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string order_id = content["order_id"].asString();
    std::string user_id = content["user_id"].asString();
    int offset = content["offset"].asInt();
    int num = content["num"].asInt();


    if (user_id.empty() || order_id.empty()) {
        log(__func__,"invalid client request, seller_id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    if (role != "user") {
        log(__func__,"invalid client request, role error");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // get order from db
        std::unique_ptr<mongo::DBClientCursor> cursor = c.query(DB_ORDER, 
                MONGO_QUERY("user_id" << user_id << "order_id" << order_id << "status" << "new"));
        if (cursor->more()) {
            BSONObj order = cursor->next();
            if (!order.hasField("replies")) // sellers dont replies
                return APIE_OK;


            std::string order_type = order.getStringField("order_type");
            std::string order_service_type = order.getStringField("order_service_type");
            BSONObj location = order.getObjectField("location");

            int count = 0;
            Json::Value replies(Json::arrayValue);
            BSONObj p = order.getObjectField("replies");
            for (int i = offset; (i < num && i < p.nFields()); i++) {
                BSONObj reply = p[i].Obj();
                Json::Value item;

                // get replies information
                std::string status = reply.getStringField("status");
                if (status == "agree") {
                    // get seller info
                    std::string seller_id = reply.getStringField("seller_id");
                    cursor = c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
                    if (cursor->more()) {
                        BSONObj seller = cursor->next();
                        item["name"] = seller.getStringField("name");
                        item["address"] = seller.getStringField("address");
                        item["telephone"] = seller.getStringField("telephone");
                        item["rate"] = seller.getStringField("rate");
                        item["cost"] = reply.getStringField("cost");
                        item["time"] = reply.getStringField("time");
                        item["seller_id"] = seller_id;
                        item["images"] = getImageNamesFromBson(reply, "images");

                        // get distance
                        BSONObj loc = seller.getObjectField("location");
                        double lat = loc[0].Double();
                        double lng = loc[1].Double();
                        double distance = get_distance(lat, lng, location[0].Double(), location[1].Double());
                        item["distance"] = distance;
                        //set seller's location
                        Json::Value lloc;
                        lloc["lat"] = lat;
                        lloc["lng"] = lng;
                        item["location"] = lloc;

                        replies.append(item);
                        count++;
                    }
               }
            }
            out["total"] = count;
            out["data"] = replies;
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;


}
int OrderReplies::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int OrderReplies::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int OrderReplies::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req);
}

int OrderReplies::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
