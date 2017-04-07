//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "order.h"

namespace seller {

DECL_RESOURCE_CLASS(OrderList, "seller/order-list");

int OrderList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string scope = content["scope"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || seller_id.empty()) {
        return APIE_INVALID_CLIENT_REQ;
    }

    // create query
    Query query;
    if (scope == "user") 
        query = MONGO_QUERY("selected_seller" << seller_id << "user_id" << user_id);
    else if (scope == "seller")
        query = MONGO_QUERY("replies" << BSON("$elemMatch" << BSON("seller_id" << seller_id)));
    else 
        query = MONGO_QUERY("target_seller" << seller_id << "status" << "new" << "replies.seller_id" << BSON("$ne" << seller_id));

    Json::Value &out = req.getOutObject();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int offset = content["offset"].asInt();
        int number = content["num"].asInt();
        Json::Value orders;

        // create query conditon
        std::unique_ptr<mongo::DBClientCursor> cursor = c.query(DB_ORDER, query,  number, offset);
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value info;

            std::string order_type = obj.getStringField("order_type");
            std::string order_service_type = obj.getStringField("order_service_type");
            info["order_id"] = obj.getStringField("order_id");
            info["user_id"] = obj.getStringField("user_id");
            info["user_name"] = obj.getStringField("user_name");
            info["order_type"] = order_type;
            info["order_service_type"] = order_service_type;
            info["user"] = obj.getStringField("user_id");
            info["title"] = obj.getStringField("title");
            info["date_time"] = obj.getStringField("date_time");
            info["status"] = obj.getStringField("status"); 
            info["platenumber"] = obj.getStringField("platenumber");
            info["car_brand"] = obj.getStringField("car_brand");

            BSONObj detail = obj.getObjectField("detail");
            // get detail according to order type

            if (order_type == OrderTypeBook) {
                Json::Value detailInfo;
                Json::Value services(Json::arrayValue);
                BSONObj mservices = detail.getObjectField("services");
                for (BSONObjIterator i = mservices.begin(); i.more(); ) {
                    services.append(rcar::tojson(i.next().Obj()));

                }
                detailInfo["services"] = services;
                info["detail"] = detailInfo;
            } else if (order_type == OrderTypeBidding) {
                info["detail"] = rcar::tojson(detail); 
            }

            // get replies if exist
            if (scope == "seller" && obj.hasField("replies")) {
                BSONObj replies = obj.getObjectField("replies");
                for (BSONObjIterator i = replies.begin(); i.more();) {
                    BSONObj reply = i.next().Obj();
                    if (seller_id == reply.getStringField("seller_id")) {
                        info["reply"] = rcar::tojson(reply);
                        break;
                    }
                }
            } 
            orders.append(info);
        }
        out["data"] = orders;
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
