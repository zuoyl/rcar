#ifndef _PUSH_MESSAGE_H_
#define _PUSH_MESSAGE_H_
#include <assert.h>
#include <map>
#include <string>
#include <sstream>
#include "json/json.h"
#include "key-manager.h"

namespace rcar { 
   enum e_message_type {
        e_message_type_message = 0,
        e_message_type_notice,
        e_message_type_max
    };

    enum e_push_type { 
        e_push_type_someone = 1,
        e_push_type_group,
        e_push_type_anyone,
        e_push_type_max
    };
    
    enum e_device_type {
        e_device_type_browser = 1,
        e_device_type_pc,
        e_device_type_android,
        e_device_type_ios,
        e_device_type_window_phone,
        e_device_type_max
    };

    typedef std::pair<std::string, std::string> PAIR;
    class Message {
        public:
            Message() = delete;
            Message(KeyManager *key, const std::string& message): 
                m_key(key),m_message(message){}
            virtual ~Message() { delete m_key;}
            virtual const std::string httpmethod() = 0;
            virtual const std::string cclass() = 0;
            virtual const std::string method() = 0;
            const std::string format(const char* contentType = "application/json");
            const std::string url() {
                return "http://api.tuisong.baidu.com/rest/3.0/" + cclass() + "/" + method();;
            }
            void setTitle(const std::string &title) { m_noticeTitle = title; }
            void setDescription(const std::string &desc) { m_noticeDescription = desc; }
            void setMessage(const std::string &msg) { m_message = msg; }
            const std::string& getTitle() const  {  return m_noticeTitle; }
            const std::string& getDescription()const { return m_noticeDescription; }
            const std::string& getMessage()const { return m_message;}
            void setPushType(e_push_type push_type) { m_pushType = push_type; }
            void setDeviceType(e_device_type device_type ) { m_deviceType = device_type; }
            void setMessageType(e_message_type message_type) { m_messageType = message_type; }

            void setTag(const std::string &tag) { 
                assert(m_pushType == e_push_type_group);
                m_tag = tag; 
            }

            void setUserId(const std::string &user_id){ 
                assert( m_pushType == e_push_type_someone );
                m_userId = user_id; 
            };
             void setChannelId(const std::string &channel_id) { 
                assert(m_pushType == e_push_type_someone ); 
                m_channelId = channel_id; 
            };
            void setChannelIds(std::vector<std::string> ids) { 
                assert(m_pushType == e_push_type_someone ); 
                std::vector<std::string>::iterator i = ids.begin();
                for (;i != ids.end(); ++i)
                    m_channelIds.append(*i); 
            };
 
       private:
            std::string encodeUrl(const std::string &str); 
            unsigned char to_hex(unsigned char x); 
            std::string sign(const Json::Value &data); 
        protected: 
            virtual void serialize(Json::Value &val) = 0; 
            std::string m_tag;
            std::string m_userId;
            std::string m_channelId;
            Json::Value m_channelIds;
            e_push_type m_pushType; 
            e_device_type m_deviceType; 
            e_message_type m_messageType; 
 
            std::string m_noticeTitle;
            std::string m_noticeDescription; 
            KeyManager *m_key;
            std::string m_message; 
    };
}// namespace rcar
#endif //_MESSAGE_H_
