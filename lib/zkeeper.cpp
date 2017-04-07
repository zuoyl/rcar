#include "zkeeper.h"
#include "log.h"
#include <iostream>

using namespace rcar;

ZooKeeper::~ZooKeeper(){
    zookeeper_close(_handle);
    _handle = nullptr;
    _observers.erase(_observers.begin(), _observers.end());
}

void ZooKeeper::zooWatcher(zhandle_t *handle, int type, int stat, 
        const char *path, void *context) {
    ZooKeeper *keeper = static_cast<ZooKeeper*>(context);
    // find observer
    std::map<const std::string, ZooKeeperObserver*>::iterator ite =
        keeper->_observers.find(path);
    if (ite != keeper->_observers.end()) {
       ZooKeeperObserver *observer = ite->second; 
       if (observer)
           observer->zooKeeperStateChanged(keeper, path);
    }
}

bool ZooKeeper::initialize(const std::string &host) {
    int timeout = 30000;
    
    _handle = zookeeper_init(host.c_str(), zooWatcher, timeout, 0, this, 0); 
    if (_handle == nullptr) {
        Log::error("Error when connecting to zookeeper server");
        return false;
    }
    return true;
}

bool ZooKeeper::setNode(const std::string &path, const std::string &content) {
    if (_handle != nullptr && !path.empty() && !content.empty()) {
        if (zoo_set(_handle, path.c_str(), content.data(), content.length(), -1) == ZOK) {
            return true;
        }
    }
    //Log::error("Error when creating path %s\n", path.c_str());
    std::cout << "Error when creating path:" <<  path << std::endl;
    return false;
}

bool ZooKeeper::createNode(const std::string &path, const std::string &content) {
    char buffer[64];
    if (_handle != nullptr && !path.empty() && !content.empty()) {
        if (zoo_create(_handle, path.c_str(), content.data(),
                    content.length(), &ZOO_OPEN_ACL_UNSAFE, 0,
                    buffer, sizeof(buffer)) == ZOK) {
            return true;
        }
    }
    Log::error("Error when creating path %s\n", path.c_str());
    return false;
}

void ZooKeeper::deleteNode(const std::string &path) {
    if (_handle != nullptr && !path.empty())
        zoo_delete(_handle, path.c_str(), -1);
}
const std::string ZooKeeper::getNode(const std::string &path) {
    char buffer[64] = {'\0'};
    int length = sizeof(buffer);
    if (_handle != nullptr && !path.empty()) {
        if (zoo_get(_handle, path.c_str(), 0, buffer, &length, nullptr) == ZOK) {
            return buffer;
        }
    }

    Log::error("Error when getting node %s\n", path.c_str());
    return std::string("");
}

bool ZooKeeper::isNodeExist(const std::string &path) {
    if (_handle != nullptr && !path.empty())
        if (zoo_exists(_handle, path.c_str(), 1, nullptr) == ZOK)
            return true;

    return false;
}


void ZooKeeper::getChildNode(const std::string &path, std::vector<std::string> &childs) {
    struct String_vector items;
    zoo_get_children(_handle, path.c_str(), 0, &items); 
    for (int index = 0; index < items.count; index++) {
        std::string child = *(items.data + index);
        childs.push_back(child);
    }
}

void ZooKeeper::registerObserver(ZooKeeperObserver *observer, const std::string &path) {
    if (_observers.find(path) == _observers.end()) {
        _observers.insert(std::make_pair(path,observer));
    }
}

void ZooKeeper::unregisterObserver(ZooKeeperObserver *observer) {
    std::map<const std::string, ZooKeeperObserver*>::iterator ite =
        _observers.begin();
    for (; ite != _observers.end(); ite++) {
        if (ite->second == observer) { 
            _observers.erase(ite);
            break;
        }
    }
}

void ZooKeeper::setDebugLevel(ZooLogLevel level) {
    zoo_set_debug_level(level);
}
