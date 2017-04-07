//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(CarList, "user/car-list");

// get user CarList list
int CarList::Get(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();
    Json::Value &out = req.getOutObject();

    // get client request's detail information
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
#if 0
    if (role.empty() || user_id.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;
#endif
    role = "user";
    user_id = "13889536307";
    log(__func__, "user_id = %s, role = %s", user_id.c_str(), role.c_str());

    try {
        // connect DB		
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        Json::Value objs;
        int count = 0;

        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__, "user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        else {
            //get user's car list
            BSONObj p = cursor->next();
            BSONObj cars = p.getObjectField("cars");
            for (int i = 0; i < cars.nFields(); i++) {
                BSONObj car = cars[i].Obj();	
                std::string platenumber = car.getStringField("platenumber");
                Json::Value car_obj;
                car_obj["platenumber"] = platenumber;
                car_obj["kind"] = car.getStringField("kind");
                car_obj["brand"] = car.getStringField("brand");
                car_obj["buy_date"] = car.getStringField("buy_date");
                car_obj["miles"] = car.getStringField("miles");	
                car_obj["insurance"] = car.getStringField("insurance");
                objs.append(car_obj);
                count++;
            } 
        }	
        out["total"] = count;
        out["data"] = objs;
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred  with mongodb");
        return APIE_SERVER_INTERNAL;
    }
    return APIE_OK;
}


// add a car into user info
int CarList::Post(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's detail information
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string platenumber = content["platenumber"].asString();
    std::string kind = content["kind"].asString();
    std::string brand = content["brand"].asString();
    std::string buy_date = content["buy_date"].asString();
    std::string miles = content["miles"].asString();
    std::string insurance = content["insurance"].asString();

    if (role.empty() || user_id.empty() || platenumber.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s, role = %s, platenumber = %s",
            user_id.c_str(), role.c_str(), platenumber.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        // update CarList info
        BSONObjBuilder car; 
        car.append("platenumber", platenumber);	
        car.append("kind", kind);
        if (!brand.empty())
            car.append("brand", brand);

        if (!buy_date.empty())
            car.append("buy_date", buy_date);

        if (!miles.empty())
            car.append("miles", miles);

        if (!insurance.empty())
            car.append("insurance", insurance);
     
        BSONObj info = BSON("$push" << BSON("cars" << car.obj()));
        c.update(DB_USR, MONGO_QUERY("user_id" << user_id), info);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

// delete a car from user info
int CarList::Delete(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's detail information and check
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string platenumber = content["platenumber"].asString();

    if (role.empty() || user_id.empty() || platenumber.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__, "user_id = %s, role = %s, platenumber = %s",
            user_id.c_str(), role.c_str(), platenumber.c_str());

    try {
        // connect DB        
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        //query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }
        else {
            // delete car from car list
            BSONObj info = BSON("$pull" <<  BSON("cars" << BSON("platenumber" << platenumber)));
            c.update(DB_USR, MONGO_QUERY("user_id" << user_id),info, false, false);
        }	

        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}

// update a car info
int CarList::Patch(const ResourceContext &context, Request &req) {
    const Json::Value &content = req.getInObject();

    // get client request's detail information
    std::string role = content["role"].asString();
    std::string user_id = content["user_id"].asString();
    std::string platenumber = content["platenumber"].asString();

    std::string brand = content["brand"].asString();
    std::string buy_date = content["buy_date"].asString();
    std::string miles = content["miles"].asString();
    std::string insurance = content["insurance"].asString();

    if (role.empty() || user_id.empty() || platenumber.empty())
        return APIE_INVALID_CLIENT_REQ;

    if (role != "user")
        return APIE_INVALID_CLIENT_REQ;

    log(__func__,"user_id = %s, role = %s, platenumber = %s",
            user_id.c_str(), role.c_str(), platenumber.c_str());

    try {
        // connect DB
        mongo::DBClientConnection c;
        c.connect(context.getMongo());
        
        // query wether the user exist
        std::unique_ptr<mongo::DBClientCursor> cursor =
            c.query(DB_USR, MONGO_QUERY("user_id" << user_id));
        if (!cursor->more()) {
            log(__func__,"user %s doesn't exist", user_id.c_str());
            return APIE_USER_NO_EXIST;
        }

        // update record
        BSONObjBuilder car;
        car.append("cars.$.platenumber", platenumber);
		
        if (!brand.empty())
            car.append("cars.$.brand", brand);

        if (!buy_date.empty())
            car.append("cars.$.buy_date", buy_date);

        if (!miles.empty())
            car.append("cars.$.miles", miles);

        if (!insurance.empty())
            car.append("cars.$.insurance", insurance);


        BSONObj info = BSON("$set" << car.obj());
        c.update(DB_USR, MONGO_QUERY("user_id" << user_id << "cars.platenumber" << platenumber), info);
        return APIE_OK;
    }
    catch (const mongo::DBException &e) {
        log(__func__," exception occurred with mongodb");
        return APIE_SERVER_INTERNAL;
    }

    return APIE_OK;
}


int CarList::Put(const ResourceContext &context, Request &req) {
    return Resource::Put(context, req);
}
} // namespace 
