//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "order.h"

namespace user {

DECL_RESOURCE_CLASS(SOS, "user/sos");

int get_sellers_uuid(
        mongo::DBClientConnection &c,
        std::string city,
        Json::Value &location,
        int distance,
        std::vector<std::string> &uuids,
        int &count)
{
    // check parameter
    if (location.empty() || city.empty())
        return APIE_INVALID_CLIENT_REQ;
       
    double lng = location["lng"].asDouble();
    double lat = location["lat"].asDouble();
    count = 0; 	   
    Query _query; 
    
    // get seller's info where in condition
    BSONObjBuilder appendRegex;	
    appendRegex.append("city",city);
    appendRegex.append("status","open");
    appendRegex.append("location",BSON("$near" <<
                BSON_ARRAY(lng << lat)<<
                "$maxDistance" << (distance/6378)));
    _query.obj = appendRegex.obj();
    
    std::unique_ptr<mongo::DBClientCursor> ret_cursor = 
        c.query(DB_SELLER, _query);
        
    while (ret_cursor->more()) {
        BSONObj p = ret_cursor->next();
        std::string uuid = p.getStringField("seller_id");
        uuids.push_back(uuid);
        count++;
    }
	
    return APIE_OK;
}
int SOS::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int SOS::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    Json::Value location = content["location"];
    std::string city = content["city"].asString();
    std::string time = get_customstring_time();
    std::string order_id = OID::gen().toString();
    int distance = 10*1000; //default is 10km

    if (role.empty() || user_id.empty() || location.empty() || city.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        // connect DB		
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s does not exist", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        BSONObj user = cursor->next();
        std::string userName = user.getStringField("name");

        // get seller's uuid
        int count;
        std::vector<std::string> uuids;
        get_sellers_uuid(c, city, location, distance, uuids, count); 
        if (count < 1) {
            return APIE_NO_SELLER_FIND;
        }

        // insert sos report to db.sos
        BSONObjBuilder builder;
        builder.append("role", "user");
        builder.append("order_id", OID::gen().toString());
        builder.append("user_id", user_id);
        builder.append("user_name", userName);
        builder.append("time", time);
        builder.append("city", city);
        builder.append("status", "new");
        builder.append("service_type", OrderServiceTypeSOS);
        if (!location.empty()) {
            BSONObjBuilder b;
            b.append("lng", location["lng"].asDouble());
            b.append("lat", location["lat"].asDouble());
            builder.append("location", b.obj());
        }
        std::vector<std::string>::iterator ite = uuids.begin();
        BSONArrayBuilder sellers_builder;
        for (; ite != uuids.end(); ite++) {
            sellers_builder.append(*ite);
        }
        builder.append("sellers",sellers_builder.arr());			
        c.insert(DB_SOS, builder.obj());

        // notify all seller who saitisified the conditon
        Json::Value msg;
        msg["source"] = "user";
        msg["kind"] = "order";
        msg["type"] = "sos";
        pushMessageToSellers(uuids, msg);
        out["order_id"] = order_id;
        out["count"] = (int)uuids.size();
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int SOS::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SOS::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SOS::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
