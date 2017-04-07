//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Seller, "user/seller");

int getSellerDetailInfo(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &seller = req.getOutObject();

    // get valid seller's uuid 
    std::string seller_id = content["seller_id"].asString();
    std::string role = content["role"].asString();

    if (role != "user" || seller_id.empty()) {
        //error(__func__, "seller_id is empty");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) 
            return APIE_SELLER_NO_EXIST;

        BSONObj p = cursor->next();
        // basic seller info
        seller["seller_id"] = p.getStringField("seller_id");			
        seller["name"] = p.getStringField("name");
        seller["uuid"] = p.getStringField("uuid");
        seller["telephone"] = p.getStringField("telephone");
        seller["address"] = p.getStringField("addr");
        seller["intro"] = p.getStringField("intro");

        Json::Value images = getImageNamesFromBson(p, "face_images");;
        seller["images"] = images.append(getImageNamesFromBson(p, "internal_images"));
#if 0
        BSONObj loc = p.getField("location").Obj();
        Json::Value location;
        location["lat"] = loc.getStringField("lat");
        location["lng"] = loc.getStringField("lng");
        seller["location"] = location;
#endif

        Json::Value commodities;
        std::unique_ptr<mongo::DBClientCursor> com_cursor = 
            c.query(DB_STORE, MONGO_QUERY("seller_id" << seller_id));
        while (com_cursor->more()) {
            BSONObj obj = com_cursor->next();
            Json::Value info;
            info["seller_id"] = obj.getStringField("seller_id");	
            info["commod_id"] = obj.getStringField("commod_id");	 
            info["name"] = obj.getStringField("name");
            info["desc"] = obj.getStringField("desc");		 
            info["price"] = obj.getStringField("price");
            info["brand"] = obj.getStringField("brand");		 
            info["images"] = getImageNamesFromBson(obj, "images");
            // how to deal with;
            info["rate"] = "100%";
            commodities.append(info);
        }
        seller["commodities"] = commodities;

        // activities
        Json::Value activities;
        if (p.hasField("activities")) {
            BSONObj activityList = p.getObjectField("activities");
            for (BSONObj::iterator i = activityList.begin(); i.more();) {
                BSONObj activity = i.next().Obj();
                Json::Value info;
                info["seller_id"] = activity.getStringField("seller_id");
                info["activity_id"] = activity.getStringField("activity_id");
                info["url"] = activity.getStringField("url");
                info["title"] = activity.getStringField("title");		 
                info["desc"] = activity.getStringField("desc");
                info["start_date"] = activity.getStringField("start_date");
                info["end_date"] = activity.getStringField("end_date");
                info["images"] = getImageNamesFromBson(activity, "images");
                activities.append(info);
            }
        }
        seller["activities"] = activities;

        // services
        Json::Value services;
        if (p.hasField("services")) {
            BSONObj serviceList = p.getObjectField("services");
            for (BSONObj::iterator i = serviceList.begin(); i.more();) {
                BSONObj service = i.next().Obj();
                Json::Value info;
                info["seller_id"] = service.getStringField("seller_id");
                info["service_id"] = service.getStringField("service_id");	 
                info["url"] = service.getStringField("url");
                info["title"] = service.getStringField("title");		 
                info["desc"] = service.getStringField("desc");
                info["type"] = service.getStringField("type");
                info["price"] = service.getStringField("price");
                //get_images(SERVICE_IMAGE_PATH  + info["service_id"].asString(), info);
                services.append(info);
            }
        }
        seller["services"] = services;
        seller["deals"] = 0;
#if 0
        // get image list
        BSONObj images_obj = p.getObjectField("images");
        Json::Value images;
        for (int index = 0; index < images_obj.nFields(); index++) {
            BSONObj img_obj = images_obj[index].Obj();
            Json::Value image;
            image["name"] = img_obj.getStringField("name");
            image["data"] = img_obj.getStringField("data");
            images.append(image);
        }
        seller["images"] = images;
#endif			
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
//        error(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
 
    return APIE_OK;


}
int getSellerInfo(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get valid seller's uuid 
    std::string seller_id = content["seller_id"].asString();
    if (seller_id.empty()) {
//        log(__func__,"%s, seller_id is empty");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (cursor->more()) {
            BSONObj p = cursor->next();
            out["seller_id"] = p.getStringField("seller_id");			
            out["name"] = p.getStringField("name");
            out["telephone"] = p.getStringField("telephone");
            out["address"] = p.getStringField("addr");
            out["intro"] = p.getStringField("intro");
            out["rate"] = p.getStringField("rate");
            out["images"] = getImageNamesFromBson(p, "images");

            // location
            BSONObj loc = p.getObjectField("location");
            out["lat"] = loc[0].Double();
            out["lng"] = loc[1].Double();

            // if user's location is specified, compute distance
            Json::Value location = content["location"];
            if (!location.empty()) {
                double lat = location["lat"].asDouble();
                double lng = location["lng"].asDouble();
                out["distance"] = get_distance(lat, lng, loc[0].Double(), loc[1].Double());
            }

#if 0
            // get image list
            BSONObj images_obj = p.getObjectField("images");
            Json::Value images;
            for (int index = 0; index < images_obj.nFields(); index++) {
                BSONObj img_obj = images_obj[index].Obj();
                Json::Value image;
                image["name"] = img_obj.getStringField("name");
                image["data"] = img_obj.getStringField("data");
                images.append(image);
            }
            seller["images"] = images;
#endif			
            return APIE_OK;
        }
        else {
            return APIE_SELLER_NO_EXIST;
        }
    }
    catch (const mongo::DBException &e) {
      //  log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
    return APIE_OK;

}
int Seller::Get(const ResourceContext &context, Request &req) {
    const Json::Value &in = req.getInObject();
    const std::string detail = in["detail"].asString();
    if (detail.empty())
        return getSellerInfo(context, req);
    else
        return getSellerDetailInfo(context, req);
}

int Seller::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Seller::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Seller::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Seller::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
