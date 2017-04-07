//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Comments, "user/comments");

int Comments::Get(const ResourceContext &context, Request &req) {
    // get in and out json object 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    int page = content["page"].asInt();
    int page_size = content["page_size"].asInt();

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try{
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        int count = 0;
        Json::Value comments; 
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_COMMENT, MONGO_QUERY("seller_id" << seller_id));            
        //maybe find ads use "where status = publicated" 
        if (cursor->more()) {
            BSONObj com = cursor->next();
#if 0
            Json::Value advertisement;
            advertisement["ads_id"] = ads.getStringField("ads_id");
            advertisement["seller_id"] = ads.getStringField("seller_id");
            advertisement["type"] = ads.getStringField("type");
            advertisement["link"] = ads.getStringField("link");
            advertisement["start_time"] = ads.getStringField("start_time");
            advertisement["end_time"] = ads.getStringField("end_time");
            advertisement["title"] = ads.getStringField("content");
            advertisement["image"] = ads.getStringField("image");
#endif	
            BSONObj com_bson = com.getObjectField("comments");
            for (int i = (page*page_size); (i < com_bson.nFields()) && (count < page_size); i++) {
                BSONObj temp = com_bson[i].Obj();
                Json::Value comment;
                comment["user_id"] = temp.getStringField("user_id");
                comment["content"] = temp.getStringField("content");
                comment["time"] = temp.getStringField("time");
                comments.append(comment);
                count++;
            }            
        }
        out["status"] = APIE_OK;
        out["total"] = count;
        out["data"] = comments;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int Comments::Post(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string user_id = val["user_id"].asString();
    std::string content = val["content"].asString();
    std::string time = get_customstring_time();


    if (role.empty() || seller_id.empty() || user_id.empty() || content.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__," user_id = %s, seller_id = %s, content = %s",
            __func__, user_id.c_str(), seller_id.c_str(), content.c_str());

    BSONObjBuilder comment_obj;
    comment_obj.append("user_id", user_id);
    comment_obj.append("content", content);
    comment_obj.append("time", time);
    
    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__, "user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
#if 0
        if (!cursor->more()) {
            BSONObjBuilder builder;
            builder.append("seller_id",seller_id);
            c.insert(DB_COMMENT, builder.obj());
        }
#endif
        // update
        BSONObj info = BSON("$push" <<  BSON("comments" << comment_obj.obj()));
        c.update(DB_COMMENT, MONGO_QUERY("seller_id" << seller_id),info, true, false);

        // push msg to seller
        Json::Value msg;
        msg["source"] = "user";
        msg["kind"] = "comment";
        pushMessageToSeller(seller_id, msg);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Comments::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Comments::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Comments::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
