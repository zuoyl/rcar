//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Advertisement, "user/advertisement");

int Advertisement::Get(const ResourceContext &context, Request &req) {
    // get in and out json object 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string city = content["city"].asString();

    try{
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int count = 0;
        Json::Value ads_objs; 
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ADS, MONGO_QUERY("status" << "publicated"));             
        while (cursor->more()) {
            BSONObj ads = cursor->next();
            Json::Value advertisement;
            std::string ads_id = ads.getStringField("ads_id");
            advertisement["ads_id"] = ads_id;
            advertisement["seller_id"] = ads.getStringField("seller_id");
            advertisement["type"] = ads.getStringField("type");
            advertisement["link"] = ads.getStringField("link");
            advertisement["start_time"] = ads.getStringField("start_time");
            advertisement["end_time"] = ads.getStringField("end_time");
            advertisement["title"] = ads.getStringField("content");
            advertisement["images"] = getImageNamesFromBson(ads,"images");
            ads_objs.append(advertisement);
            count++;
        }
        out["total"] = count;
        out["data"] = ads_objs;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Advertisement::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Advertisement::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Advertisement::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Advertisement::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
