#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "request.h"
#include "json/json.h"
#include "error.h"
#include <string>
#include <stdexcept>

namespace rcar {
    class ResourceContext;

    class ResourceException : public std::runtime_error {
        public:
            explicit ResourceException(const std::string &except):
                std::runtime_error(except) {}
    };
 
    class Resource {
        public:
            explicit Resource(const char* name): m_name(name){}
            virtual ~Resource() {}
            virtual const char* name() { return m_name.c_str();}
            virtual int Get(const ResourceContext &context, Request &req) { return 400; }
            virtual int Post(const ResourceContext &context, Request &req) { return 400; }
            virtual int Put(const ResourceContext &context, Request &req) { return 400;}
            virtual int Delete(const ResourceContext &context, Request &req) { return 400; }
            virtual int Patch(const ResourceContext &context, Request &req) { return 400; }
        protected:
            void error(const char *method, const char *fmt, ...);
            void log(const char *method, const char *fmt, ...);
            std::string m_name;
    };

    typedef Resource* (*ResourceAllocator)();
    void register_resource(const char* uri, ResourceAllocator allocator); 
    void register_subsystem(const char* name, bool (*initializer)()); 

    #define resource_decl(class_name, uri) \
    void __attribute__((constructor)) do_resource_init_##class_name(void)\
    { \
        rcar::register_resource(uri, class_name::allocator); \
    }

    #define Resource_subsystem_init(name, init_method) \
    void __attribute__((constructor)) do_Resource_subsystem_##init_method(void) \
    { \
        rcar::register_subsystem(name, init_method); \
    }



}; // namespace 
#endif
