//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Profile, "user/profile");

int Profile::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &user = req.getOutObject();

    // get valid user's uuid 
    std::string user_id = content["user_id"].asString();
    std::string role = content["role"].asString();
    if (user_id.empty()) {
        log(__func__, "user's user_id is empty");
        return APIE_INVALID_CLIENT_REQ;
    }

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // check wethere the user is registered
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("role" << role << "user_id" << user_id));
        if (!cursor->more()) {
            log("the use %s is not in session", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (cursor->more()) {
            BSONObj p = cursor->next();
            BSONObj uinfo = p.getObjectField("userinfo");            
            user["user_name"] = uinfo.getStringField("user_name");
            user["images"] = getImageNamesFromBson(p, "images");
//            user["fault_count"] = convertIntToString((int)c.count(DB_FAULT, MONGO_QUERY("user_id" << user_id)));
            user["credits"] = "0";
            user["coupons"] = "0";

            BSONObj cars = p.getObjectField("cars");
            for (int index = 0; index < cars.nFields(); index++) {
                BSONObj car_obj = cars[index].Obj();
                //user["car_name"] = car_obj.getStringField("name");
                user["insurance"] = car_obj.getStringField("insurance");
                
                //dummy
                user["insurance_url"] = "www.rcar.com";
                break;
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
int Profile::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Profile::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Profile::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Profile::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
