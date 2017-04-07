//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(GroupMessage, "seller/group-message");

int GroupMessage::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int GroupMessage::Post(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string group_id = val["group_id"].asString();
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
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            return APIE_SELLER_NO_EXIST;
        }
       // get all users
        BSONArrayBuilder arrayBuilder;
        std::vector<std::string> uuids;

        cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));
        BSONObj seller = cursor->next();
        BSONObj p = seller.getObjectField("groups");

        for (BSONObj::iterator i = p.begin(); i.more();) {
            BSONObj group = i.next().Obj();
            std::string gid = group.getStringField("group_id");
            if (gid == group_id) {
                BSONObj m = group.getObjectField("users");
                for (BSONObj::iterator n = m.begin(); n.more();) {
                    uuids.push_back(n.next().str());
                    arrayBuilder.append(n.next().str());
                }
            }
        }
        // construct msg
        OID id = OID::gen();
        BSONObj m = BSON("msg_id" << id.toString()
                               << "type" << "group" 
                               << "group_id" << group_id
                               << "source" << "seller"
                               << "from" << seller_id
                               << "to" << arrayBuilder.arr() 
                               << "content" << content
                               << "time" << time);

        c.insert(DB_MSG, m);

 
        // notify user
        Json::Value msg;
        msg["source"] = "seller";
        msg["kind"] = "msg";
        pushMessageToUsers(uuids, msg);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int GroupMessage::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int GroupMessage::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int GroupMessage::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
