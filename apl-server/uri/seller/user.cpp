//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(User, "seller/user");

int User::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string user_id = val["user_id"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // confirm the seller exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller %s doesn's exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }
  
        cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id ));
        if (cursor->more()) {
            BSONObj seller = cursor->next();
            if (seller.hasField("users")) {
                BSONObj p = seller.getObjectField("users");
                for (BSONObj::iterator i = p.begin(); i.more();) {
                    BSONObj user = i.next().Obj();
                    std::string id = user.getStringField("user_id");
                    if (user_id == id) {
                        out["user_id"] = id;
                        out["name"]= user.getStringField("user_name");	
                        out["car_kind"] = user.getStringField("car_kind");
                        out["car_type"] = user.getStringField("car_type");
                        out["car_no"] = user.getStringField("car_no");
                        out["car_miles"] = user.getStringField("car_miles");
                        out["memo"] = user.getStringField("memo");
                        out["images"] = getImageNamesFromBson(user, "images");
                        return APIE_OK;
                    }
                }
            }
            return APIE_OK;
        } else {
            return APIE_USER_NO_EXIST;
        }
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
    
    return APIE_OK;

}
int User::Post(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string group_name = val["group_name"].asString();
    std::string user_id = val["user_id"].asString();
    std::string user_name = val["user_name"].asString();
    std::string car_kind = val["car_kind"].asString();
    std::string car_type = val["car_type"].asString();
    std::string car_no = val["car_no"].asString();
    std::string car_miles = val["car_miles"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || group_name.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor; 
        // check wether the user exist
        cursor = c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log("user_id %s does not exist", user_id.c_str());
            return  APIE_USER_NO_EXIST;
        }
        // construct user object 
        BSONObjBuilder user;
        user.append("user_id", user_id);
        user.append("car_kind", car_kind);
        user.append("car_type", car_type);
        user.append("car_no", car_no);
        user.append("user_name", user_name);
        user.append("car_miles", car_miles);

        // construct group
        BSONObjBuilder group;
        group.append("group_name", group_name);

        // check  wether the seller exist
        cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            // if user does not exist 
            //BSONObj p = BSON("$push" << BSON("users" << user.obj() << "groups.$.users" << item.obj()));
            group.append("group_id", OID::gen().toString());
            group.append("users", BSON_ARRAY(user_id));
            c.insert(DB_USRMNG, BSON("seller_id" << seller_id 
                        << "groups" << BSON_ARRAY(group.obj()) 
                        << "users" << BSON_ARRAY(user.obj())));
            return  APIE_OK;
        }

        // check wether the group exist
        cursor = c.query(DB_USRMNG, 
                 MONGO_QUERY("seller_id" << seller_id << "groups.group_name" << group_name));
        if (!cursor->more()) {
            // if user does not exist 
            //BSONObj p = BSON("$push" << BSON("users" << user.obj() << "groups.$.users" << item.obj()));
            group.append("group_id", OID::gen().toString());
            group.append("users", BSON_ARRAY(user_id));
            BSONObj p = BSON("$push" << BSON("users" << user.obj()) 
                        << "$set"<<  BSON("groups" << BSON_ARRAY(group.obj())));
            c.update(DB_USRMNG, BSON("seller_id" << seller_id), p);
            return  APIE_OK;
        }

        // if group already exist
        BSONObj info = BSON("$addToSet" <<  BSON("users" << user.obj() << "groups.$.users" << user_id));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id << "groups.group_name" << group_name), info, true, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int User::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &val = req.getInObject();

    // get client request's parameter and check
    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();
    std::string group_id = val["group_id"].asString();
    std::string user_id = val["user_id"].asString();
    std::string user_name = val["user_name"].asString();
    std::string car_kind = val["car_kind"].asString();
    std::string car_type = val["car_type"].asString();
    std::string car_no = val["car_no"].asString();
    std::string car_miles = val["car_miles"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // first. query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id << "users.user_id" << user_id));
        if (!cursor->more()) {
            log("user_id %s does not exist", user_id.c_str());
            return  APIE_USER_NO_EXIST;
        }
        // get the older group id and user name
        BSONObj seller = cursor->next();
        std::string old_group;
        BSONObj p = seller.getObjectField("groups");
        for (BSONObjIterator i = p.begin(); i.more(); ) {
            // uers
            BSONObj pp = i.next().Obj();
            std::string name = pp.getStringField("group_id");
            for (BSONObjIterator ii = pp.begin(); ii.more();) {
                BSONObj item = ii.next().Obj();
                // user item found
                if (item.getStringField("user_id") == user_id) {
                    old_group = group_id;
                    break;
                }
            }
        }
        
        // update user infor
        BSONObj info = BSON("$set" <<  BSON("users.$.car_kind" << car_kind <<
                                            "users.$.car_type" <<  car_type <<
                                            "users.$.car_no" << car_no << 
                                            "users.$.user_name" << user_name <<
                                            "users.$.car_miles" << car_miles));
        c.update(DB_USRMNG,
                MONGO_QUERY("seller_id" << seller_id << "users.user_id" << user_id), info, true, false);

        // if user group is specified, add the user into group
        if (!group_id.empty() && (old_group != group_id)) {
            // remove from older group 
            BSONObj p = BSON("$pull" << BSON("groups.$.users"  << user_id));
             c.update(DB_USRMNG, 
                    MONGO_QUERY("seller_id" << seller_id << "groups.group_id" << old_group),
                    p, false, false);
            // insert into new group
            p = BSON("$push" << BSON("groups.$.users" << user_id));
            c.update(DB_USRMNG, 
                    MONGO_QUERY("seller_id" << seller_id << "groups.group_id" << group_id),
                    p, false, false);
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int User::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || seller_id.empty())
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
            log("seller_id = %s ", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }

        // delete user from record.users list 
        BSONObj p = BSON("$pull" <<  BSON("users.user_id" << user_id));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id), p, false, false);

        // delete user from all groups
        p = BSON("$pull" << BSON("groups.$.users" << user_id));
        c.update(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id), p, false, false);

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int User::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
