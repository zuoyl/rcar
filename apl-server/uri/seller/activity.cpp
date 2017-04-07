//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Activity, "seller/activity");

int Activity::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Activity::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string title = content["title"].asString();
    std::string detail = content["detail"].asString();
    std::string start_date = content["start_date"].asString();
    std::string end_date = content["end_date"].asString();
    OID id = OID::gen();

    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s", seller_id.c_str());

    try {
        // connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }
        BSONObj seller = cursor->next();

        BSONObj p = BSON("activity_id" << id.toString() << "seller_id" << seller_id);
        c.insert(DB_ACTIVITY, p);
        // construct object at frist
        BSONArray images = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(images);
        p = BSON("activity_id" << id.toString()
                << "title" << title 
                << "detail" << detail 
                << "start_date" << start_date
                << "end_date" << end_date
                << "images" << images); 


        // get activity subarray

        if (!seller.hasField("activities")) {
            BSONArrayBuilder arrayBuilder;
            arrayBuilder.append(p);

            BSONObjBuilder objBuilder;
            objBuilder.append("activities", arrayBuilder.arr());

            // update
            BSONObjBuilder set;
            set.append("$set", objBuilder.obj());
            c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), set.obj());
        } else {
            BSONElement activities = seller.getField("activities");
            BSONObj obj = activities.Obj();
            // check wether the activity already exist
            for (BSONObj::iterator i = obj.begin();i.more();) {
                BSONObj activity = i.next().Obj();
                BSONElement e = activity["title"]; 
                if (title == e.toString())
                    return APIE_INVALID_CLIENT_REQ;
            }
            // add new activity into list
            BSONObjBuilder objBuilder;
            objBuilder.append("activities", p);
            // update
            BSONObjBuilder push;
            push.append("$push", objBuilder.obj());
            c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), push.obj());
        }
        out["activity_id"] = id.toString();
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Activity::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string activity_id = content["activity_id"].asString();
    std::string title = content["title"].asString();
    std::string detail = content["detail"].asString();
    std::string start_date = content["start_date"].asString();
    std::string end_date = content["end_date"].asString();

    if (role.empty() || seller_id.empty() || activity_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;


    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }

        // remove images at first
        BSONObj seller = cursor->next();
        if (seller.hasField("activities")) {
            BSONObj activities = seller.getObjectField("activities");
            for (BSONObjIterator i = activities.begin(); i.more();) {
                BSONObj activity = i.next().Obj();
                std::string actid = activity.getStringField("activity_id");
                if (actid == activity_id) {
                    removeImages(activity, "images");
                    break;
                }
            }
        }

        // construct object 
        BSONArray images = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(images);
        BSONObj activity = BSON("activity_id" << activity_id 
                << "title" << title 
                << "detail" << detail 
                << "start_date" << start_date
                << "end_date" << end_date
                << "images" << images);

        // delete activity at first 
        BSONObj pull = BSON("$pull" << BSON("activities" << BSON("activity_id" << activity_id)));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), pull);

        // insert new activity into activity set 
        BSONObj push = BSON("$push" << BSON("activites" << activity));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), push);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Activity::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string activity_id = content["activity_id"].asString();

    if (role.empty() || seller_id.empty() || activity_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s, activity = %s", seller_id.c_str(), activity_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller %s doesn't exist", seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }

        // delete activity
        BSONObj p = BSON("$pull" << BSON("activities" << BSON("activity_id" << activity_id)));
        c.update(DB_SELLER, MONGO_QUERY("seller_id" << seller_id), p, true, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Activity::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
