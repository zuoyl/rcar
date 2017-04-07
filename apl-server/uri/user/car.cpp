//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Car, "user/car");

int Car::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's detail information and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string platenumber = content["platenumber"].asString();

    if (role.empty() || user_id.empty() || platenumber.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s, role = %s, platenumber = %s",
            user_id.c_str(), role.c_str(), platenumber.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        else {
            //get car information   
            BSONObj p = cursor->next();
            BSONObj cars = p.getObjectField("cars");
            int i = 0;		
            for (i = 0; i < cars.nFields(); i++) {
                BSONObj car = cars[i].Obj();	
                std::string platenumber = car.getStringField("platenumber");
                if (platenumber == car.getStringField("platenumber")) {
                    out["platenumber"] = car.getStringField("platenumber");
                    out["brand"] = car.getStringField("brand");
                    out["buy_date"] = car.getStringField("buy_date");
                    out["miles"] = car.getStringField("miles");	
                    out["insurance"] = car.getStringField("insurance");
                    //get_images(USR_IMAGE_PATH+user_id+"/cars"+"/"+platenumber, out);
                    break;
                }
            }

            if (i >= cars.nFields()) {
                return APIE_CAR_NO_EXIST;
            }
        }	
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int Car::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Car::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Car::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Car::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
