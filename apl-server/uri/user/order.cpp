//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "order.h"

namespace user {

DECL_RESOURCE_CLASS(Order, "user/order");


int Order::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get seller's name
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string order_id = content["order_id"].asString();
    std::string status = content["status"].asString();

    if (role.empty() || role != "user" || user_id.empty()|| order_id.empty()) {
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        // get order detail info
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ORDER, MONGO_QUERY("order_id" << order_id << "user_id" << user_id));
        if (!cursor->more()) {
            log(__func__," invliad order id(%s)", order_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        BSONObj obj = cursor->next();

        if (!status.empty()) { // return order status only
            out["order_status"] = obj.getStringField("status");
            out["order_id"] = obj.getStringField("order_id");
            return APIE_OK;
        }

        out["order_id"] = obj.getStringField("order_id");
        out["user_id"] = user_id;
        out["order_type"] = obj.getStringField("order_type");
        out["order_service_type"] = obj.getStringField("order_service_type");
        out["date_time"] = obj.getStringField("date_time");
        out["status"] = obj.getStringField("status"); 
        out["platenumber"] = obj.getStringField("platenumber");
        out["selected_seller"] = obj.getStringField("selected_seller");
        out["detail"] = rcar::tojson(obj, "detail");

        std::string order_status = obj.getStringField("status");
        if (order_status == "confirmed") {
            std::string seller_id = obj.getStringField("selected_seller");
            if (obj.hasField("replies")) {
                BSONObj replies = obj.getObjectField("replies");
                for (BSONObjIterator i = replies.begin(); i.more(); ) {
                    BSONObj seller = i.next().Obj();
                    if (seller_id == seller.getStringField("seller_id")) {
                        out["seller"] = rcar::tojson(seller);
                        break;
                    }
                }
            }
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
}

static void getSellersId( mongo::DBClientConnection &c,
        Json::Value &location,
        Json::Value &condition,
        std::vector<std::string> &sellers) {
    int radius = atoi(condition["radius"].asString().c_str());
    std::string type = condition["type"].asString();
    std::string level = condition["level"].asString();
    double lat = location["lat"].asDouble();
    double lng = location["lng"].asDouble();

    if (radius== 0) radius = 10;

    // search sellers within bound of radius
    std::unique_ptr<mongo::DBClientCursor> cursor = c.query(DB_SELLER,
            MONGO_QUERY("location" << BSON("$near" << BSON_ARRAY(lng << lat) 
                    << "$maxDistance" << radius)));
    while (cursor->more()) {
        BSONObj seller = cursor->next();
        std::string seller_type = seller.getStringField("type");
        std::string seller_id = seller.getStringField("seller_id");
        if (type == "all" || type == seller_type) 
            sellers.push_back(seller_id);
    }
}

static void makeOrderDetail(
        mongo::BSONObjBuilder &builder, 
        const std::string &orderType, const std::string &serviceType,
        const Json::Value &content, 
        mongo::DBClientConnection &c) {

    std::string position = content["position"].asString();
    Json::Value items = content["items"];
    BSONObjBuilder objBuilder; 

    if (orderType == OrderTypeBook) {
        Json::Value service_list = content["service_list"];
        Json::Value sellers = content["target_seller"];
        std::string seller_id = sellers[(unsigned int)0].asString();
        // get serivce type/title/price
        std::unique_ptr<mongo::DBClientCursor> cursor =
                c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (cursor->more()) {
           BSONObj seller = cursor->next();
           objBuilder.append("seller_name", seller.getStringField("name"));
           BSONArrayBuilder arrayBuilder; 
           // insert service list 
           for (unsigned int index = 0; index < service_list.size(); index++) {
               std::string service_id = service_list[index].asString();
               BSONObj service_list = seller.getObjectField("services");
               for (BSONObj::iterator i = service_list.begin(); i.more();) {
                   BSONObj service = i.next().Obj();
                   std::string id = service.getStringField("service_id");
                   std::string service_type = service.getStringField("type");
                   std::string price = service.getStringField("price");
                   std::string title = service.getStringField("title");
                   if (id == service_id) {
                       BSONObjBuilder info;
                       info.append("service_id", id);
                       info.append("service_type", service_type);
                       info.append("price", price);
                       info.append("title", title);
                       arrayBuilder.append(info.obj());
                   }
               }
           }
           objBuilder.append("services", arrayBuilder.arr());
        }
        builder.append("detail", objBuilder.obj());

    } else if (serviceType == OrderServiceTypeMaintenance) {
#if 0
        // position
        objBuilder.append("type", items["type"].asString());
        objBuilder.append("mileage", items["mileage"].asString());
        objBuilder.append("mileage_items", rcar::fromjson(items, "mileage_items"));
        objBuilder.append("user_items",rcar::fromjson(items, "user_items"));
        builder.append("detail", objBuilder.obj());
#else 
        builder.append("detail", rcar::fromjson(items));
#endif

    } else if (serviceType == OrderServiceTypeFault) {
        objBuilder.append("position", position);
        objBuilder.append("touser", content["touser"].asString());
        BSONArrayBuilder arrayBuilder;
        for (std::size_t i = 0; i < items.size(); i++) {
            arrayBuilder.append(items[i].asString());
        }
        objBuilder.append("items", arrayBuilder.arr());
        builder.append("detail", objBuilder.obj());
    }
}

int Order::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get parameters
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string mode = content["mode"].asString();
    std::string date = content["date_time"].asString();
    std::string order_type = content["order_type"].asString();
    std::string order_service_type = content["order_service_type"].asString();

    std::string position = content["position"].asString();
    Json::Value sellers = content["target_seller"];

    // get target sellers id
    Json::Value condition  = content["condition"];
    Json::Value location = content["location"];

    OID oid = OID::gen();
    std::string order_id = oid.toString();

    if (role.empty() || user_id.empty() || role != "user" || sellers.empty())
        return APIE_INVALID_CLIENT_REQ;

    log("user_id = %s" , user_id.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log("user %s does not exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        BSONObj user = cursor->next();

        // if target seller is not specfied, search all sellrs by condtion
        std::vector<std::string> target_sellers;
        if (sellers.empty()) {
            getSellersId(c, location, condition, target_sellers); 
        } else {
            for (unsigned int index = 0; index < sellers.size(); index++) {
                Json::Value seller = sellers[index];
                target_sellers.push_back(seller.asString());
            }
        }
        if (target_sellers.empty())  {
            log(__func__, "no sellers exist for the order");
            return APIE_NO_SERVICE;
        }

        // insert maintenance report to db.maintenance
        BSONObjBuilder builder;
        builder.append("role", "user");
        builder.append("user_id", user_id);
        builder.append("user_name", user.getStringField("name"));
        builder.append("order_id", order_id);
        builder.append("order_type", order_type);
        builder.append("order_service_type", order_service_type);
        builder.append("date_time", date);
        builder.append("publicate_time", content["publicate_time"].asString());
        builder.append("platenumber", content["platenumber"].asString());
        // location
        double lat = location["lat"].asDouble();
        double lng = location["lng"].asDouble();
        builder.append("location", BSON_ARRAY(lat << lng));

        // get car brand
        std::string platenumber = content["platenumber"].asString();
        BSONObj cars = user.getObjectField("cars");
        for (BSONObj::iterator i = cars.begin(); i.more();) {
            BSONObj car = i.next().Obj();
            if (platenumber == car.getStringField("platenumber")) {
                builder.append("car_band",  car.getStringField("band"));
                break;
            }
        }
        //  condition
        {
            BSONObjBuilder b;
            b.append("radius", condition["radius"].asString());
            b.append("seller", condition["seller"].asString());
            b.append("level",  condition["level"].asString()); 
            builder.append("condition", b.obj());
        }

        // mode setting
#if 0
        if (target_sellers.size() > 1) { 
            builder.append("order_type", OrderTypeBidding);
        } else {
            builder.append("order_type", OrderTypeBook);
            builder.append("order_service_type", OrderTypeBook);
        }
#endif
        // images
        BSONArray bsonNames = getImageNames(content["images"]); 
        builder.append("images", bsonNames); 
        out["images"] = getImageNamesFromBson(bsonNames);



        makeOrderDetail(builder, order_type, order_service_type, content, c);
        builder.append("status", "new");

        // target seller
        BSONArrayBuilder sellers_builder;
        std::vector<std::string>::iterator ite = target_sellers.begin();
        for (; ite != target_sellers.end(); ite++) {
            sellers_builder.append(*ite);
        }
        builder.append("target_seller", sellers_builder.arr());			

        c.insert(DB_ORDER, builder.obj());
        out["order_id"] = order_id; 
        // notify all seller who saitisified the conditon
        Json::Value msg;
        msg["source"] = "user";
        msg["kind"] = "order";
        msg["type"] = order_service_type;
        msg["order_id"] = order_id;
        pushMessageToSellers(target_sellers, msg);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;


}
int Order::Put(const ResourceContext &context, Request &req) { 
     //get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string order_id = content["order_id"].asString();
    std::string status = content["status"].asString();


    if (user_id.empty() || order_id.empty() || status.empty()) {
        log(__func__, "invalid client request, seller_id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    if (role != "user") {
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
            std::string mode = order.getStringField("mode");
            std::string order_status = order.getStringField("status");

            if (order_status == "new") {
                // if order status is new, user can cancel/agree the order
                if (status == "canceled") {
                    BSONObj p = BSON("$set" << BSON("status" << "canceled"));
                    c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), p);
#if 0
                    // Todo:notify seller 
                    MsgPublish publish;
                    std::vector<std::string> uuids;
                    uuids.push_back(user_id);
                    Json::Value msg;
                    msg["type"] = "order_status_change";
                    msg["order_id"] = order_id;
                    msg["seller_id"] = seller_id;
                    publish.push_msg_someuser(uuids, msg.asString());
#endif 
                } else if (status == "agree") {
                    // the status indicate that user select seller for the order
                    std::string seller_id = content["seller_id"].asString();
                    BSONObj p = BSON("$set" << BSON("status" << "confirmed" << "selected_seller" << seller_id));
                    c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), p);
#if 0
                    // TODO:notify seller 
                    MsgPublish publish;
                    std::vector<std::string> uuids;
                    uuids.push_back(user_id);
                    Json::Value msg;
                    msg["type"] = "order_status_change";
                    msg["order_id"] = order_id;
                    msg["seller_id"] = seller_id;
                    publish.push_msg_someuser(uuids, msg.asString());
#endif 
 
                }
            } else if (order_status == "canceled") { 
                // if the order is canceled, user can delete the order
                if (status == "delete") {
                    // TODO:notify seller
                    c.remove(DB_ORDER, MONGO_QUERY("order_id" << order_id));
                    return APIE_OK;
                }
            } else if (order_status == "selected") {
                // if the order is selected, user can cancel or delete the order
                if (status == "cancel") {
                    BSONObj p = BSON("$set" << BSON("status" << "canceled"));
                    c.update(DB_ORDER, MONGO_QUERY("order_id" << order_id), p);
                }
                else if (status == "delete") {
                    c.remove(DB_ORDER, MONGO_QUERY("order_id" << order_id));
                }
                // TODO: notify the user
            } else {
                log(__func__, "invalid order status");
                return APIE_SERVER_INTERNAL;
            }
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

int Order::Delete(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get seller's name
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string order_id = content["order_id"].asString();

    if (role.empty() || role != "user" || user_id.empty()|| order_id.empty()) {
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ORDER, MONGO_QUERY("user_id" << user_id <<"order_id" << order_id));
        if (cursor->more()) {
            BSONObj order = cursor->next();
            c.remove(DB_ORDER, MONGO_QUERY("user_id" << user_id <<"order_id" << order_id));
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

int Order::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
