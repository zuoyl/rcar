#include <dirent.h>
#include <dlfcn.h>
#include <ostream>
#include "resource-mgr.h"
#include "resource.h"
#include "lib/log.h"
#include "stream.h"
#include "request.h"
#include <cassert>
#include <cstdarg>
#include "event2/http.h"
#include "event2/event.h"
#include "json/json.h"
#include "lib/sys-config.h"
#include "resource-context.h"

using namespace rcar;

void ResourceManager::httpCallback(struct evhttp_request *req, void *arg) {
    // ResourceManager is not thread safe, it will be improved later
    try {
        rcar::Request request(req);
        rcar::ResourceManager *manager = rcar::ResourceManager::getInstance();
        manager->handleRequest(request);
    } catch (...) {

    }
}

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
static void default_api_cb(struct evhttp_request *req, void *arg) {
	const char *uri = evhttp_request_get_uri(req);
    rcar::Log::hl_info("invalid request:%s", uri);
	evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "OK", NULL);
}



ResourceManager::ResourceManager()
{}

ResourceManager::~ResourceManager()
{}
// not thread safe
ResourceManager * ResourceManager::getInstance(){
    static ResourceManager factory;
    return &factory;
}

bool ResourceManager::initialize(struct evhttp *http) {
    // check Resource file path
#if 0
    if (path.empty()) {
        Log::error("Resource file path is null\n");
        return false;
    }
#endif 
    m_http = http;

    // initialize all subsystem
    std::map<const char*, SubsystemInitializer>::iterator ite;
    for (ite = m_subsystem_init_list.begin();
         ite != m_subsystem_init_list.end(); ite++) {
        const char *name = ite->first;
        SubsystemInitializer initializer = ite->second;
        if (!initializer()) {
            Log::error("failed to initialize subsystem(%s)\n", name);
            return false;
        }
    }
    // registre all Resources to uri
    std::map<const char*, ResourceAllocator>::iterator it;
    for (it = m_allocators.begin(); 
         it != m_allocators.end(); it++) {
        std::string name = "/rcar/api/v1.0/";
        name += it->first;
        evhttp_set_cb(m_http, name.c_str(), ResourceManager::httpCallback, NULL);
    }
    // disallowed method
    evhttp_set_allowed_methods(m_http, 
            EVHTTP_REQ_POST|EVHTTP_REQ_GET|
            EVHTTP_REQ_DELETE|EVHTTP_REQ_PUT|
            EVHTTP_REQ_PATCH);
 
    // We want to accept arbitrary requests, so we need to set a "generic"
	// cb.  We can also add callbacks for specific paths.
	evhttp_set_gencb(http, default_api_cb, NULL);

    // set content type



    return true;
}
void ResourceManager::handleRequest(Request &req) {
    int method = req.get_method();
    int result = 400;
    Resource *resource = createResource(req);
    if (!resource) {
        return req.finish(result);
    }
    SystemConfig *config = SystemConfig::getInstance();
    ResourceContext context;
    context.setMongo(config->getDBServer());
    switch (method) {
        case Request::HTTPMethodGet:
            result = resource->Get(context, req);
            break;
        case Request::HTTPMethodPost:
            result = resource->Post(context, req);
            break;

        case Request::HTTPMethodPut:
            result = resource->Put(context, req);
            break;

        case Request::HTTPMethodDelete:
            result = resource->Delete(context, req);
            break;

       case Request::HTTPMethodPatch:
            result = resource->Patch(context, req);
            break;
        default:
            break;
    }
    req.finish(result);
}

void ResourceManager::dumpAllResources() {
    int index = 0;
    std::map<const char*, ResourceAllocator>::iterator ite;

    Log::ll_info("ResourceManager dump:\n");
    for (ite = m_allocators.begin(); 
         ite != m_allocators.end(); ite++) 
        Log::ll_info("(%d)\t%s\n", index++, ite->first);
}

Resource* ResourceManager::createResource(Request &req) {
    std::string api_name = req.getResourceName();
    if (!api_name.empty()) {
    //    if (m_trans.find(api_name.c_str()) != m_trans.end())
    //        return m_trans[api_name.c_str()];
        std::map<const char*, ResourceAllocator>::iterator ite;
        for (ite = m_allocators.begin(); 
             ite != m_allocators.end(); ite++) {
            if (api_name == ite->first) {
                ResourceAllocator allocator = ite->second;
                return allocator();
            }
        }
    }
    return nullptr;
}

bool ResourceManager::hasResource(const std::string &name) {
    return (m_allocators.find(name.c_str()) != m_allocators.end());

}
void ResourceManager::enableResource(const std::string &name) {
    if (m_allocators.find(name.c_str()) != m_allocators.end()) 
        m_ctrls[name.c_str()] = TF_ENABLED;
}
void ResourceManager::disableResource(const std::string &name) {
    if (m_allocators.find(name.c_str()) != m_allocators.end()) 
        m_ctrls[name.c_str()] = TF_DISABLED;
}

bool ResourceManager::isResourceEnabled(const std::string &name) {
    if (m_allocators.find(name.c_str()) != m_allocators.end()) 
            return (m_ctrls[name.c_str()] == TF_ENABLED);
    return false;

}

void ResourceManager::registerResource(const char *uri,
        ResourceAllocator allocator) {
    // it should check wether there are same Resources
    m_allocators.insert(std::make_pair(uri, allocator));
}
void ResourceManager::registerSubsystem(
        const char *name, SubsystemInitializer initializer) {
    m_subsystem_init_list.insert(std::make_pair(name, initializer));
}

void rcar::register_resource(const char *name, 
        ResourceAllocator allocator) {
    ResourceManager *factory = ResourceManager::getInstance();
    factory->registerResource(name, allocator);
}
void rcar::register_subsystem(const char *name, 
        ResourceManager::SubsystemInitializer initializer) {
    ResourceManager *factory = ResourceManager::getInstance();
    factory->registerSubsystem(name, initializer);
}

