//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Notification, "sys/notification");

int Notification::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get parameters
    const std::string start_time = content["start_time"].asString();
    const std::string end_time = content["end_time"].asString();
    const std::string kind = content["kind"].asString();
    const std::string key_word = content["key_word"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_MSG, MONGO_QUERY("kind" << kind));
        int index = 0;
        Json::Value notify_list;
        while (cursor->more()) {
            index++;
            BSONObj obj = cursor->next();
            std::string obj_time = obj.getStringField("time");
            std::string obj_kind = obj.getStringField("kind");
            std::string obj_content= obj.getStringField("content");
            std::string obj_id = obj.getStringField("id");

            // check condition
            if (time_is_early_than(obj_time, end_time) &&
                time_is_late_than(obj_time, start_time)) {
                Json::Value notify;
                if (!key_word.empty()) {
                    if (obj_content.find_first_of(key_word) != std::string::npos) {
                        notify["id"] = obj_id;
                        notify["content"] = obj_content;
                        notify["time"] = obj_time;
                        notify["kind"] = obj_kind;
                        notify_list.append(notify);
                    }
                }
                else {
                    notify["id"] = obj_id;
                    notify["content"] = obj_content;
                    notify["time"] = obj_time;
                    notify["kind"] = obj_kind;
                    notify_list.append(notify);
                }
            }
            out["total"] = index;
            out["item_list"] = notify_list;
        }
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Notification::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    const std::string time = content["time"].asString();
    const std::string kind = content["kind"].asString();
    const std::string text = content["content"].asString();

    if (time.empty() || kind.empty()) {
        log("invalid parameter");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // insert notification into db.notify
        BSONObjBuilder builder;
        builder.append("time", time);
        builder.append("kind", kind);
        builder.append("content", text);

        c.insert(DB_NOTIFY, builder.obj());

        // notify all users online
        Json::Value msg;
        msg["source"] = "sys";
        msg["kind"] = "notification";
        msg["title"] = kind;
        msg["content"] = text;
        pushMessageToAllUsers(msg);
        pushMessageToAllSellers(msg);

        // for offline users, how to deal with them
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Notification::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Notification::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    const std::string id = content["id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        std::unique_ptr<mongo::DBClientCursor> cursor = 
            c.query(DB_NOTIFY, MONGO_QUERY("id" << id));
        if (cursor->more()) 
            c.remove(DB_NOTIFY, cursor->next());
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

int Notification::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
