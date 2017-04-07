//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(ActivityList, "seller/activity-list");

int ActivityList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value& content = req.getInObject();
    Json::Value& out = req.getOutObject();
    //std::string name = content["name"].asString();
    // get seller's name
    std::string seller_id = content["seller_id"].asString();
    if (seller_id.empty()) {
        log(__func__,"invalid client request, id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller doesn't exist", seller_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
      
        // open commodity database and get basic information
        int offset = content["offset"].asInt();
        int num = content["num"].asInt();
        int count = 0;
        Json::Value ac_list;
       cursor = c.query(DB_ACTIVITY, MONGO_QUERY("seller_id" << seller_id), num, offset);
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value info;
            info["activity_id"] = obj.getStringField("activity_id");
            info["title"] = obj.getStringField("title");
            info["url"] = obj.getStringField("url");
            info["detail"] = obj.getStringField("detail");
            info["start_date"] = obj.getStringField("start_date");
            info["end_date"] = obj.getStringField("end_date");
            info["images"] = getImageNamesFromBson(obj, "images");
            ac_list.append(info);
            count++;
        }
        
        out["total"] = count;
        out["data"] = ac_list;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int ActivityList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int ActivityList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int ActivityList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int ActivityList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
