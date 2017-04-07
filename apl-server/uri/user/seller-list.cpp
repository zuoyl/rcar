//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"
#include "query-helper.h"

namespace user {

DECL_RESOURCE_CLASS(SellerList, "user/seller-list");

int getSellerListForService(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string city = content["city"].asString();	
    std::string serviceType = content["type"].asString();
    int offset = content["offset"].asInt();
    int number = content["number"].asInt();
    double lng = content["lng"].asDouble();
    double lat = content["lat"].asDouble();

    int count = 0;

    if (role.empty() || city.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

   // log(__func__,"role = %s", role.c_str());
    try {
        //connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        Query query = createQueryConditionForService(content, serviceType);
        Json::Value sellerList;
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_SELLER, query, number, offset);

        while (cursor->more()) {
            count++;
            //get seller_id
            BSONObj obj = cursor->next();
            std::string seller_id = obj.getStringField("seller_id");
            Json::Value seller;
            // get seller info
            seller["seller_id"] = obj.getStringField("seller_id");
            seller["type"] = obj.getStringField("seller_type");
            seller["name"] = obj.getStringField("name");
            seller["telephone"] = obj.getStringField("telephone");
            seller["address"] = obj.getStringField("addr");
            seller["intro"] = obj.getStringField("intro");
            seller["images"] = getImageNamesFromBson(obj, "images");
            // how to deal with;
            seller["rate"] = "100%";
    
            BSONObj location = obj.getObjectField("location");
            Json::Value loc;
            loc["lat"] = location[0].Double();
            loc["lng"] = location[1].Double();
            seller["location"] = loc;
            seller["distance"] = get_distance(lat, lng, location[0].Double(), location[1].Double());


            // get specified service
            BSONObj services = obj.getObjectField("services");
            Json::Value serviceList;
            for (BSONObj::iterator i = services.begin(); i.more(); ) {
                BSONObj s = i.next().Obj();
                std::string type = s.getStringField("type");
                if (serviceType == type) {
                    Json::Value info;
                    info["service_id"] = s.getStringField("service_id");
                    info["title"] = s.getStringField("title");
                    info["url"] = s.getStringField("url");
                    info["desc"] = s.getStringField("desc");
                    info["price"] = s.getStringField("price");
                    info["images"] = getImageNamesFromBson(s, "images");
                    info["type"] = type;
                    serviceList.append(info);
                }
            }
            seller["services"] = serviceList;
            sellerList.append(seller);
        }
        out["total"] = count;
        out["data"] = sellerList;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        //log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

int getSellerListForActivity(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string city = content["city"].asString();	
    int offset = content["offset"].asInt();
    int number = content["number"].asInt();
    double lng = content["lng"].asDouble();
    double lat = content["lat"].asDouble();

    int count = 0;

    if (role.empty() || city.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    Log::ll_info("%s, role = %s", __func__, role.c_str());
    try {
        //connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        Query query = createQueryConditionForAllActivity(content);
        Json::Value sellerList;
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_SELLER, query, number, offset);

        while (cursor->more()) {
            //get seller_id
            BSONObj obj = cursor->next();
            if (obj.hasField("activities")) {
                count++;
                // get specified service
                BSONObj activities = obj.getObjectField("activities");
                Json::Value activityList;
                for (BSONObj::iterator i = activities.begin(); i.more(); ) {
                    BSONObj s = i.next().Obj();
                    Json::Value info;
                    info["activity_id"] = s.getStringField("activity_id");
                    info["title"] = s.getStringField("title");
                    info["detail"] = s.getStringField("detail");
                    info["start_date"] = s.getStringField("start_date");
                    info["end_date"] = s.getStringField("end_date");
                    
                    // get users count who take party in the activity
                    std::string activity_id = s.getStringField("activity_id");
                    std::unique_ptr<mongo::DBClientCursor> pcursor =
                            c.query(DB_ACTIVITY, MONGO_QUERY("activity_id" << activity_id)); 
                    if (pcursor->more()) {
                        BSONObj activity = pcursor->next();
                        int total = activity.getIntField("total");
                        info["total_user"] = total;
                    } else  {
                        info["total_user"] = 0;
                    }


                    activityList.append(info);
                }

                if (!activityList.empty()) {
                    std::string seller_id = obj.getStringField("seller_id");
                    Json::Value seller;
                    // get seller info
                    seller["seller_id"] = obj.getStringField("seller_id");
                    seller["type"] = obj.getStringField("seller_type");
                    seller["name"] = obj.getStringField("name");
                    seller["telephone"] = obj.getStringField("telephone");
                    seller["address"] = obj.getStringField("addr");
                    seller["intro"] = obj.getStringField("intro");
                    seller["images"] = getImageNamesFromBson(obj, "images");
                    // how to deal with;
                    seller["rate"] = "100%";
            
                    BSONObj location = obj.getObjectField("location");
                    Json::Value loc;
                    loc["lat"] = location[0].Double();
                    loc["lng"] = location[1].Double();
                    seller["location"] = loc;
                    seller["distance"] = get_distance(lat, lng, location[0].Double(), location[1].Double());
                
                    seller["activities"] = activityList;
                    sellerList.append(seller);
                }
            }
        }
        out["total"] = count;
        out["data"] = sellerList;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        Log::error(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}


int SellerList::Get(const ResourceContext &context, Request &req) {
    const Json::Value in = req.getInObject();
    std::string scope = in["scope"].asString();
    if (scope == "service")
        return getSellerListForService(context, req);
    else
        return getSellerListForActivity(context, req);
}

int SellerList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
