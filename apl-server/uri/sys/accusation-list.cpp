//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(AccusationList, "sys/accusation-list");

int AccusationList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get parameters
    const std::string start_time = content["start_time"].asString();
    const std::string end_time = content["end_time"].asString();
    const std::string key_word = content["key_word"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // search the whole data is not so  good
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ACCUSATION);
        int index = 0;
        Json::Value accusation_list;
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
                Json::Value accusation;
                if (!key_word.empty()) {
                    if (obj_content.find_first_of(key_word) != std::string::npos) {
                        accusation["id"] = obj_id;
                        accusation["content"] = obj_content;
                        accusation["time"] = obj_time;
                        accusation["kind"] = obj_kind;
                        accusation_list.append(accusation);
                    }
                }
                else {
                    accusation["id"] = obj_id;
                    accusation["content"] = obj_content;
                    accusation["time"] = obj_time;
                    accusation["kind"] = obj_kind;
                    accusation_list.append(accusation);
                }
            }
            out["total"] = index;
            out["item_list"] = accusation_list;
        }
 
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int AccusationList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int AccusationList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int AccusationList::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int AccusationList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
