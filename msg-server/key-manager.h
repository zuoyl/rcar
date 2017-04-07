#ifndef _KEY_MANAGER_H_
#define _KEY_MANAGER_H_

#include <string>
#include "json/json.h"

namespace rcar { 
    class KeyManager {
        public:
            KeyManager(const std::string &to):m_to(to){} 
            KeyManager() = delete;
            ~KeyManager(){}
            std::string getApiKey(); 
            std::string getSecretKey();
        private:
            std::string m_to;
    };
}// namespace rcar
#endif //_HTTP_MANAGER_H_
