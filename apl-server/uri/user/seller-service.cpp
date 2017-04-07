//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace user {

DECL_RESOURCE_CLASS(SellerService, "user/seller-service");

int SellerService::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int SellerService::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int SellerService::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int SellerService::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int SellerService::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
