//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(CarMaintenanceInfo, "user/car-maintenance-info");

int CarMaintenanceInfo::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's detail information and check
    std::string role = content["role"].asString();
    std::string kind = content["kind"].asString();
    std::string brand = content["brand"].asString();
    int mileage = content["mileage"].asInt();

    if (role.empty() || brand.empty() || kind.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"role = %s", role.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        Query query =  MONGO_QUERY("kind" << kind << "kinds.name" << brand);
        std::unique_ptr<mongo::DBClientCursor> cursor = c.query(DB_CAR, query);
        if (!cursor->more()) {
            log(__func__,"%s, car info doesn't exist");
            return APIE_USER_NO_EXIST;
        }
        //get car information   
        BSONObj p = cursor->next();
        BSONObj kinds = p.getObjectField("kinds");
        for (BSONObjIterator i = kinds.begin(); i.more();) {
            BSONObj kind = i.next().Obj();
            std::string name = kind.getStringField("name");

            if (name == brand) {
                BSONObj items = kind.getObjectField("items");
                Json::Value v(Json::arrayValue);
                for (BSONObjIterator k = items.begin(); k.more(); ) {
                    std::string val = k.next().str();
                    v.append(val);
                }
                out["user"]  = v;

                BSONObj mileages = kind.getObjectField("mileages");
                for (BSONObjIterator j = mileages.begin(); j.more();) {
                    BSONObj item = j.next().Obj();
                    int n = item.getIntField("mileage");
                    if (n >= mileage) { // found 
                        // construct items 
                        Json::Value v(Json::arrayValue);
                        BSONObj pp = item.getObjectField("items");
                        for (BSONObjIterator k = pp.begin(); k.more(); ) {
                            BSONObj t = k.next().Obj();
                            v.append(rcar::tojson(t));
                        }
                        out["mileage"] = v;

                       break;
                    }
                }
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
int CarMaintenanceInfo::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int CarMaintenanceInfo::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int CarMaintenanceInfo::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int CarMaintenanceInfo::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
