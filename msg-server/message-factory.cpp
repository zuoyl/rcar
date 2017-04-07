#include "mongo/client/dbclient.h"
#include "json/writer.h"
#include "json/reader.h"
#include "lib/log.h"
#include "message-impl.h"
#include "message-factory.h"
#include "http-client.h"
#include "key-manager.h"
#include "lib/sys-config.h"

using namespace mongo;
using namespace rcar;


MessageFactory* MessageFactory::getInstance() {
    static MessageFactory factory;
    if (factory._mongoHost.empty())
        factory._mongoHost = SystemConfig::getInstance()->getDBServer();
    return &factory;
}
bool MessageFactory::initializeCert() {
    return true;
}

bool MessageFactory::checkParameterValidity(const Json::Value &msg) {
    if (msg.empty()) return false;

    // check method type
    int method = msg["method"].asInt();
    if (method >= rcar::MessageTypeMax) {
        Log::ll_info("message type is illegal");
        return false;
    }
    // check to
    std::string to = msg["to"].asString();
    if (to.empty() ||(to != "user" && to != "seller" && to != "group")) {
        if (to.empty())
            Log::ll_info("message target is null");
        else 
            Log::ll_info("message target(%s) is illegal", to.c_str());
        return false;
    }
    return true;
}

rcar::Message* MessageFactory::createMessage(const Json::Value &in) {
    Json::StyledWriter writer;
    std::string msg = writer.write(in["msg"]);;
    KeyManager* key = new KeyManager(in["to"].asString());
    std::cout << "\n" << "API Key" << key->getApiKey() << std::endl;
    std::cout << "\n" << "Secrety Key" << key->getSecretKey() << std::endl;

    Message *msgptr = nullptr;
 
    switch (in["method"].asInt()) {
        case MessageTypePushToAll: 
            msgptr = new PushToAllMessage(key, msg) ; 
            break;
        case MessageTypePushToSingle: 
            msgptr = new PushToSingleMessage(key, msg); 
            break;
        case MessageTypePushToBatch: 
            msgptr = new PushToBatchMessage(key, msg); 
            break;
        case MessageTypePushToGroup: 
            msgptr = new PushToGroupMessage(key, msg); 
            break;
        case MessageTypeCreateGroup: 
            msgptr = new CreateGroupMessage(key, msg); 
            break;
        case MessageTypeDeleteGroup: 
            msgptr = new DeleteGroupMessage(key, msg);
            break;
        case MessageTypeAddToGroup: 
            msgptr = new AddToGroupMessage(key, msg); 
            break;
        case MessageTypeDeleteFromGroup: 
            msgptr = new DeleteFromGroupMessage(key, msg); 
            break;
        default: 
            delete key;
            break;
    }
    return msgptr;
}

void MessageFactory::handleMessage(const char*body, int len) {
    // construct json object
    Json::Value msg;
    Json::Reader reader;
    // dump content type

    if (!reader.parse(body, body + len, msg) || !checkParameterValidity(msg)) {
        Log::ll_info("failed to parse message");
        return;
    }
    switch (msg["method"].asInt()) {
        case MessageTypePushToAll: handleMessagePushToAll(msg); break;
        case MessageTypePushToSingle: handleMessagePushToSingle(msg); break;
        case MessageTypePushToBatch: handleMessagePushToBatch(msg); break;
        case MessageTypePushToGroup: handleMessagePushToGroup(msg); break;
        case MessageTypeCreateGroup: handleMessageCreateGroup(msg); break;
        case MessageTypeDeleteGroup: handleMessageDeleteGroup(msg); break;
        case MessageTypeAddToGroup:  handleMessageAddToGroup(msg); break;
        case MessageTypeDeleteFromGroup: handleMessageDeleteFromGroup(msg); break;
        default: break;
    }
    postHandleMessage(msg);
    return;
}

void MessageFactory::postHandleMessage(const Json::Value &msg) {
}

void MessageFactory::request(rcar::Message *msg) {
    const std::string method = msg->httpmethod();
    HttpClient httpClient;
    if (method == "POST")
        httpClient.post(msg->url(), msg->format());
    else if (method == "GET")
        httpClient.get(msg->url(), msg->format());
    else {}
}

void MessageFactory::handleMessagePushToAll(const Json::Value &msg) {

}
void MessageFactory::handleMessagePushToSingle(const Json::Value &in) {
    std::string to = in["to"].asString();
    std::string userId = in["target"].asString();
    
    Message *msg = MessageFactory::createMessage(in);;
    msg->setPushType(e_push_type_someone);

    try {
        mongo::DBClientConnection c;
        c.connect("localhost");

        // construct query condition
        std::unique_ptr<mongo::DBClientCursor> cursor = c.query("rcar-base.session", 
                MONGO_QUERY("role" << to << "identifier" << userId));
        if (cursor->more()) {
            BSONObj obj = cursor->next();
            std::string push_channel_id = obj.getStringField("push_channel_id");
            std::string push_user_id = obj.getStringField("push_user_id");
            std::string device_type = obj.getStringField("device_type");

            if (push_channel_id.empty() || push_user_id.empty()) {
                Log::ll_info("user %s information does not exist\n", to.c_str());
                delete msg;
                return;
            }

            if (device_type == "ios") {
                msg->setMessageType(e_message_type_notice);
                msg->setDeviceType(e_device_type_ios);
            }
            else if (device_type == "android") {
                msg->setMessageType(e_message_type_message);
                msg->setDeviceType(e_device_type_android);
            }
            else {
                msg->setMessageType(e_message_type_message);
                msg->setDeviceType(e_device_type_android);
            }
            msg->setUserId(push_user_id);           
            msg->setChannelId(push_channel_id);
            request(msg);
        }
        delete msg;
        return;
    }
    catch (const mongo::DBException &e) {
        //Log::error("%s: exception occured with mongodb.\n", __func__);
        delete msg;
        return;
    }

}
void MessageFactory::handleMessagePushToBatch(const Json::Value &val) {
    std::string to = val["to"].asString();
    Json::Value targets = val["targets"];
    std::vector<std::string> ids;
    for (Json::Value::iterator i = targets.begin(); i != targets.end(); i++) { 
        std::string id = (*i).asString();
        ids.push_back(id);
    }
    
    Message *msg = MessageFactory::createMessage(val);;
    msg->setPushType(e_push_type_someone);

    try {
        mongo::DBClientConnection c;
        c.connect("localhost");

        // construct query condition
        std::unique_ptr<mongo::DBClientCursor> cursor = c.query("rcar-base.session", 
                MONGO_QUERY("role" << to << "identifier" << BSON("$in" << ids)));
        while (cursor->more()) {
            BSONObj obj = cursor->next();
            std::string push_channel_id = obj.getStringField("push_channel_id");
            std::string push_user_id = obj.getStringField("push_user_id");
            std::string device_type = obj.getStringField("device_type");

            if (device_type == "ios") {
                msg->setMessageType(e_message_type_notice);
                msg->setDeviceType(e_device_type_ios);
            }
            else if (device_type == "android") {
                msg->setMessageType(e_message_type_message);
                msg->setDeviceType(e_device_type_android);
            }
            else {
                msg->setMessageType(e_message_type_message);
                msg->setDeviceType(e_device_type_android);
            }
            msg->setUserId(push_user_id);           
            msg->setChannelId(push_channel_id);
            request(msg);
        }
        delete msg;
        return;
    }
    catch (const mongo::DBException &e) {
        //Log::error("%s: exception occured with mongodb.\n", __func__);
        delete msg;
        return;
    }
}

void MessageFactory::handleMessagePushToGroup(const Json::Value &msg) {

}
void MessageFactory::handleMessageCreateGroup(const Json::Value &msg) {
}
void MessageFactory::handleMessageDeleteGroup(const Json::Value &msg) {
}
void MessageFactory::handleMessageAddToGroup(const Json::Value &msg) {
}

void rcar::MessageFactory::handleMessageDeleteFromGroup(const Json::Value &msg) {

}



