#include <string>
#include "db.h"

using namespace mongo;

bool rcar::mongodb_initialize() {
    mongo::client::Options option;
    mongo::Status status = mongo::client::initialize(option);
    if (!status.isOK()) {
        return false;
    }
    return true;
}


Json::Value rcar::tojson(const mongo::BSONObj &root, const char *key) {
    Json::Value val(Json::objectValue);
    Json::Reader reader;
    mongo::BSONObj obj = root; 

    if (key != nullptr && root.hasField(key))
        obj = root.getObjectField(key);
        

    reader.parse(mongo::tojson(obj), val);
    return val;
}


mongo::BSONObj rcar::fromjson(const Json::Value &root, const char *key) {
    Json::FastWriter writer;
    Json::Value val = key?root[key]:root;; 
    return mongo::fromjson(writer.write(val));
}


mongo::BSONArray rcar::getImageNames(const Json::Value &number) {
    return getImageNames(number.asInt());
}
mongo::BSONArray rcar::getImageNames(int number) {
    BSONArrayBuilder ids;
    for (int i = 0; i < number; i++) {
        OID id = OID::gen();
        std::string name = id.toString() + ".jpeg";
        ids.append(name);
    }
    return ids.arr();
}

// converer helpers
Json::Value rcar::getImageNamesFromBson(const mongo::BSONObj &names) {
    Json::Value result(Json::arrayValue);
    for (BSONObj::iterator i = names.begin(); i.more();) {
        result.append(i.next().str());
    }
    return result;
}

Json::Value rcar::getImageNamesFromBson(const mongo::BSONArray &names) {
    Json::Value result(Json::arrayValue);
    for (BSONObj::iterator i = names.begin(); i.more();) {
        result.append(i.next().str());
    }
    return result;
}

Json::Value rcar::getImageNamesFromBson(const mongo::BSONObj &root, 
        const std::string &key) {
    Json::Value result(Json::arrayValue);
    if (root.hasField(key)) {
        BSONObj names = root.getObjectField(key);
        for (BSONObj::iterator i = names.begin(); i.more();) {
            result.append(i.next().str());
        }
    }
    return result;
}


mongo::BSONArray rcar::getImagesNameFromJson(const Json::Value &names) {
    BSONArrayBuilder ids;
    for (int i = 0; i < (int)names.size(); i++) {
        ids.append(names[i].asString());
    }
    return ids.arr();
}


Json::Value rcar::getImageNamesFromBson(const mongo::BSONArray &root, 
        const std::string &key) {
    Json::Value names(Json::arrayValue);
    if (root.hasField(key)) {
        names = getImageNamesFromBson(root.getObjectField(key));
    }
    return names;
}
// remove images
void rcar::removeImages(const mongo::BSONObj &root, const std::string& key) {
}

