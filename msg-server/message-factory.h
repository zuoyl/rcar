#ifndef _RCAR_MESSAGE_FACTORY_H_
#define _RCAR_MESSAGE_FACTORY_H_

#include "message.h"
#include "json/json.h"

namespace rcar {
    enum {
        MessageTypePushToAll,
        MessageTypePushToSingle,
        MessageTypePushToBatch,
        MessageTypePushToGroup,
        MessageTypeCreateGroup,
        MessageTypeDeleteGroup,
        MessageTypeAddToGroup,
        MessageTypeDeleteFromGroup,
        MessageTypeMax
    };

    class MessageFactory {
        public:
            static MessageFactory* getInstance();
            bool initializeCert();
            void handleMessage(const char *body, int len);
        private:
            MessageFactory(){}
            ~MessageFactory(){}
            static Message* createMessage(const Json::Value &in);

            bool checkParameterValidity(const Json::Value &msg); 
            void postHandleMessage(const Json::Value &msg);
            void request(rcar::Message *msg);
            void handleMessagePushToAll(const Json::Value &msg);
            void handleMessagePushToSingle(const Json::Value &msg);
            void handleMessagePushToBatch(const Json::Value &msg);
            void handleMessagePushToGroup(const Json::Value &msg);
            void handleMessageCreateGroup(const Json::Value &msg);
            void handleMessageDeleteGroup(const Json::Value &msg);
            void handleMessageAddToGroup(const Json::Value &msg);
            void handleMessageDeleteFromGroup(const Json::Value &msg);
        private:
            std::string _mongoHost;
    };
} // namespace rcar

#endif

