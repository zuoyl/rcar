//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(UserGroup, "seller/user-group");

int UserGroup::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int UserGroup::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string group_name = content["group_name"].asString();
    std::string group_id = content["group_id"].asString();
    Json::Value users = content["users"];


    if (role.empty() || seller_id.empty() || group_name.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    try {
        // connetc DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the seller exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));

        if (!cursor->more()) {
            log("seller_id = %s, group_name = %s", seller_id.c_str(), group_name.c_str());
            return APIE_SELLER_NO_EXIST;
        }
        // construt group object
        BSONObjBuilder group;
        group.append("group_name", group_name);
        if (group_id.empty())
            group.append("group_id", OID::gen().toString());

        // construct users object
        BSONArrayBuilder items;
        for (Json::Value::iterator i = users.begin(); i != users.end(); i++) {
            Json::Value user = *i;
            items.append(user.asString());
        }

        // if the seller does not exist
        cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            group.append("users", items.arr());
            c.insert(DB_USRMNG, 
                    BSON("seller_id" << seller_id << "groups" << BSON_ARRAY(group.obj())));
            return APIE_OK;
        }

        BSONObj seller = cursor->next();
        group.append("users", items.arr());
        BSONObj info = BSON("$push" << BSON("groups" << group.obj()));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id), info, false, false);
        out["group_id"] = group_id;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int UserGroup::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &val = req.getInObject();

    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string group_name = val["group_name"].asString();
    std::string user_id = val["user_id"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || group_name.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // first. query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));

        if (!cursor->more()) {
            return APIE_OK;
        }

        BSONObj info = BSON("$pull" <<  BSON("groups.$.users" << user_id));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id << "groups.group_name"<< group_name),info, true, false);		

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int UserGroup::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string group_id = content["group_id"].asString();

    if (role.empty() || seller_id.empty() || group_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    try {
        // connetc DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));

        if (!cursor->more()) {
            log("seller_id = %s, group_id = %s", seller_id.c_str(), group_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }

        // update
        BSONObj p = BSON("$pull" <<  BSON("groups" << BSON("group_id" << group_id)));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id), p, false, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int UserGroup::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
