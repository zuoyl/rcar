//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(ServiceList, "seller/service-list");

int ServiceList::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int ServiceList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int ServiceList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int ServiceList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int ServiceList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
