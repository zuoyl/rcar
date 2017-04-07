//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Recommendations, "seller/recommendations");

int Recommendations::Get(const ResourceContext &context, Request &req) {
    // get in and out json object
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    int page = content["page"].asInt();
    int page_size = content["page_size"].asInt();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
		
        int count = 0;
        Json::Value objs;     
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> seller_cursor =
            c.query(DB_SELLER);
		
        while (seller_cursor->more() && (count < page_size)) {
            BSONObj obj = seller_cursor->next();
            Json::Value recommendations;
            recommendations["type"] = "seller";
            recommendations["seller_id"] = obj.getStringField("seller_id");
            recommendations["title"] = obj.getStringField("name");
            recommendations["summary"] = obj.getStringField("introduction");
            recommendations["image"] = obj.getStringField("image");
            recommendations["date"] = obj.getStringField("addr");
            objs.append(recommendations);
            count++;
        }
#if 0
        std::unique_ptr<mongo::DBClientCursor> ads_cursor = 
            c.query(DB_ADS, MONGO_QUERY("status" << "wait"));
        while (ads_cursor->more() && (count < page_size)) {
            BSONObj obj = ads_cursor->next();
            Json::Value recommendations;
            recommendations["type"] = "ads";
            recommendations["ads_id"] = ads.getStringField("ads_id");
            recommendations["link_type"] = ads.getStringField("type");
            recommendations["link"] = ads.getStringField("link");
            recommendations["title"] = ads.getStringField("title");
            recommendations["summary"] = ads.getStringField("content");
            recommendations["image"] = ads.getStringField("image");
            recommendations["data"] = ads.getStringField("start_time");
            
            objs.append(recommendations);
            count++;
        }
        
        std::unique_ptr<mongo::DBClientCursor> commodity_cursor = 
            c.query(DB_STORE);
        while (commodity_cursor->more() && (count < page_size)) {
            
            BSONObj obj = commodity_cursor->next();
            Json::Value recommendations;
            recommendations["type"] = "commodity";
            recommendations["commod_id"] = obj.getStringField("commod_id");
            recommendations["title"] = obj.getStringField("name");
            recommendations["summary"] = obj.getStringField("price");
            recommendations["image"] = obj.getStringField("image");
            recommendations["data"] = obj.getStringField("brand");
            
            objs.append(recommendations);
            count++;
        }
#endif		
        out["total"] = count;
        out["data"] = objs;
		
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
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
