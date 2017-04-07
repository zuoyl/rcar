//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Advertisement, "seller/advertisement");

int Advertisement::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get user's name
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();	
    int num = content["num"].asInt();
    int offset = content["offset"].asInt();

    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
    

        Json::Value adsList;
        // open fault database and get basic information
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ADS, MONGO_QUERY("seller_id" << seller_id), num, offset);
        while(cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value advertisement;
            advertisement["ads_id"] = obj.getStringField("ads_id");
            advertisement["type"] = obj.getStringField("type");
            advertisement["title"] = obj.getStringField("title");
            advertisement["detail"] = obj.getStringField("content");
            advertisement["start_time"] = obj.getStringField("start_date");
            advertisement["end_time"] = obj.getStringField("end_date");
            advertisement["url"] = obj.getStringField("url");
            advertisement["status"] = obj.getStringField("status");
            advertisement["images"] = getImageNamesFromBson(obj, "images");
            advertisement["city"] = obj.getStringField("city");
            adsList.append(advertisement);
        }
        out["data"] = adsList;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Advertisement::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string type = content["type"].asString();
    std::string title = content["title"].asString();
    std::string start_date = content["start_date"].asString();
    std::string end_date = content["end_date"].asString();
    std::string url = content["url"].asString();
    std::string city = content["city"].asString();
    OID id = OID::gen();

    if (role.empty() || seller_id.empty() || type.empty() 
        || title.empty() || start_date.empty() || end_date.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s", seller_id.c_str());

    try {
        // connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }
        // check wether the sampe title exist
        cursor = 
            c.query(DB_ADS, MONGO_QUERY("seller_id" << seller_id << "title" << title));
        if (cursor->more()) {
            log("same advertisement title %s for seller %s exist", 
                    title.c_str(), seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }
        // construct images
        BSONArray images = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(images);;


        // construct object at frist
        BSONObjBuilder objBuilder;
        BSONObj p = BSON("seller_id" << seller_id 
                << "ads_id" << id.toString() 
                << "type" << type
                << "title" << title 
                << "start_date" << start_date
                << "end_date" << end_date
                << "url" << url
                << "status" << "wait"
                << "city" << city
                << "images" << images ); 

        c.insert(DB_ADS, p);

        // TODO: notify system manager
        out["ads_id"] = id.toString();
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "error exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Advertisement::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string ads_id = content["ads_id"].asString();

    if (role.empty() || seller_id.empty() || ads_id.empty())
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
            c.query(DB_ADS, MONGO_QUERY("seller_id" << seller_id << "ads_id" << ads_id));
        if (!cursor->more()) {
            log("advertisement %s for seller %s doesn't exist", 
                    ads_id.c_str(), seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }

        //check password
        BSONObj seller = cursor->next();
        BSONObj recs = BSON("status" << "canceled");
        BSONObj condition = BSON("seller_id" << seller_id << "ads_id" << ads_id);
        c.update(DB_SELLER, condition, BSON("$set" << recs));

        // TODO:notify system manager

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Advertisement::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string ads_id = content["ads_id"].asString();

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
            c.query(DB_ADS, MONGO_QUERY("seller_id" << seller_id << "ads_id" << ads_id));
        if (!cursor->more()) {
            log("advertisement %s for seller %s doesn't exist", ads_id.c_str(), seller_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        BSONObj advertisement = cursor->next();
        std::string status = advertisement.getStringField("status");
        if (status != "doing")
            c.remove(DB_ADS, MONGO_QUERY("seller_id" << seller_id << "ads_id" << ads_id));
        else 
            return APIE_INVALID_CLIENT_REQ;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Advertisement::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
