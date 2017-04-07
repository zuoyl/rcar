#ifndef _BAIDU_MESSAGE_H_
#define _BAIDU_MESSAGE_H_

#include <iostream>
#include "message.h"

namespace rcar {
    
    class PushToBatchMessage : public Message{
        public:
            PushToBatchMessage() = delete;
            PushToBatchMessage(KeyManager *key, const std::string& message) :
                Message(key, message){}
            ~PushToBatchMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };

    class PushToSingleMessage : public Message{
        public:
            PushToSingleMessage() = delete;
            PushToSingleMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~PushToSingleMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "single_device"; }
        protected:
            void serialize(Json::Value &v) {
                v["channel_id"] = m_channelId;
                v["msg_type"] = m_messageType;
            }
    };

    class PushToAllMessage : public Message {
        public:
            PushToAllMessage() = delete;
            PushToAllMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~PushToAllMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };

    class PushToGroupMessage : public Message {
        public:
            PushToGroupMessage() = delete;
            PushToGroupMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~PushToGroupMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };

    class CreateGroupMessage : public Message{
        public:
            CreateGroupMessage() = delete;
            CreateGroupMessage(KeyManager* key, const std::string& message)
                :Message(key, message){}
            ~CreateGroupMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };

    class AddToGroupMessage : public Message{
        public:
            AddToGroupMessage() = delete;
            AddToGroupMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~AddToGroupMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };
    class DeleteGroupMessage : public Message {
        public:
            DeleteGroupMessage() = delete;
            DeleteGroupMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~DeleteGroupMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };


    class DeleteFromGroupMessage : public Message {
        public:
            DeleteFromGroupMessage() = delete;
            DeleteFromGroupMessage(KeyManager* key, const std::string& message):
                Message(key, message){}
            ~DeleteFromGroupMessage() {}
            const std::string httpmethod() { return "POST"; }
            const std::string cclass() { return "push"; }
            const std::string method() { return "batch_device"; }
        protected:
            void serialize(Json::Value &v) {}
    };


    
} // namespace rcar 


#endif
