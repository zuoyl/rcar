//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(UserList, "seller/user-list");

int UserList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &val = req.getInObject();
    Json::Value &out = req.getOutObject();

    std::string role = val["role"].asString();
    std::string seller_id = val["seller_id"].asString();

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;

    if (role.empty() || seller_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // confirm the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log("seller %s doesn's exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }
  
        cursor = c.query(DB_USRMNG, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            // if the seller's user management document does's exist, just create 
            //c.insert(DB_USRMNG, BSON("seller_id" << seller_id));
            return APIE_OK;
        } 
        // get all user list
        BSONObj seller = cursor->next();
        BSONObj groups = seller.getObjectField("groups");
        Json::Value groupList; 
        for (BSONObjIterator i = groups.begin(); i.more();) {
            Json::Value group;
            Json::Value users;
            // group
            BSONObj obj = i.next().Obj();
            group["group_id"] = obj.getStringField("group_id");
            group["name"] = obj.getStringField("group_name");
            // users
            BSONObj p = obj.getObjectField("users");
            for (BSONObjIterator n = p.begin(); n.more();) {
                std::string id = n.next().str();

                // user info
                Json::Value user;
                user["user_id"] = id;
                // get user online status

                cursor = c.query(DB_USR, MONGO_QUERY("user_id" << id));
                if (cursor->more()) {
                    BSONObj pp = cursor->next();
                    user["online"] = pp.getIntField("online");
                    user["name"] = pp.getStringField("name");
                }  else {
                    user["online"] = 0;
                    user["name"] = "unknown";
                }
                users.append(user);
            }
            group["users"] = users;
            groupList.append(group);
        }

#if 0
        // for users
        BSONObj musers = seller.getObjectField("users");
        Json::Value users; 
        for (int i = 0; i < musers.nFields(); i++) {
            BSONObj muser = musers[i].Obj();
         
            // for grop name
            Json::Value user;
            std::string user_id = muser.getStringField("user_id");
            std::string user_name = muser.getStringField("user_name");	
            std::string car_kind = muser.getStringField("car_kind");
            std::string car_type = muser.getStringField("car_type");
            std::string car_no = muser.getStringField("car_no");
            std::string car_miles = muser.getStringField("car_miles");
            
            //add user id
            user["user_id"] = user_id;
            user["name"] = user_name;
            user["car_kind"] = car_kind;
            user["car_type"] = car_type;	
            user["car_no"] = car_no;	
            user["car_miles"] = car_miles;
            user["images"] = get_images(muser, "images");
            users.append(user); 
        }
        out["users"] = users;
#endif 
        out["data"] = groupList;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
    
    return APIE_OK;

}
int UserList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int UserList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int UserList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int UserList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
