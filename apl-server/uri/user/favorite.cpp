//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Favorite, "user/favorite");

int Favorite::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // first. query wether the user exist
        std::auto_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("name" << user_id));
        if (!cursor->more()) {
            log(__func__, "user no exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        BSONObj user = cursor->next();

        // check wether the favorite field exist
        if (user.hasField("favorite")) {
            Json::Value sellerList;
            Json::Value commodityList;
            BSONObj favorite = user.getObjectField("favorite");

            // seller
            if (favorite.hasField("seller")) {
                BSONObj sellers = favorite.getObjectField("seller");
                for (BSONObjIterator i = sellers.begin(); i.more();) {
                    BSONObj seller = i.next().Obj();
                    Json::Value val;
                    val["seller_id"] = seller.getStringField("seller_id");
                    val["name"] = seller.getStringField("name");
                    val["telephone"] = seller.getStringField("telephone");
                    val["address"] = seller.getStringField("address");
                    val["type"] = seller.getStringField("type");
                    val["intro"] = seller.getStringField("intro");
                    val["images"] = getImageNamesFromBson(seller, "images");
                    sellerList.append(val);
                }
            }

            if (favorite.hasField("commodity")) {
                BSONObj commodities = favorite.getObjectField("commodities");
                for (BSONObjIterator i = commodities.begin(); i.more();) {
                    BSONObj c = i.next().Obj();
                    Json::Value val;
                    val["seller_id"] = c.getStringField("seller_id");
                    val["cid"] = c.getStringField("cid");
                    val["name"] = c.getStringField("name");
                    val["desc"] = c.getStringField("desc");
                    val["band"] = c.getStringField("band");
                    val["cutoff"] = c.getStringField("cutoff");
                    val["rate"] = c.getStringField("rate");
                    val["images"] = getImageNamesFromBson(c, "images");
                    commodityList.append(val);
                }
            }
            out["sellers"] = sellerList;
            out["commodities"] = commodityList;
        }
 
       return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Favorite::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string type = content["type"].asString();//commodities sellers
    std::string id = content["id"].asString();

    if (role.empty() || user_id.empty() || type.empty()
            || id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__, "user_id = %s, type = %s,id = %s",
            user_id.c_str(), type.c_str(), id.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the user exis
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__, "user %s no exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        // update record
        BSONObj info = BSON("$addToSet" << BSON(type << id));
        c.update(DB_USR, MONGO_QUERY("user_id" << user_id), info, true, false);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}
int Favorite::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Favorite::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Favorite::Patch(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    Json::Value sellers = content["sellers"];
    Json::Value commodities = content["commodities"];

    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // first. query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("name" << user_id));
        if (!cursor->more()) {
            log(__func__, "user no exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        BSONObj user = cursor->next();

        // construct object
        // sellers
        BSONArrayBuilder sellerBuilder;
        for (unsigned int i = 0; i < sellers.size(); i++) {
            Json::Value seller = sellers[i]; 
            BSONObjBuilder p;
            p.append("seller_id", seller["seller_id"].asString());
            p.append("name", seller["name"].asString());
            p.append("telephone", seller["telephone"].asString());
            p.append("address", seller["address"].asString());
            p.append("type", seller["type"].asString());
            p.append("intro", seller["intro"].asString());
            p.append("images", getImageNames(seller["images"]));
            sellerBuilder.append(p.obj());
        }
        // commodity
        BSONArrayBuilder comBuilder;
        for (unsigned int i = 0; i < commodities.size(); i++) {
            Json::Value c = commodities[i];
            BSONObjBuilder p;
            p.append("cid", c["cid"].asString());
            p.append("seller_id", c["seller_id"].asString());
            p.append("name", c["name"].asString());
            p.append("desc", c["desc"].asString());
            p.append("band", c["band"].asString());
            p.append("cutoff", c["cutoff"].asString());
            p.append("rate", c["rate"].asString());
            p.append("images", getImageNames(c["images"]));
            comBuilder.append(p.obj());
        }

        // check wether the favorite field exist
        if (!user.hasField("favorite")) {
            // insertf favorite 
            BSONObj info = BSON("$push" << BSON("favorite" << 
                           BSON("seller" << sellerBuilder.arr() <<
                                "commodity" << comBuilder.arr())));
            c.update(DB_USR, MONGO_QUERY("user_id" << user_id), info);
            return APIE_OK;

        } else {
            // delete all favorite
            c.update(DB_USR, MONGO_QUERY("user_id" << user_id), BSON("$pull" << "favorite"));
            // insert new favorite
            BSONObj info = BSON("$push" << BSON("favorite" << 
                           BSON("seller" << sellerBuilder.arr() <<
                                "commodity" << comBuilder.arr())));
 
            c.update(DB_USR, MONGO_QUERY("user_id" << user_id), info);
            return APIE_OK;
        }
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

} // namespace 
