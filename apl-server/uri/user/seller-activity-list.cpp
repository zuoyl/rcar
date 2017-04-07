//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerActivityList, "user/seller-activity-list");

int SellerActivityList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get seller's name
    std::string seller_id = content["seller_id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // open commodity database and get basic information
        int offset = content["offset"].asInt();
        int num = content["num"].asInt();
        int count = 0;
        Json::Value activities;

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), num, offset);
        while (cursor->more()) {
            BSONObj seller = cursor->next();
            if (seller.hasField("activities")) {
                BSONObj p = seller.getObjectField("activities");
                for (BSONObj::iterator i = p.begin(); i.more();) {
                    BSONObj activity = i.next().Obj();
                    Json::Value info;
                    info["activity_id"] = activity.getStringField("activity_id");
                    info["title"] = activity.getStringField("title");
                    info["url"] = activity.getStringField("url");
                    info["desc"] = activity.getStringField("desc");
                    info["start_date"] = activity.getStringField("start_date");
                    info["end_date"] = activity.getStringField("end_date");
                    info["images"] = getImageNamesFromBson(activity, "images");
     
                    activities.append(info);
                    count++;
                }
            }
        }
        out["total"] = count;
        out["data"] = activities;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int SellerActivityList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerActivityList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerActivityList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerActivityList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
