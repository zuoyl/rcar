//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include "handler.h"


using namespace rcar;
using namespace mongo;


void rcar::handleImageDownload(evhttp_request *req, ImageContext &context) {
    const char* uri = evhttp_request_get_uri(req);
    // all key and vals
    std::string imageName = get_image_id(req);
    std::string size = get_image_size(req);
    std::string target = get_image_target(req);
    std::string thumbnail = get_image_thumbnail(req);
    std::string libname;
    std::string collection;

    // check target parameter and image name
    if (target != "seller" && target != "user" && 
        target != "common" && imageName.empty()) {
        Log::error("uri%s has illegal format %s\n", uri, target.c_str());
        evhttp_send_error(req, 404, "image was not found");
        return;
    }
    if (!size.empty()) {
        if (size != "32x32" && size != "64x64") {
            Log::error("uri%s has illegal thumbnial sizeformat %s\n", uri, target.c_str());
            evhttp_send_error(req, 404, "image was not found");
            return;
        }
    }
    // construct image document and collection 
    if (thumbnail.empty() && size.empty()) { 
        // if thumbnail and size fileld is not specified
        // client ontly want to get full image
        libname = "rcar-images-" + target;
        collection = "fs";
    } else {
        libname = "rcar-thumbnails-" + target;
        if (size == "32x32" || size == "64x64") 
            collection = size;
        else  {
            evhttp_send_error(req, 404, "image was not found");
            Log::error("uri%s has illegal format %s\n", uri, target.c_str());
            return;
        }
    }

    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // get file
        mongo::GridFS gridfs(c, libname, collection);
        mongo::GridFile file = gridfs.findFileByName(imageName);

        if (file.exists()) { // image file exists
            struct evbuffer *buffer = evbuffer_new();
            int numberChunks = file.getNumChunks();
            for (int i = 0; i < numberChunks; i++) {
                mongo::GridFSChunk chunk = file.getChunk(i);
                int len = chunk.len();
                const char *data = chunk.data(len);
                evbuffer_add(buffer, data, len);
            }
            evhttp_add_header(req->output_headers, "Content-Type", "image/jpeg");
            evhttp_send_reply(req, 200, "OK", buffer);
            evbuffer_free(buffer);
            return;

        } else if (!thumbnail.empty() && !size.empty()){ 
            // check to see wether the full image exist
            // create thumbnail if full image exists
            libname = "rcar-images-" + target;
            mongo::GridFS gridfs2(c, libname, "fs");
            mongo::GridFile file = gridfs2.findFileByName(imageName);

            if (file.exists()) { // image file exists
                struct evbuffer *buffer = evbuffer_new();
                // alloc memory by content length
                gridfs_offset content_len = file.getContentLength();
                unsigned char* dataptr = new unsigned char[content_len];
 
                int numberChunks = file.getNumChunks();
                gridfs_offset cursor = 0;
                for (int i = 0; i <= numberChunks; i++) {
                    mongo::GridFSChunk chunk = file.getChunk(i);
                    int len = chunk.len();
                    const char *data = chunk.data(len);
                    memcpy(dataptr + cursor, data, len);
                    cursor += len;
                }
                int w = 32;
                int h = 32;
                if (size == "64x64")  { w = h = 64; }
                // create thumbnail
                int thumbnail_len;
                unsigned char *thumbnail_ptr = NULL; 
                create_image_thumbnail(dataptr, content_len, h, w, &thumbnail_ptr, &thumbnail_len);

                if (thumbnail_ptr != NULL && thumbnail_len > 0) {
                    
                    evbuffer_add(buffer, thumbnail_ptr, thumbnail_len);
                    // save thumbnail into gridfs
                    libname = "rcar-thumbnails-" + target;
                    mongo::GridFS gridfs_thumbnail(c, libname, size);
                    gridfs_thumbnail.storeFile((const char *)thumbnail_ptr, 
                            thumbnail_len, imageName, "image/jpeg");

                    // send data to client
                    evhttp_add_header(req->output_headers, "Content-Type", "image/jpeg");
                    evhttp_send_reply(req, 200, "OK", buffer);
                }
                free(thumbnail_ptr);
                evbuffer_free(buffer);
                return;
            }
            evhttp_send_error(req, 404, "image was not found");
            return;
        }
        evhttp_send_error(req, 404, "image was not found");
        return;
     } catch (const mongo::DBException &e) {
         evhttp_send_error(req, 404, "mongodb exception occurred");
         Log::ll_info("%s:exception occured with mongodb \n", __func__);
         return;
    }
}
