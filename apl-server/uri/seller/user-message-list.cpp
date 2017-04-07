//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(UserMessageList, "seller/user-message-list");

int UserMessageList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string user_id = val["user_id"].asString();

    if (role != "seller")
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

        Query query = MONGO_QUERY("$or" << 
                BSON_ARRAY(BSON("from" << seller_id << "to" << user_id) 
                        << BSON("from" << user_id << "to" << seller_id)));
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_MSG, query, number, offset);
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
int UserMessageList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int UserMessageList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int UserMessageList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int UserMessageList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
