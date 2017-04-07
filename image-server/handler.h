#ifndef _IMAGES_HANDLER_H_
#define _IMAGES_HANDLER_H_

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/util.h>
#include "json/json.h"
#include "lib/log.h"
#include "error.h"
#include "mongo/client/dbclient.h"
#include "mongo/bson/bsonobj.h"
#include "lib/base64.h"
#include <string>
#include "context.h"

namespace rcar {
    bool initialize_image_handle();
    // download and upload handler
    void handleImageUpload(evhttp_request *req, ImageContext &context); 
    void handleImageDownload(evhttp_request *req, ImageContext &context);
    // utils
    void dump_http_request(evhttp_request *req);
    std::string get_image_id(evhttp_request *req);
    std::string get_image_target(evhttp_request *req);
    std::string get_image_size(evhttp_request *req);
    std::string get_image_thumbnail(evhttp_request *req);

    Json::Value create_in_json_object(evhttp_request *req);
    Json::Value create_out_json_object(evhttp_request *req);

    // create thumbnail
    void create_image_thumbnail(unsigned char*data, long len, 
            int thumbnail_height, int thumbnail_with,
            unsigned char **thumbnail_data, int *thumbnail_len);
}
#endif
