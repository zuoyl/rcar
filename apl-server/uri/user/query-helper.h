//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#ifndef _QUERY_H_
#define _QUERY_H_

using namespace rcar;
using namespace mongo;


inline mongo::Query createQueryConditionForService(const Json::Value content, 
                const std::string &service) {
    // get client request's parameter and check
    std::string city = content["city"].asString();	
    double lng = content["lng"].asDouble();
    double lat = content["lat"].asDouble();
    Json::Value condition = content["condition"];
    int distance = condition["distance"].asUInt();
    std::string seller_type = condition["seller_type"].asString();//4s repair
    std::string sort_type = condition["sort_type"].asString();//distance priceL2H priceH2L

    Query query;
    //sort by distance
    if (sort_type == "distance") {
        // get seller_id where in condition
        if (seller_type == "all") {
            query = MONGO_QUERY("city" << city 
                    << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                          << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance))
                    << "services.type" << service);
        }
        else {
            query = MONGO_QUERY("city" << city 
                    << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                          << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance))
                    << "services.type" << service
                    << "seller_type" << seller_type); 
        }
    } else  {
        if (seller_type == "all") {
           query = MONGO_QUERY("city" << city 
                            << "location" << BSON("$near"
                                              << BSON("$geometry" 
                                                  << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                              << "$maxDistance" << distance)) 
                            << "services.type" << service);
        } else {
            query = MONGO_QUERY("city" << city 
                    << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                          << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance)) 
                    << "services.type" << service
                    << "seller_type" << seller_type );
 
        }
    }
    if (sort_type == "priceL2H") 
       query.sort(BSON("price" << 1));
    else 
       query.sort(BSON("price" << -1));

    return query;

}

inline mongo::Query createQueryConditionForAllActivity(const Json::Value content) {
    // get client request's parameter and check
    std::string city = content["city"].asString();	
    double lng = content["lng"].asDouble();
    double lat = content["lat"].asDouble();
    Json::Value condition = content["condition"];
    int distance = condition["distance"].asUInt();
    std::string seller_type = condition["seller_type"].asString();//4s repair
    std::string sort_type = condition["sort_type"].asString();//distance priceL2H priceH2L

    Query query;

    //sort by distance
    if (sort_type == "distance") {
        // get seller_id where in condition
        if (seller_type == "all") {
            query = MONGO_QUERY("city" << city 
                            << "location" << BSON("$near"
                                              << BSON("$geometry" 
                                              << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                              << "$maxDistance" << distance)));
 
        }
        else {
            query = MONGO_QUERY("city" << city
                    << "seller_type" << seller_type 
                    << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                      << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance)));
        }
    }
    else  {
        if (seller_type == "all") {
            query = MONGO_QUERY("city" << city 
                       << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                      << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance)));

        } else {
            query = MONGO_QUERY("city" << city 
                    << "seller_type" << seller_type
                    << "location" << BSON("$near"
                                      << BSON("$geometry" 
                                      << BSON("type" << "Point" << "coordinates" << BSON_ARRAY(lng << lat)) 
                                      << "$maxDistance" << distance)));


        }
    }
    if (sort_type == "priceL2H") 
       query.sort(BSON("price" << 1));
    else 
       query.sort(BSON("price" << -1));
    return query;
}

#endif //
