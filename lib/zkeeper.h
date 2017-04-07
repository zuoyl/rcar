#ifndef _ZKEEPER_H_
#define _ZKEPPER_H_

#include <string>
#include <map>
#include <cstdarg>
#include <vector>
#include "zookeeper/zookeeper.h"

namespace rcar {
    class ZooKeeper;
    class ZooKeeperObserver {
        public:
            virtual void zooKeeperStateChanged(ZooKeeper *keeper, const std::string &path) = 0; 
    };
    class ZooKeeper { 
        public:
            ZooKeeper():_handle(nullptr){}
            ~ZooKeeper();
            void setDebugLevel(ZooLogLevel level);
            bool initialize(const std::string &host);
            bool createNode(const std::string &path, const std::string &content);
            bool setNode(const std::string &path, const std::string &content);
            void getChildNode(const std::string &path, std::vector<std::string> &childs);
            void deleteNode(const std::string &path);
            const std::string getNode(const std::string &path);
            bool isNodeExist(const std::string &path);
            void registerObserver(ZooKeeperObserver *observer, const std::string &path);
            void unregisterObserver(ZooKeeperObserver *observer);
        private:
            static void zooWatcher(zhandle_t *handle, int type, int stat, 
                    const char *path, void *context); 
        private:
            zhandle_t *_handle;
            std::map<const std::string, ZooKeeperObserver*> _observers;
    };
}; // namespace
#endif // _ZKEPPER_H_
