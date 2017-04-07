//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(AccusationList, "seller/accusation-list");

int AccusationList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get parameters
    const std::string role = content["role"].asString();
    const std::string seller_id = content["seller_id"].asString();

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // search the whole data is not so  good
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ACCUSATION, MONGO_QUERY("seller_id" << seller_id ));
        int count = 0;
        Json::Value accusation_list;
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            Json::Value accusation;
            accusation["accusation_id"] = obj.getStringField("accusation_id");
            accusation["user_id"] = obj.getStringField("user_id");
            accusation["seller_id"] = obj.getStringField("seller_id");
            accusation["title"] = obj.getStringField("title");
            accusation["detail"] = obj.getStringField("detail");
            accusation["date"] = obj.getStringField("date");
            accusation["status"] = obj.getStringField("status");
            accusation["images"] = getImageNamesFromBson(obj, "images");

            Json::Value replies;
            if (obj.hasField("replies")) {
                BSONObj p = obj.getObjectField("replies");
                for (BSONObjIterator i = p.begin(); i.more();) {
                    BSONObj pp = i.next().Obj();
                    Json::Value reply;
                    reply["role"] = pp.getStringField("role");
                    reply["time"] = pp.getStringField("time");
                    reply["content"] = pp.getStringField("content");
                    replies.append(reply);
                }
            accusation["replies"] = replies;
            accusation_list.append(accusation);
            count++;
            }
        }
        out["total"] = count;
        out["data"] = accusation_list;
        return APIE_OK;
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
    return Resource::Delete(context, req); 
}

int AccusationList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
