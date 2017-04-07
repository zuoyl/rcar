//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(AdvertisementList, "sys/advertisement-list");

int AdvertisementList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        int offset = content["offset"].asInt();
        int num = content["num"].asInt();
        // open ADS database and get basic information
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ADS, MONGO_QUERY("status" << "wait"), num, offset);
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value info = obj.getStringField("ads_id");
            out["ads_list"].append(info);
        }

        out["total"] = (int)c.count(DB_STORE, MONGO_QUERY("status" << "wait"));
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int AdvertisementList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int AdvertisementList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int AdvertisementList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int AdvertisementList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
