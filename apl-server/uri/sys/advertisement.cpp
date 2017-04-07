//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace sys {

DECL_RESOURCE_CLASS(Advertisement, "sys/advertisement");

static void getSellerUUIDs(
        mongo::DBClientConnection &c,
        std::string &ads_id,
        std::vector<std::string> &uuid)
{
    std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ADS, MONGO_QUERY("ads_id" << ads_id));
    if (cursor->more()) {
        BSONObj obj = cursor->next();
        std::string info = obj.getStringField("seller");
        uuid.push_back(info);
    }
}

int Advertisement::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    const std::string ads_id = content["ads_id"].asString();

    if (ads_id.empty()) {
        log(__func__, " invalid parameter ads_id:%s",ads_id.c_str());
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ADS, MONGO_QUERY("ads_id" << ads_id));
        if (!cursor->more()) {
            log(__fun__, " invalid parameter ads_id:%s", ads_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        BSONObj p = cursor->next();

        out["appli_time"] = p.getStringField("appli_time");
        out["seller"] = p.getStringField("seller");
        out["start_time"] = p.getStringField("start_time");
        out["end_time"] = p.getStringField("end_time");
        out["text"] = p.getStringField("text");
        out["condition"] = p.getStringField("condition");
        BSONObj images_bson = p.getObjectField("images_list");

        Json::Value image_list;
        for (int i = 0; i < images_bson.nFields(); i++) {
            BSONObj img_bson = images_bson[i].Obj();
            Json::Value img;
            img["name"] = img_bson.getStringField("name");
            img["data"] = img_bson.getStringField("data");
            image_list.append(img);
        }
        out["image_list"] = image_list;
        
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Advertisement::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    const std::string ads_id = content["ads_id"].asString();

    if (ads_id.empty()) {
        log(__func__, "invalid parameter ads_id:%s", ads_id.c_str());
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ADS, MONGO_QUERY("ads_id" << ads_id));
        if (!cursor->more()) {
            log(__func__, "invalid parameter ads_id:%s", ads_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        // update ads status into publicated
        BSONObj ads_status = BSON("$set" <<
                BSON("status" << "publicated")
                );
        c.update(DB_ADS, MONGO_QUERY("ads_id" << ads_id), ads_status, false, false);

        //notify users         
        BSONObj p = cursor->next();

        const std::string start_time = p.getStringField("start_time");
        const std::string end_time = p.getStringField("end_time");
        Json::Value condition = p.getStringField("condition"); 
        
        // how to deal with start_time & end_time

        // get user's uuids
        // std::vector<std::string> uuids;
        // get_users_uuid(c, condition, uuids);
        
        // notify all users online
        Json::Value msg;
        msg["source"] = "sys";
        msg["kind"] = "publicate_ads";
        msg["content"] = ads_id;
        pushMessageToUsers(uuids, msg);
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Advertisement::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();

    std::string ads_id = content["ads_id"].asString();
    std::string status = content["status"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //first. query wether the ads_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ADS, MONGO_QUERY("ads_id" << ads_id));
        if (!cursor->more()) {
            log("ads_id doesn't exist", ads_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        // update record
        BSONObj p = BSON("$set" << BSON("status" << status));
        c.update(DB_ADS, MONGO_QUERY("ads_id" << ads_id), p, false, false);

        if (status == "refused") {
            std::vector<std::string> ids;
            getSellerUUIDs(c, ads_id, ids);

            // notify sellers
    	    Json::Value msg;
            msg["source"] = "sys";
	        msg["kind"] = "ads_refused";
	        msg["content"] = ads_id;
            pushMessageToSellers(ids, msg);
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Advertisement::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    std::string ads_id = content["ads_id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //first. query wether the ads_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ADS, MONGO_QUERY("ads_id" << ads_id));
        if (!cursor->more()) {
            log(__func__,"ads_id %s doesn't exist", ads_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        // update record
        BSONObj p = BSON("$set" <<
                BSON("status" << "deleted")
                );
        c.update(DB_ADS, MONGO_QUERY("ads_id" << ads_id), p, false, false);
#if 0
        if (status == "refused") {
            std::vector<std::string> uuid;
            get_seller_uuid(c, ads_id, uuid);

        }
#endif
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Advertisement::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
