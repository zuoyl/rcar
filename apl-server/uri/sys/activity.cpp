//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Activity, "sys/activity");

int Activity::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Activity::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    const std::string seller_id = content["seller_id"].asString();
    const std::string activity_id = create_act_id();
    const std::string desc = content["desc"].asString();
    const std::string title = content["title"].asString();
    const std::string url = content["url"].asString();
    const std::string start_date = content["start_date"].asString();
    const std::string end_date = content["end_date"].asString();

    if (url.empty()) {
        log(__func__, " invalid parameter url:%s", url.c_str());
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // update act status into publicated
        BSONObj activity = BSON(GENOID << 
                "seller_id" << seller_id <<
                "activity_id" << activity_id <<
                "title" << title <<
                "desc" << desc <<
                "url" << url <<
                "start_date" << start_date <<
                "end_date" << end_date <<
                "status" << "publicated"
                );
        c.insert(DB_ACTIVITY, activity);
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Activity::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();

    std::string act_id = content["act_id"].asString();
    std::string status = content["status"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //first. query wether the act_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ACTIVITY, MONGO_QUERY("act_id" << act_id));
        if (!cursor->more()) {
            log("act_id doesn't exist", act_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        // update record
        BSONObj p = BSON("$set" <<
                BSON("status" << status)
                );
        c.update(DB_ACTIVITY, MONGO_QUERY("act_id" << act_id), p, false, false);
#if 0
        if (status == "refused") {
            std::vector<std::string> uuid;
            get_seller_uuid(c, act_id, uuid);

            // notify sellers
    	    Json::Value msg;
	        msg["kind"] = "ads_refused";
	        msg["content"] = act_id;
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

int Activity::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    std::string act_id = content["act_id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //first. query wether the act_id exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ACTIVITY, MONGO_QUERY("act_id" << act_id));
        if (!cursor->more()) {
            log(__func__,"act_id  %s doesn't exist", act_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }
        // update record
        BSONObj p = BSON("$set" <<
                BSON("status" << "deleted")
                );
        c.update(DB_ACTIVITY, MONGO_QUERY("act_id" << act_id), p, false, false);
#if 0
        if (status == "refused") {
            std::vector<std::string> uuid;
            get_seller_uuid(c, act_id, uuid);

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

int Activity::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
