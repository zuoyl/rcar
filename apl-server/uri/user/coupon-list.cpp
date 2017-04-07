//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(CouponList, "user/coupon-list");

int CouponList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get user's name
    std::string name = content["name"].asString();
    if (name.empty()) {
        log(__func__, "invalid client request, id is null");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
    
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("uid" << name));
   
        if (!cursor->more()) {
            log("user doesn't exist", name.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        // open fault database and get basic information
        cursor = c.query(DB_COUPON, MONGO_QUERY("user_id" << name));
        int count = 0;
        Json::Value coupon_list;
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            count++;
            Json::Value coupon;
            coupon["title"] = obj.getStringField("title");
            coupon["content"] = obj.getStringField("content");
            coupon["seller_id"] = obj.getStringField("seller_id");
            coupon["start_date"] = obj.getStringField("start_date");
            coupon["end_date"] = obj.getStringField("end_date");
            coupon["status"] = obj.getStringField("status");
            coupon["images"] = getImageNamesFromBson(obj, "images");
            coupon_list.append(coupon);
        }
        out["status"] = APIE_OK;
        out["total"] = count;
        out["data"] = coupon_list;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int CouponList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int CouponList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int CouponList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int CouponList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
