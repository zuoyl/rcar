//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Accusation, "seller/accusation");

int Accusation::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Accusation::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string user_id;
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string reply_time = content["time"].asString();
    std::string reply_text = content["content"].asString();
    std::string accusation_id = content["accusation_id"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (seller_id.empty() || accusation_id.empty() || reply_text.empty())
        return APIE_INVALID_CLIENT_REQ;

    log(__func__," accusation_id = %s, reply = %s",
            __func__, accusation_id.c_str(), reply_text.c_str());

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // first. query wether the user exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_ACCUSATION, MONGO_QUERY("seller_id" << seller_id <<"accusation_id" << accusation_id));
        if (!cursor->more()) 
            return APIE_INVALID_CLIENT_REQ;

        std::string time = get_customstring_time();
        BSONObjBuilder objBuilder;
        objBuilder.append("role", "seller");
        objBuilder.append("seller_id", seller_id);
        objBuilder.append("content", reply_text);
        objBuilder.append("time", reply_time);


        // update evaluation info into fault db
        BSONObj info = BSON("$push" <<  BSON("replies" << objBuilder.obj()));
        c.update(DB_ACCUSATION, 
                MONGO_QUERY("seller_id" << seller_id << "accusation_id" << accusation_id),
                info, false, false);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Accusation::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Accusation::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["id"].asString();
    std::string accusation_id = content["accustion_id"].asString();

    if (role.empty() || seller_id.empty() || accusation_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    log("seller_id = %s", seller_id.c_str());

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__," seller %s doesn't exist", 
                    __func__, seller_id.c_str() );
            return APIE_SELLER_NO_EXIST;
        }
        BSONObj p = BSON("$pull" << BSON("accustions" << BSON("accustion_id" << accusation_id)));
        c.update(DB_SELLER, MONGO_QUERY("role" << role << "seller_id" << seller_id), p, true, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Accusation::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
