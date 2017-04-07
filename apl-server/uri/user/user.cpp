//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(User, "user/");

int User::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &user = req.getOutObject();

    // get client request of user's id
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // check wethere the user is registered or login
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("role" << role << "user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"%s is not in session", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"%s is not registered", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

	  //get user's detail information
        BSONObj p = cursor->next();
        user["user_name"] = p.getStringField("user_name");
        user["sex"] = p.getStringField("sex");
        user["email"] = p.getStringField("email");
        user["home_city"] = p.getStringField("home_city");            
        user["home_addr"] = p.getStringField("home_addr");
        user["images"] = getImageNamesFromBson(p, "images");
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
 
    return APIE_OK;

    return Resource::Get(context, req);
}
int User::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["mobile"].asString();
    std::string pwd = content["pwd"].asString();

    if (role.empty() || user_id.empty() || pwd.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__,"user_id = %s, pwd = %s", user_id.c_str(), pwd.c_str());

    try {
        // connetct to DB		
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (cursor->more()) {
            log(__func__,"user %s already exist", user_id.c_str());
            return APIE_USER_ALREADY_EXIST;
        }

        // insert record to DB
        BSONObj p = BSON(GENOID << "user_id" << user_id 
                                << "pwd" << pwd
                                << "online" << 0);
        c.insert(DB_USR, p);
        out["user_id"] = user_id;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int User::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int User::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s", user_id.c_str());

    try {
        // connect DB		
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));

        if (cursor->more()) {
            // delete user from db.user 
            c.remove(DB_USR,MONGO_QUERY("user_id" << user_id));
            return APIE_OK;
        }

        log(__func__,"user %s no exist", user_id.c_str());
        return APIE_USER_NO_EXIST;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int User::Patch(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();

    // get client request's detail information
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string user_name = content["user_name"].asString();
    std::string sex = content["sex"].asString();
    std::string email = content["email"].asString();
    std::string home_city = content["home_city"].asString();	
    std::string home_addr = content["home_addr"].asString();

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;
	
    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        //connetct to DB	
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        //query wether the user exist or login
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SESSION, MONGO_QUERY("role" << role << "user_id" << user_id));

        if (!cursor->more()) {
            log(__func__,"%s is not in session", user_id.c_str());
            return APIE_INVALID_CLIENT_REQ;
        }

        // update information to DB
        BSONObjBuilder builder;
        builder.append("user_name", user_name);
        builder.append("sex", sex);
        builder.append("email", email);
        builder.append("home_city", home_city);
        builder.append("home_addr", home_addr);
     
        BSONObj info = BSON("$set" << builder.obj());
        c.update(DB_USR, MONGO_QUERY("user_id" << user_id), info, false, false);

        // creat folder for user's icon
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

} // namespace 
