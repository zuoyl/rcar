#ifndef _HEADERS_H_
#define _HEADERS_H_

#include "error.h"
#include "message.h"
#include "resource-context.h"
#include "resource.h"
#include "utils.h"
#include "mongo/client/dbclient.h"
#include "mongo/db/json.h"
#include "db.h"

using namespace rcar;
using namespace mongo;

#define DECL_RESOURCE_CLASS(resource, uri) \
    class resource : public rcar::Resource { \
        public: \
            resource():Resource(uri){} \
            ~resource(){} \
            static Resource* allocator() { \
                return new resource(); \
            }\
            int Get(const ResourceContext &context, Request &req);\
            int Post(const ResourceContext &context, Request &req);\
            int Put(const ResourceContext &context, Request &req);\
            int Delete(const ResourceContext &context, Request &req);\
            int Patch(const ResourceContext &context, Request &req);\
    };\
    resource_decl(resource, uri);\


#endif
