//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Message, "user/message");

static int getUnloginUserMsg(const ResourceContext &context,
        Request &req, mongo::DBClientConnection &c) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    int num = val["num"].asInt();
    int offset = val["offset"].asInt();

    Json::Value msgs(Json::arrayValue);
    std::unique_ptr<mongo::DBClientCursor> cursor =
        c.query(DB_MSG, MONGO_QUERY("kind" << "sys"), num, offset);
    while (cursor->more()) {
        BSONObj msg = cursor->next();
        Json::Value info;
        info["msg_id"] = msg.getStringField("msg_id");
        info["source"] = "sys";
        info["kind"] = msg.getStringField("kind");
        info["type"] = msg.getStringField("type");
        info["title"] = msg.getStringField("title");
        info["time"] = msg.getStringField("time");
        info["url"] = msg.getStringField("url");
        info["seller_id"] = msg.getStringField("seller_id");
        info["replylable"] = "no";
        msgs.append(info);
    } 
    out["data"] = msgs;
    return APIE_OK;
}


static int getLoginUserMsg(const ResourceContext &context,
        Request &req, mongo::DBClientConnection &c) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    int num = val["num"].asInt();
    int offset = val["offset"].asInt();

    Json::Value msgs(Json::arrayValue);
    std::unique_ptr<mongo::DBClientCursor> cursor =
        c.query(DB_MSG, MONGO_QUERY("kind" << BSON_ARRAY("sys" << "seller")), num, offset);
    while (cursor->more()) {
        BSONObj msg = cursor->next();
        Json::Value info;
        info["msg_id"] = msg.getStringField("msg_id");
        info["source"] = msg.getStringField("source");
        info["kind"] = msg.getStringField("kind");
        info["type"] = msg.getStringField("type");
        info["title"] = msg.getStringField("title");
        info["time"] = msg.getStringField("time");
        info["url"] = msg.getStringField("url");
        info["seller_id"] = msg.getStringField("seller_id");
        info["user_id"] = msg.getStringField("user_id");
        info["replylable"] = msg.getStringField("replyable");
        msgs.append(info);
    } 
    out["data"] = msgs;
    return APIE_OK;

}

static int getMessageDetail(const ResourceContext &context,
        Request &req, mongo::DBClientConnection &c) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string msgid = val["msgid"].asString();

    std::unique_ptr<mongo::DBClientCursor> cursor =
        c.query(DB_MSG, MONGO_QUERY("msg_id" << msgid));
    if (cursor->more()) {
        BSONObj msg = cursor->next();
        Json::Value info;
        info["msg_id"] = msg.getStringField("msg_id");
        info["source"] = msg.getStringField("source");;
        info["kind"] = msg.getStringField("kind");
        info["type"] = msg.getStringField("type");
        info["title"] = msg.getStringField("title");
        info["time"] = msg.getStringField("time");
        info["url"] = msg.getStringField("url");
        info["seller_id"] = msg.getStringField("seller_id");
        info["user_id"] = msg.getStringField("user_id");
        info["replylable"] = "no";


        // get replies
        Json::Value replies(Json::arrayValue);
        if (msg.hasField("replies")) {
            BSONObj p = p.getObjectField("replies");
            for (BSONObjIterator i = p.begin(); i.more();)  {
                BSONObj obj = i.next().Obj();
                Json::Value reply;
                reply["source"] = obj.getStringField("source");
                reply["time"] = obj.getStringField("time");
                reply["content"] = obj.getStringField("content");
                reply["seller_id"] = obj.getStringField("seller_id");
                reply["user_id"] = obj.getStringField("user_id");
                reply["seller_name"] = obj.getStringField("seller_name");
                reply["user_name"] = obj.getStringField("user_name");
                reply["pname"] = obj.getStringField("pname");
                reply["images"] = getImageNamesFromBson(obj, "images");
                replies.append(reply);
            }
        }
        info["replies"] = replies;
        out["data"] = info;

    } else {
        return APIE_INVALID_CLIENT_REQ;
    }
    return APIE_OK;

}

int Message::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string detail = val["detail"].asString();
    std::string userid = val["user_id"].asString();

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        if (detail == "yes")
            return getMessageDetail(context, req, c);
        if (!userid.empty())
            return getLoginUserMsg(context, req, c);
        else
            return getUnloginUserMsg(context, req, c);
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }


}
int Message::Post(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string msg_id = val["msg_id"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string user_id = val["user_id"].asString();
    std::string content = val["content"].asString();
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
        BSONObj seller = cursor->next();
        std::string seller_name = seller.getStringField("name"); 
        // check wether user exist
        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            return APIE_USER_NO_EXIST;
        }
        BSONObj user = cursor->next();
        std::string user_name = user.getStringField("name");

        // if message id is null, just create a new message
        if (msg_id.empty()) {
            OID id = OID::gen();
            msg_id = id.toString();
            BSONObj obj = BSON("msg_id" << msg_id 
                           << "type" << "user" 
                           << "user_id" << user_id 
                           << "seller_id" << seller_id
                           << "user_name" << user_name
                           << "seller_name" << seller_name
                           << "content" << content
                           << "time" << time);
            c.insert(DB_MSG, obj);
        } else {
            // a reply message
            cursor = c.query(DB_MSG, MONGO_QUERY("msg_id" << msg_id));
            if (!cursor->more()) {
                return APIE_USER_NO_EXIST;
            }
            BSONObj obj = BSON("type" << "user" 
                           << "user_id" << user_id 
                           << "user_name" << user_name
                           << "seller_name" << seller_name
                           << "seller_id" << seller_id
                           << "content" << content
                           << "time" << time);
            BSONObj p = BSON("$push" << BSON("replies" << obj));
            c.update(DB_MSG, MONGO_QUERY("msg_id" << msg_id), p); 
        }
        // notify user
        Json::Value msg;
        msg["source"] = "user";
        msg["kind"] = "msg";
        msg["msg_id"] = msg_id;
        pushMessageToSeller(seller_id, msg);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int Message::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Message::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Message::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
