#ifndef _ZOO_CONFIG_H_
#define _ZOO_CONFIG_H_

#include <string>
#include <vector>
#include "zkeeper.h"
#include "config.h"

namespace rcar {
    class ZooKeeperConfig : public Configuration, ZooKeeperObserver  { 
        public:
            ZooKeeperConfig(){}
            ~ZooKeeperConfig(){}
            bool initialize(const std::string &hosts, 
                    const std::string & rootPrefix, std::vector<std::string> &optionNames);
            const std::string getGeneral(const char* key);
            const std::string getGeneral(const char* key, const  std::string &section);
            void zooKeeperStateChanged(ZooKeeper *keeper, const std::string &path); 
        private:
            ZooKeeper _zooKeeper;
            std::vector<std::string> _optionNames;
            std::map<std::string, std::string> _options;
            std::string _rootPrefix;
    };
}; // namespace
#endif // _ZOO_CONFIG_H_
