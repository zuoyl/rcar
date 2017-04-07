//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(UserMessage, "seller/user-message");

int UserMessage::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string msg_id = val["msg_id"].asString(); 

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || msg_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // check wether user exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            return APIE_SELLER_NO_EXIST;
        }

        cursor = c.query(DB_MSG, MONGO_QUERY("msg_id" << msg_id << "to" << seller_id));
        if (cursor->more()) {
            BSONObj msg = cursor->next();
            out["msg_id"] = msg.getStringField("msg_id");
            out["type"] = msg.getStringField("type");
            out["source"] = msg.getStringField("source");
            out["from"] = msg.getStringField("from");
            out["content"] = msg.getStringField("content");
            out["time"] = msg.getStringField("time");
        } else {
           return APIE_USER_NO_EXIST;  // temp
        }

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int UserMessage::Post(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string user_id = val["user_id"].asString();
    std::string content = val["msg"].asString();
    std::string time = get_customstring_time();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || content.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // check wether seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            return APIE_SELLER_NO_EXIST;
        }
        // check wether user exist
        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            return APIE_USER_NO_EXIST;
        }
        OID id = OID::gen();
        // construct msg
        BSONObj p = BSON("msg_id" << id.toString()
                       << "group_id" << ""
                       << "source" << "seller"
                       << "kind" << "seller"
                       << "seller_id" << seller_id
                       << "targets" << BSON_ARRAY(user_id)
                       << "content" << content
                       << "time" << time);

        c.insert(DB_MSG, p);

        // notify user
        Json::Value msg;
        msg["source"] = "seller";
        msg["kind"] = "msg";
        msg["msg_id"] = id.toString();
        msg["title"] = content;
        pushMessageToUser(user_id, msg);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int UserMessage::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int UserMessage::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int UserMessage::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
