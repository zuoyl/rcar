#include "sys-config.h"
#include "lib/log.h"
#include "lib/config.h"
#include "lib/config-zookeeper.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>

struct rcar::CmdlineConfig::OptionItem _cmdlineOptions [] = {
    {"version", "v", false, NULL, "display version number"},
    {"config", "c", true, "rcar.conf", "config file for rcar server"}
};


using namespace rcar;

SystemConfig::SystemConfig() {}

SystemConfig::~SystemConfig() {
    if (_zooConfig) 
        delete _zooConfig;
}

SystemConfig* SystemConfig::getInstance() { 
    static SystemConfig SystemConfig;
    return &SystemConfig;
}

bool SystemConfig::create(int argc, char* argv[], const std::string &serverName) {
    try {
        if (serverName.empty())
            return false;
        _serverName = serverName;
        std::string configFile ="../conf/rcar.conf";	
        // parse command line and get options
        CmdlineConfig cmdlineConfig(_cmdlineOptions);
        if (cmdlineConfig.parse(argc, argv)) {
            configFile = cmdlineConfig.getGeneral("config"); 
        }

        // create user configuration
        if (configFile.empty())
            configFile = "../conf/rcar.conf";		

        UserConfig userConfig(configFile);

        // create zookeeper configuration
        std::string hosts = userConfig.getGeneral("zookeeper-hosts");
        std::vector<std::string> optionNames;
        optionNames.push_back("mongo-server");
        if (!hosts.empty()) {
            _zooConfig = new ZooKeeperConfig();
            std::string rootPrefix = "/" + _serverName;
            if (!_zooConfig->initialize(hosts, rootPrefix, optionNames)) {
                return false;
            }
        }
        // check all important options are set
        if (!loadConfigs()) {
            std::cout << "illegal configurations" << std::endl;
            return false;
        }
    }
    catch (...) {
        std::cout << "exception occured in creation of application" << std::endl;
        return false;
    }
    return true;
}

bool SystemConfig::loadConfigs() {
    std::string val = getGeneral("port");
    if (!val.empty())
        _port = atoi(val.c_str());
    else 
        return false;

    _logFile = getGeneral("log-file");
    if (_logFile.empty()) 
        return false;

    val = getGeneral("log-level");
    if (!val.empty())
        _logLevel = atoi(val.c_str());
    else 
        return false;
 
    _DBServer = getGeneral("mongo-server");
    if (_DBServer.empty())
        return false;
    return true;
}

std::string SystemConfig::getGeneral(const char *key) {
    return _zooConfig->getGeneral(key);
}

