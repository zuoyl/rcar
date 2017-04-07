//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(CommentList, "seller/comment-list");

int CommentList::Get(const ResourceContext &context, Request &req) {
    // get in and out json object 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    int offset = content["offset"].asInt();
    int number = content["number"].asInt();


    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s", seller_id.c_str());

    try{
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int count = 0;
        Json::Value data;

        // get comment info from DB
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_COMMENT, MONGO_QUERY("seller_id" << seller_id));            

        if (cursor->more()) {
            BSONObj p = cursor->next();
            BSONObj comments = p.getObjectField("comments");
            for (int i = offset; (i < comments.nFields()) && (count < number); i++) {
                BSONObj c = comments[i].Obj();
                Json::Value comment;
                comment["user_id"] = c.getStringField("user_id");
                comment["content"] = c.getStringField("content");
                comment["time"] = c.getStringField("time");
                data.append(comment);
                count++;
            }            
        }
        out["total"] = count;
        out["data"] = data;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int CommentList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int CommentList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int CommentList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int CommentList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
