//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Seller, "sys/seller");

int Seller::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    std::string role = content["role"].asString();
    std::string city = content["city"].asString();
    std::string status = content["status"].asString();

    if (role != "sys")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        BSONObjBuilder find;
	  
        if (!city.empty())
            find.append("city", city);

        if (!city.empty())
            find.append("status", status);

        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, Query(find.obj()));
        Json::Value sellers;
        int count = 0;
        while (cursor->more()) {
            BSONObj seller_obj = cursor->next();
            Json::Value seller;
            seller["seller_id"] = seller_obj.getStringField("seller_id");
            seller["name"] = seller_obj.getStringField("name");
            sellers.append(seller);
            count++;
        }
        out["count"] = count;
        out["sellers"] = sellers;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Seller::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Seller::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string status = content["status"].asString();

    if (role != "sys")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //first. query wether the fault_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller doesn't exist", seller_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        // update record
        BSONObj p = BSON("$set" << BSON("status" << status));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), p, false, false);
#if 0
        /* get seller's uuid.
           notify all sellers ???
           or notify sellers in 3km ???
         */
        Json::Value location;
        Json::Value condition;
        std::vector<std::string> uuids;
        get_sellers_uuid(c, location, condition, uuids);

        // notify sellers
        Json::Value msg;
        msg["kind"] = "fault_status";
#endif
        return APIE_OK;    
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Seller::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Seller::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
