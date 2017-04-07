//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Recommendations, "user/recommendations");

int Recommendations::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string city = content["city"].asString();
    int offset = content["offset"].asInt();
    int num = content["num"].asInt();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
		
        int count = 0;
        Json::Value objs;     
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, Query(), num, offset);
        while (cursor->more() && (count < num)) {
            BSONObj obj = cursor->next();
            Json::Value rec;
            std::string seller_id = obj.getStringField("seller_id");
            rec["type"] = "seller";
            rec["seller_id"] = seller_id;
            rec["name"] = obj.getStringField("name");
            rec["intro"] = obj.getStringField("intro");
            rec["addr"] = obj.getStringField("address");
            rec["images"] = getImageNamesFromBson(obj, "face_images");
            objs.append(rec);
            count++;
        }
        out["total"] = count;
        out["data"] = objs;
		
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Recommendations::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Recommendations::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Recommendations::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Recommendations::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
