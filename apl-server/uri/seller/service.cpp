//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Service, "seller/service");

int Service::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Service::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string service_id = OID::gen().toString();
    std::string title = content["title"].asString();
    std::string detail = content["detail"].asString();
    std::string url = content["url"].asString();
    std::string type = content["type"].asString();
    std::string price = content["price"].asString();
    //type is:car_clean, major_maintenance, small_maintenance
    //trailer_service, 
    if (role.empty() || seller_id.empty() || title.empty()
            || detail.empty() || type.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s, title = %s, desc = %s, type = %s, price = %s",
            seller_id.c_str(), title.c_str(), detail.c_str(), type.c_str(), price.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller no exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }
        // construct bson object
        BSONArray images = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(images);;
        BSONObj p = BSON("service_id" << service_id
                         << "type" << type
                         << "title" << title
                         << "detail" << detail 
                         << "price" << price
                         << "images" << images
                         );
        out["service_id"] = service_id;


        BSONObj seller = cursor->next();

        if (!seller.hasField("services")) {
            BSONArrayBuilder arrayBuilder;
            arrayBuilder.append(p);

            BSONObjBuilder objBuilder;
            objBuilder.append("services", arrayBuilder.arr());

            // update
            BSONObjBuilder set;
            set.append("$set", objBuilder.obj());
            c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), set.obj());
        } else {
            BSONElement activities = seller.getField("services");
            BSONObj obj = activities.Obj();
            // check wether the service already exist
            for (BSONObj::iterator i = obj.begin();i.more();) {
                BSONObj service = i.next().Obj();
                BSONElement e = service["title"]; 
                if (title == e.toString())
                    return APIE_INVALID_CLIENT_REQ;
            }
            // add new activity into list
            BSONObjBuilder objBuilder;
            objBuilder.append("services", p);
            // update
            BSONObjBuilder push;
            push.append("$push", objBuilder.obj());
            c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), push.obj());
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Service::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string service_id = content["service_id"].asString();
    std::string title = content["title"].asString();
    std::string desc = content["desc"].asString();
    std::string url = content["url"].asString();
    std::string type = content["type"].asString();
    double price = content["price"].asDouble();

    if (role.empty() || seller_id.empty() || title.empty()
            || desc.empty() || url.empty() || type.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s, title = %s, desc = %s, url = %s, type = %s, price = %f",
            seller_id.c_str(), title.c_str(), desc.c_str(), url.c_str(), type.c_str(), price);

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller no exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }
        BSONArray images = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(images);;
 
        BSONObj service = BSON("service_id" << service_id
                               << "type" << type
                               << "title" << title
                               << "desc" << desc
                               << "url" << url
                               << "images" << images);

        // delete service at first 
        BSONObj pull = BSON("$pull" << BSON("services" << BSON("service_id" << service_id)));
        c.update(DB_SELLER, MONGO_QUERY("role" << role << "seller_id" << seller_id), pull, true, false);

        // insert new activity into activity set 
        BSONObj push = BSON("$push" << BSON("services" << service));
        c.update(DB_SELLER, MONGO_QUERY("role" << role << "seller_id" << seller_id), push, true, false);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Service::Delete(const ResourceContext &context, Request &req) {
    const Json::Value& content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string service_id = content["service_id"].asString();

    if (role.empty() || seller_id.empty() || service_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s, service_id = %s", 
            seller_id.c_str(), service_id.c_str());

    try {
        // connect DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the service exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
	
        if (!cursor->more()) {
            log("service no exist", service_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        } 
        // delete service 
        BSONObj p = BSON("$pull" << BSON("services" << BSON("service_id" << service_id)));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), p, true, false);
 
        return APIE_OK;

    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Service::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
