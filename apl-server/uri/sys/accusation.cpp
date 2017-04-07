//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Accusation, "sys/accusation");

int Accusation::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Accusation::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Accusation::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    const std::string id = content["id"].asString(); 
    const std::string status = content["status"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ACCUSATION, MONGO_QUERY("id" << id));
        if (cursor->more()) { 
            BSONObj obj = cursor->next();
            // c.update(DB_ACCUSATION, BSON("status" << status));

            // get user and seller
            std::string seller_id = obj.getStringField("seller");
            std::string user_id = obj.getStringField("user");

            // notify the seller and the user
            Json::Value msg;
            msg["source"] = "sys";
            msg["kind"] = "notify";
            msg["title"] = "accusation";
            msg["content"] = "deleted";
            msg["id"] = id;
            pushMessageToUser(user_id, msg);
            pushMessageToSeller(seller_id, msg);
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Accusation::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    const std::string id = content["id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_ACCUSATION, MONGO_QUERY("uuid" << id));
        if (cursor->more()) { 
            BSONObj obj = cursor->next();
            std::string seller_id = obj.getStringField("seller");
            std::string user_id = obj.getStringField("user");
            c.remove(DB_NOTIFY, cursor->next());

            // notify the seller that accusation had been deleted
            Json::Value msg;
            msg["source"] = "sys";
            msg["kind"] = "sys";
            msg["title"] = "accusation";
            msg["content"] = "deleted";
            pushMessageToUser(user_id, msg);
            pushMessageToSeller(seller_id, msg);
        }
        else {
            log(__func__,"invalid notification id");
            return APIE_INVALID_CLIENT_REQ;
        }
        return APIE_OK;

    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Accusation::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
