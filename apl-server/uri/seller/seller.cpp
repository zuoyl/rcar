//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Seller, "seller/");

int Seller::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &seller = req.getOutObject();

    // get valid seller's uuid 
    std::string seller_id = content["seller_id"].asString();
    if (seller_id.empty()) {
        log(__func__, "seller_id is empty");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (cursor->more()) {
            BSONObj p = cursor->next();
            seller["seller_id"] = p.getStringField("seller_id");			
            seller["name"] = p.getStringField("name");
            seller["telephone"] = p.getStringField("telephone");
            seller["address"] = p.getStringField("addr");
            seller["intro"] = p.getStringField("intro");
            seller["city"] = p.getStringField("city");
            seller["address"] = p.getStringField("address");
            seller["service_start_time"] = p.getStringField("service_start_time");
            seller["service_end_time"] = p.getStringField("service_end_time");
            // images
            seller["face_images"] = getImageNamesFromBson(p, "face_images");
            seller["eco_images"] = getImageNamesFromBson(p, "eco_images");
            seller["internal_images"] = getImageNamesFromBson(p, "internal_images");
 
            // location
            BSONObj loc = p.getObjectField("location");
            Json::Value location;
            location["lat"] = loc[0].Double();
            location["lng"] = loc[1].Double();
            seller["location"] = location;

            // cars
            BSONObj cars = p.getObjectField("cars");
            Json::Value car_list;
            for (BSONObj::iterator i = cars.begin(); i.more();) {
                car_list.append(i.next().str());
            }
            seller["cars"] = car_list;

            Json::Value commodities;
            std::unique_ptr<mongo::DBClientCursor> com_cursor = 
                c.query(DB_STORE, MONGO_QUERY("seller_id" << seller_id));
            while (com_cursor->more()) {
                BSONObj obj = com_cursor->next();
                Json::Value info;
                info["seller_id"] = obj.getStringField("seller_id");	
                info["commodity_id"] = obj.getStringField("commodity_id");	 
                info["name"] = obj.getStringField("name");
                info["detail"] = obj.getStringField("desc");		 
                info["price"] = obj.getStringField("price");
                info["brand"] = obj.getStringField("brand");		 
                info["cutoff"] = obj.getStringField("cutoff");
                info["total"] = obj.getStringField("amount");
                // how to deal with;
                info["rate"] = "100%";
                info["images"] = getImageNamesFromBson(obj, "images");
                commodities.append(info);
            }
            seller["commodities"] = commodities;

            // activities
            if (p.hasField("activities")) {
                Json::Value activities;
                BSONObj activity_list = p.getObjectField("activities");
                for (BSONObj::iterator i = activity_list.begin(); i.more();) {
                    BSONObj obj = i.next().Obj();
                    Json::Value info;
                    info["seller_id"] = obj.getStringField("seller_id");
                    info["activity_id"] = obj.getStringField("activity_id");
                    info["url"] = obj.getStringField("url");
                    info["title"] = obj.getStringField("title");		 
                    info["detail"] = obj.getStringField("detail");
                    info["start_date"] = obj.getStringField("start_date");
                    info["end_date"] = obj.getStringField("end_date");
                    info["images"] = getImageNamesFromBson(obj, "images");
                    activities.append(info);
                }	
                seller["activities"] = activities;
            }
            
            // services
            if (p.hasField("services")) {
                Json::Value services;
                BSONObj service_list = p.getObjectField("services");
                for (BSONObj::iterator i = service_list.begin(); i.more();) {
                    BSONObj obj = i.next().Obj();
                    Json::Value info;
                    info["seller_id"] = obj.getStringField("seller_id");
                    info["service_id"] = obj.getStringField("service_id");
                    info["type"] = obj.getStringField("type");
                    info["title"] = obj.getStringField("title");		 
                    info["detail"] = obj.getStringField("detail");
                    info["price"] = obj.getStringField("price");
                    info["start_time"] = obj.getStringField("start_time");
                    info["end_time"] = obj.getStringField("end_time");
                    info["images"] = getImageNamesFromBson(obj, "images");
                    services.append(info);
                }	
                seller["services"] = services;
            }
            // user groups
            Json::Value groupArray(Json::arrayValue); 
            cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));
            if (cursor->more()) {
                // get all user list
                BSONObj p = cursor->next();
                BSONObj groups = p.getObjectField("groups");
                for (BSONObjIterator i = groups.begin(); i.more();) {
                    Json::Value group;
                    Json::Value users;
                    // group
                    BSONObj obj = i.next().Obj();
                    group["group_id"] = obj.getStringField("group_id");
                    group["name"] = obj.getStringField("group_name");
                    // users
                    BSONObj p = obj.getObjectField("users");
                    for (BSONObjIterator n = p.begin(); n.more();) {
                        std::string id = n.next().str();

                        // user info
                        Json::Value user;
                        user["user_id"] = id;
                        // get user online status

                        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << id));
                        if (cursor->more()) {
                            BSONObj pp = cursor->next();
                            user["online"] = pp.getIntField("online");
                            user["name"] = pp.getStringField("name");
                        }  else {
                            user["online"] = 0;
                            user["name"] = "unknown";
                        }
                        users.append(user);
                    }
                    group["users"] = users;
                    groupArray.append(group);
                }
            }
            seller["groups"] = groupArray;
            seller["deals"] = 0;
            return APIE_OK;
        }
        else {
            return APIE_SELLER_NO_EXIST;
        }
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
 
    return APIE_OK;

}
int Seller::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["mobile"].asString();
    std::string check = content["check"].asString();

    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;


    log("seller_id = %s", seller_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (cursor->more()) {
                log("seller %s already exist", seller_id.c_str());
                return APIE_SELLER_ALREADY_EXIST;
        }
        if (!check.empty())
            return APIE_OK;

        // get parameter
        std::string pwd = content["pwd"].asString();
        std::string name = content["shop_name"].asString();
        std::string city = content["shop_city"].asString();	
        std::string telephone = content["shop_telephone"].asString();
        std::string address = content["shop_address"].asString();
        std::string service_start_time = content["service_start_time"].asString();
        std::string service_end_time = content["service_end_time"].asString();

        Json::Value cars  = content["cars"];	      
        Json::Value location = content["location"];
        Json::Value services = content["services"];
 
        // insert record
        BSONObjBuilder builder;
        builder.append("seller_id", seller_id); 
        builder.append("pwd", pwd);
        builder.append("name", name);
        builder.append("city", city);
        builder.append("address", address);
        builder.append("telephone", telephone);
        builder.append("service_start_time", service_start_time);
        builder.append("service_end_time", service_end_time);
        builder.append("status", "authenticating");

        // service
        BSONArrayBuilder service_list;
        if (!services.isNull()) {
            Json::Value::iterator ite = services.begin();
            for (; ite != services.end(); ite++) {
                Json::Value service = *ite;
                // insert record
                BSONObjBuilder p;
                OID service_id = OID::gen();
                p.append("service_id", service_id.toString());
                p.append("type", service["type"].asString());
                p.append("title", service["title"].asString());
                p.append("price", service["price"].asString());
                p.append("title", service["title"].asString());
                p.append("detail", service["detail"].asString());
                p.append("start_time", service["start_time"].asString());
                p.append("end_time", service["end_time"].asString());
                service_list.append(p.obj());
            }
        }
        builder.append("services", service_list.arr());

        // cars
        BSONArrayBuilder car_list;
        if (!cars.isNull()) {
            Json::Value::iterator ite = cars.begin();
            for (; ite != cars.end(); ite++) {
                Json::Value car = *ite;
                car_list.append(car.asString());
            }
        }
        builder.append("cars", car_list.arr());

        // location
        BSONArrayBuilder loc;
        loc.append(location["lat"].asDouble());
        loc.append(location["lng"].asDouble());
        builder.append("location", loc.arr());

        // images
        Json::Value::iterator ite;
        // face images
        BSONArray faceImageNames =  getImageNames(content["face_images"]);
        BSONArray ecoImageNames =  getImageNames(content["eco_images"]);
        BSONArray internalImageNames =  getImageNames(content["internal_images"]);

        builder.append("face_images", faceImageNames);
        builder.append("internal_images", internalImageNames);
        builder.append("eco_images", ecoImageNames);

        out["face_images"] = getImageNamesFromBson(faceImageNames);
        out["internal_images"] = getImageNamesFromBson(internalImageNames);
        out["eco_images"] = getImageNamesFromBson(ecoImageNames);
       
        // insert record
        c.insert(DB_SELLER, builder.obj());

        // initialize other db
        c.insert(DB_MSG, BSON("seller_id" << seller_id));
        c.insert(DB_USRMNG, BSON("seller_id" << seller_id));

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
// update
int Seller::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's detail information
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string name = content["name"].asString();
    std::string address = content["address"].asString();
    Json::Value location = content["location"];
    std::string intro = content["intro"].asString();
    std::string telephone = content["telephone"].asString();

    // update db
    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // first. query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__, "seller doesn't exist");
            return APIE_INVALID_CLIENT_REQ;
        }
        BSONObj seller = cursor->next();

        // insert record
        BSONObjBuilder builder;
        builder.append("name", name);
        builder.append("address", address);
        BSONObjBuilder loc;
        loc.append("lng", location["lng"].asString());
        loc.append("lat",  location["lat"].asString());
        builder.append("location", loc.obj());
        builder.append("intro", intro);
        builder.append("telephone", telephone);

        // remove images
        removeImages(seller, "face_images");
        removeImages(seller, "eco_images");
        removeImages(seller, "internal_images");
        // add images agagin
        BSONArray faceImageNames = getImageNames(content["face_images"]);
        BSONArray ecoImageNames = getImageNames(content["eco_images"]);
        BSONArray internalImageNames = getImageNames(content["internal_images"]);

        builder.append("face_images", faceImageNames);
        builder.append("eco_images", ecoImageNames);
        builder.append("internal_images", internalImageNames);

        out["face_images"] = getImageNamesFromBson(faceImageNames);
        out["eco_images"] = getImageNamesFromBson(ecoImageNames);
        out["internal_images"] = getImageNamesFromBson(internalImageNames);


        BSONObjBuilder info;
        info.append("$set", builder.obj());
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), info.obj(), false, false);
        return APIE_OK;
    }

    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Seller::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();

    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"seller_id = %s", seller_id.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (cursor->more()) {
            // delete seller from db.user 
            c.remove(DB_SELLER,MONGO_QUERY("seller_id" << seller_id));
            return APIE_OK;
        }

        log(__func__,"seller %s no exist", seller_id.c_str());
        return APIE_SELLER_NO_EXIST;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Seller::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
