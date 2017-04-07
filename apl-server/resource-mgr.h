#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_

#include <string>
#include <vector>
#include <map>
#include <event2/http.h>
#include "resource.h"

namespace rcar {
    enum ResourceFlag {
        TF_DISABLED,
        TF_ENABLED
    };

    class Request;
    class Resource;
    class ResourceManager {
        public:
            static ResourceManager* getInstance();
            bool initialize(struct evhttp *http);
            Resource* createResource(Request &req);
            bool hasResource(const std::string &name);
            void enableResource(const std::string &name);
            void disableResource(const std::string &name);
            bool isResourceEnabled(const std::string &name);
            void dumpAllResources();
            void registerResource(const char *name, ResourceAllocator allocator);
            void registerSubsystem(const char *name, bool (*subsytem_init)());
            typedef bool (*SubsystemInitializer)();
        private:
            bool load_Resources();
            ResourceManager();
            ~ResourceManager();
            void handleRequest(Request &req);
        private:
            std::map<const char*, ResourceAllocator> m_allocators;
            std::map<const char*, int> m_ctrls;
            std::map<const char*, SubsystemInitializer> m_subsystem_init_list;
            evhttp* m_http;
            static void httpCallback(struct evhttp_request *req, void *arg);
    };
}; // namespace 
#endif
