//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(Template, "user/template");

int Template::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Template::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Template::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Template::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Template::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
