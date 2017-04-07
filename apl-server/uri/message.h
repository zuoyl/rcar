#ifndef _RCAR_MSG_PUBLISHER_H_
#define _RCAR_MSG_PUBLISHER_H_

#include <vector>
#include <string>
#include <json/json.h>

namespace rcar {
        // push msg to user or seller
        void pushMessageToUsers(const std::vector<std::string> &ids, const Json::Value &msg);
        void pushMessageToUser(const std:: string &id, const Json::Value &msg);
        void pushMessageToSellers(const std::vector<std::string> &ids, const Json::Value &msg); 
        void pushMessageToSeller(const std::string &id, const Json::Value &msg); 

         
        void pushMessageToGroup(const std::string &name, const Json::Value &msg);
        void pushMessageToAllSellers(const Json::Value &msg);
        void pushMessageToAllUsers(const Json::Value &msg);
        void createGroup(const std::string &name);
        void deleteGroup(const std::string &name);
        void createGroupWithUsers(const std::string &name, const Json::Value &users);
        void deleteUserFromGroup(const std::string &name, const Json::Value &users);
} // namespace rcar
#endif 
