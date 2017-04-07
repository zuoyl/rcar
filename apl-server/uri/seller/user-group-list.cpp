//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(UserGroupList, "seller/user-group-list");

int UserGroupList::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int UserGroupList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int UserGroupList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int UserGroupList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int UserGroupList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
