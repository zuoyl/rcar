#include <map>
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include "message.h"
#include "json/value.h"
#include "lib/md5.h"
#include <sstream>

using namespace rcar;

static std::string formatJsonWithUrlEncoded(Json::Value &data) {
    std::string val;
    for(Json::Value::iterator i = data.begin(); i != data.end(); ++i) {
        std::string key = i.key().asString();
        if (i != data.begin()) val.append( "&" );
        val.append(key);
        val.append("=");
        if(data[key].type() == Json::uintValue 
          || data[key].type() == Json::intValue) {
            std::stringstream stream;
            stream << data[key].asUInt();
            val.append(stream.str());
        } else  {
            val.append(data[key].asString());
        }
    }
    return val;
}

            
#define CONTENT_MAX_SIZE 512
const std::string Message::format(const char *contentType) { 
    std::string apiKey = m_key->getApiKey();
    Json::Value data;
    data["apikey"] = apiKey;
    data["timestamp"] = (unsigned int)time(NULL);
    data["device_type"] = m_deviceType;
    data["deploy_status"] = 1; 
    serialize(data);
    // format content
    char content[CONTENT_MAX_SIZE] = {0x00, };
    if (m_messageType == e_message_type_message) {
        if (m_deviceType == e_device_type_ios) {
            sprintf(content, "{\"aps\":{\"alert\":\"hello\"},\"param\":%s}", m_message.c_str());
            data["msg"] = content;
        } else {
            // android
        }
    } else {
        if (m_deviceType == e_device_type_ios) {
            sprintf(content, "{\"aps\":{\"alert\":\"hello\"},\"param\":%s}", m_message.c_str());
            data["msg"] = content;
        } else {
            // android
        }
    }
    data["sign"] = sign(data);
    std::string val = formatJsonWithUrlEncoded(data);

    std::cout << "Message dump:" << std::endl 
              << "url:" << url() <<std::endl
              << "messate content" << std::endl << val << std::endl;
    return val;
}

std::string rcar::Message::sign(const Json::Value &data) {
    std::string secretKey = m_key->getSecretKey();
    std::string val;
    for(Json::Value::iterator i = data.begin(); i != data.end(); ++i) {
        std::string key = i.key().asString();
        val.append(key);
        val.append("=");
        if(data[key].type() == Json::uintValue 
          || data[key].type() == Json::intValue) {
            std::stringstream stream;
            stream << data[key].asUInt();
            val.append(stream.str());
        } else  {
            val.append(data[key].asString());
        }
    }
    std::string m;
    m.append(httpmethod());
    m.append(url());
    m.append(val );
    m.append(secretKey);

    std::cout << "Message before sign:" << std::endl << m << std::endl;
    MD5 md5(encodeUrl(m));

    return md5.md5();
}
unsigned char Message::to_hex(unsigned char x) { 
    return  x > 9 ? x + 55 : x + 48; 
}

std::string Message::encodeUrl(const std::string &str) {
    std::string ret;
    unsigned int length = str.length();
    for ( unsigned int i = 0; i < length; i++ ) {
        if ( isalnum( (unsigned char)str[i]) || (str[i] == '-') || 
                      (str[i] == '_') || (str[i] == '.') || (str[i] == '~') )
            ret += str[i];
        else if ( str[i] == ' ' )
            ret += "+";
        else {
            ret += '%';
            ret += to_hex( (unsigned char)str[i] >> 4 );
            ret += to_hex( (unsigned char)str[i] % 16 );
        }
    }
    return ret;
}
