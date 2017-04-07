//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "order.h"

namespace seller {

DECL_RESOURCE_CLASS(Order, "seller/order");

int Order::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get seller's name
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string order_id = content["order_id"].asString();
    std::string statusOnly = content["status"].asString();

    if (role.empty() || role != "seller" || seller_id.empty()|| order_id.empty()) {
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ORDER, MONGO_QUERY("order_id" << order_id));
        if (cursor->more()) {
            BSONObj obj = cursor->next();

            if (!statusOnly.empty()) {
                out["order_id"] = obj.getStringField("order_id");
                out["order_status"] = obj.getStringField("status"); 
                return APIE_OK;
            }
            out["order_id"] = obj.getStringField("order_id");
            out["order_type"] = obj.getStringField("order_type");
            out["order_service_type"] = obj.getStringField("order_service_type");
            out["user_id"] = obj.getStringField("user_id");
            out["user_name"] = obj.getStringField("user_name");
            out["date_time"] = obj.getStringField("date_time");
            out["status"] = obj.getStringField("status"); 
            out["platenumber"] = obj.getStringField("platenumber");
            out["car_band"] = obj.getStringField("car_band");

            // if order is selected or completed, get order cost info
            if (obj.hasField("replies")) {
                out["replies"] = rcar::tojson(obj, "replies"); 
            }
            out["detail"] = rcar::tojson(obj, "detail");
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Order::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
// set order status
int Order::Put(const ResourceContext &context, Request &req) { 
     //get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string order_id = content["order_id"].asString();
    std::string status = content["status"].asString();


    if (seller_id.empty() || order_id.empty() || status.empty()) {
        log(__func__, "invalid client request, seller_id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    if (role != "seller") {
        log(__func__, "invalid client request, role error");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // get order from db
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ORDER, MONGO_QUERY("order_id" << order_id));
        if (cursor->more()) {
            BSONObj order = cursor->next();
            std::string user_id = order.getStringField("user_id");
            std::string order_type = order.getStringField("order_type");
            std::string order_service_type = order.getStringField("order_service_type");
            std::string order_status = order.getStringField("status");

            if (status == "agree") {
                std::string cost = content["cost"].asString();
                std::string time = content["time"].asString();
                std::string feedback = content["feedback"].asString();
                if (order_status == "new") {
                    BSONObjBuilder pBuilder;
                    pBuilder.append("seller_id", seller_id);
                    pBuilder.append("cost", cost);
                    pBuilder.append("time", time);
                    pBuilder.append("feedback", feedback);
                    pBuilder.append("status", "agree");

                    if (order_type == OrderTypeBidding && 
                            order_service_type == OrderServiceTypeMaintenance) {
                        pBuilder.append("mileage_items", rcar::fromjson(content, "mileage_items"));
                        pBuilder.append("user_items", rcar::fromjson(content, "user_items"));
                    }

                    // check wether "reply" exist
                    if (!order.hasField("replies")) {
                        BSONArrayBuilder arrayBuilder;
                        arrayBuilder.append(pBuilder.obj());

                        // update
                        BSONObjBuilder set;
                        set.append("$set", BSON("replies" << arrayBuilder.arr()));
                        c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), set.obj());

                    } else {
                        // update
                        BSONObj push = BSON("$push" << BSON("replies" << pBuilder.obj()));
                        c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), push);
                    }
                    // Todo:notify user
                    Json::Value msg;
                    msg["source"] = "seller";
                    msg["kind"] = "order";
                    msg["type"] = "order";
                    msg["order_id"] = order_id;
                    msg["seller_id"] = seller_id;
                    msg["status"] = "agree";
                    pushMessageToUser(user_id, msg);
                } else {
                    log(__func__, "invalid order status");
                    return APIE_SERVER_INTERNAL;
                }
            } else if (status == "ignore") {
                // remove seller from target_seller
                BSONObj pull = BSON("$pull" << BSON("target_seller" << seller_id));
                c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), pull);
            } else if (status == "canceled") {
                // remove seller from target_seller
                BSONObj pull = BSON("$pull" << BSON("target_seller" << seller_id));
                c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), pull);
            } else if (status == "completed") {
                BSONObj set = BSON("$set" << BSON("status" << "completed"));
                c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), set);
                //TODO: notify user
            }
            return APIE_OK;
        } else  {
            log("invalid order id", order_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
    } catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Order::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Order::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
