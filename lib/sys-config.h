#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include <string>
#include <vector>
#include "lib/config.h"
#include "lib/config-zookeeper.h"

namespace rcar {
    class SystemConfig { 
        public:
            static SystemConfig* getInstance();
            bool create(int argc, char *argv[], const std::string &serverName);
            std::string getGeneral(const char *key);
            int getPort() const { return _port;}
            int getLogLevel() const { return _logLevel;};
            const std::string getLogFile() const  { return _logFile; }
            const std::string getDBServer() const { return _DBServer;}
        private:
            SystemConfig();
            ~SystemConfig();
            bool loadConfigs();
        private:
            std::string _serverName;
            std::string _DBServer;
            std::string _logFile;
            int _logLevel;
            int _port;
            ZooKeeperConfig *_zooConfig;
    };
}; // namespace
#endif // _SYS_CONFIG_H_
