//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Commodity, "seller/commodity");

int Commodity::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &commodity = req.getOutObject();

    // get valid commodity id 
    std::string commod_id = content["commod_id"].asString();
    if (commod_id.empty()) {
        log(__func__, "commodity's id is empty");
        return APIE_INVALID_CLIENT_REQ;
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_STORE, MONGO_QUERY("commod_id" << commod_id));

        if (cursor->more()) {
            BSONObj p = cursor->next();
            commodity["seller_id"] = p.getStringField("seller_id");
            commodity["commod_id"] = p.getStringField("commod_id");
            commodity["name"] = p.getStringField("name");
            commodity["price"] = p.getStringField("price");
            commodity["brand"] = p.getStringField("brand");
            commodity["amount"] = p.getStringField("amount");
            commodity["status"] = p.getStringField("status");

            // get image
            BSONObj images_obj = p.getObjectField("commoditypic");
            Json::Value images;
            for (int index = 0; index < images_obj.nFields(); index++) {
                BSONObj img_obj = images_obj[index].Obj();
                Json::Value image;
                image["name"] = img_obj.getStringField("name");
                image["data"] = img_obj.getStringField("data");
                images.append(image);
            }
            commodity["commoditypic"] = images;
            return APIE_OK;
        }
        else
        {
            return APIE_COMMODITY_NO_EXIST;
        }
    }
    catch (const mongo::DBException &e) {
        log(__func__, "exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
 
    return APIE_OK;

}
int Commodity::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string commod_id = OID::gen().toString();
    std::string name = content["name"].asString();
    std::string desc = content["desc"].asString();
    std::string price = content["price"].asString();
    std::string brand = content["brand"].asString();
    std::string amount = content["amount"].asString();
    std::string cutoff = content["cutoff"].asString();

    if (role.empty() || seller_id.empty() || name.empty()
            || price.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__,"seller_id = %s, name = %s", seller_id.c_str(), name.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_SELLER, MONGO_QUERY("seller_id" << seller_id));
        if (!cursor->more()) {
            log(__func__,"seller no exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }

        // construct bson object
        BSONArray imageNames = getImageNames(content["images"]);
        out["images"] = getImageNamesFromBson(imageNames);
 
        // insert record
        BSONObjBuilder builder;
        builder.append("seller_id", seller_id);
        builder.append("commodity_id", commod_id);
        builder.append("name", name);
        builder.append("desc", desc);
        builder.append("price", price);
        builder.append("brand", brand);
        builder.append("amount", amount);
        builder.append("images", imageNames);
        builder.append("cutoff", cutoff);

        c.insert(DB_STORE, builder.obj());
        out["commodity_id"] =  commod_id;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__,"exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}
int Commodity::Put(const ResourceContext &context, Request &req) { 
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string seller_id = content["seller_id"].asString();
    std::string name = content["name"].asString();
    std::string desc = content["desc"].asString();
    std::string price = content["price"].asString();
    std::string brand = content["brand"].asString();
    std::string amount = content["amount"].asString();
    std::string cutoff = content["cutoff"].asString();
    std::string commodity_id = content["commodity_id"].asString();

    if (role.empty() || seller_id.empty() || commodity_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__,"seller_id = %s, name = %s", seller_id.c_str(), name.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // query wether the seller exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_STORE, MONGO_QUERY("seller_id" << seller_id << "commodity_id" << commodity_id));
        if (!cursor->more()) {
            log(__func__,"commodity no exist", seller_id.c_str());
            return APIE_SELLER_NO_EXIST;
        }
        // remove old commodity first
        c.remove(DB_STORE, 
                MONGO_QUERY("seller_id" << seller_id << "commodity_id" << commodity_id));

        // construct bson object
        BSONArray images = getImageNames(content["images"]);
        // construct images to client
        out["images"] = getImageNamesFromBson(images);;
 
        // insert record
        BSONObjBuilder builder;
        builder.append("seller_id", seller_id);
        builder.append("commodity_id", commodity_id);
        builder.append("name", name);
        builder.append("desc", desc);
        builder.append("price", price);
        builder.append("brand", brand);
        builder.append("amount", amount);
        builder.append("images", images);
        builder.append("cutoff", cutoff);

        c.insert(DB_STORE, builder.obj());
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Commodity::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's parameter and check
    std::string role = content["role"].asString();
    std::string id = content["commodity_id"].asString();
    if (role.empty() || id.empty())
        return APIE_INVALID_CLIENT_REQ;
    if (role != "seller")
        return APIE_INVALID_CLIENT_REQ;


    log(__func__," id = %s", id.c_str());

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        // first. query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_STORE, MONGO_QUERY("commodity_id" << id));
        if (cursor->more()) {
            // delete data 
            c.remove(DB_STORE,MONGO_QUERY("commodity_id" << id));
            return APIE_OK;
        }
        log(__func__," commodity no exist", id.c_str());
        return APIE_COMMODITY_NO_EXIST;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;

}

int Commodity::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
