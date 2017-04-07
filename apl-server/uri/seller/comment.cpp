//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "headers.h"

namespace seller {

DECL_RESOURCE_CLASS(Comment, "seller/comment");

int Comment::Get(const ResourceContext &context, Request &req) {
    return Resource::Get(context, req);
}
int Comment::Post(const ResourceContext &context, Request &req) {
    return Resource::Post(context, req);
}
int Comment::Put(const ResourceContext &context, Request &req) { 
    return Resource::Put(context, req);
}

int Comment::Delete(const ResourceContext &context, Request &req) {
    return Resource::Delete(context, req); 
}

int Comment::Patch(const ResourceContext &context, Request &req) { 
    return Resource::Patch(context,req); 
}

} // namespace 
