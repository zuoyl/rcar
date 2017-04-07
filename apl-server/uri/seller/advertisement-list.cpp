//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(AdvertisementList, "seller/advertisement-list");

int AdvertisementList::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int AdvertisementList::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int AdvertisementList::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int AdvertisementList::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int AdvertisementList::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
