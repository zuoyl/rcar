//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(MessageList, "user/message-list");

int MessageList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string user_id = val["user_id"].asString();

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || user_id.empty() )
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int offset = val["offset"].asInt();
        int number = val["num"].asInt();
        int count = 0;
        Json::Value msg_list;

        // check wether user exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("to" << user_id), number, offset);
        while (cursor->more()) {
            BSONObj p = cursor->next();
            Json::Value msg;
            msg["msg_id"] = p.getStringField("msg_id");
            msg["type"] = p.getStringField("type");
            msg["source"] = p.getStringField("source");
            msg["from"] = p.getStringField("from");
            msg["content"] = p.getStringField("content");
            msg["time"] = p.getStringField("time");
            msg_list.append(msg);
            count++;
        }
        out["data"] = msg_list;
        out["count"] = count;

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int MessageList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int MessageList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int MessageList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int MessageList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 