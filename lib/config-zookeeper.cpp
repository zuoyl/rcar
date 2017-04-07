#include "config-zookeeper.h"
#include "log.h"
#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

using namespace rcar;
 
bool ZooKeeperConfig::initialize(const std::string &hosts,
        const std::string &rootPrefix, std::vector<std::string> &optionNames) {
    _optionNames = optionNames;
    _rootPrefix = rootPrefix + "/";
    if (_zooKeeper.initialize(hosts)) {
        std::vector<std::string>::iterator ite = _optionNames.begin();
        for (; ite != _optionNames.end(); ++ite) {
            _zooKeeper.registerObserver(this, *ite);
        }
        return true;
    }
    return false;
}

const std::string ZooKeeperConfig::getGeneral(const char *key) {
    if (_options.find(key) != _options.end())
        return _options[key];
    else  {
        std::string optionName = _rootPrefix + key;
        _options[key] = _zooKeeper.getNode(optionName);
        return _options[key];
    }
}

const std::string ZooKeeperConfig::getGeneral(const char* key, const std::string &section) {
    if (_options.find(key) != _options.end())
        return _options[key];
    else  {
        std::string optionName = _rootPrefix + key;
        _options[key] = _zooKeeper.getNode(optionName);
        return _options[key];
    }
}

void ZooKeeperConfig::zooKeeperStateChanged(ZooKeeper *keeper, const std::string &path) {
    // no thread safe, using readlock
    // strip root prefix
    std::size_t pos1 = path.find(_rootPrefix);
    if (pos1 != std::string::npos) {
        std::string optionName = path.substr(pos1 + _rootPrefix.length() + 1);
        if (std::find(_optionNames.begin(), _optionNames.end(), optionName) != _optionNames.end()) {
            _options[optionName] = _zooKeeper.getNode(path);
        }
    }
}
