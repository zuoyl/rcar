//
//  Copyright (c) 2014 Cloud Stone Technology. All rights reserved.
//

#include "handler.h"

using namespace mongo;


void rcar::handleImageUpload(evhttp_request *req, ImageContext &context) {
    Json::Value in = create_out_json_object(req);;
    const char* uri = evhttp_request_get_uri(req);
    if (in.empty()) {
        Log::error("failed to create json object for uri%s\n", uri);
        return;
    }

    std::string target = in["target"].asString();

    // check parameter
    if (target != "seller" && target != "user") {
        Log::error("uri%s has illegal format %s\n", uri, target.c_str());
        return;
    }


    try {
        mongo::DBClientConnection c;
        c.connect(context.getMongo());

        // construct image lib 
        std::string libname = "rcar-images";
        if (!target.empty()) 
            libname += "-" + target;

        mongo::GridFS gridfs(c, libname);

        // get all image object
        Json::Value images = in["images"];
        for (unsigned int i = 0; i < images.size(); i++) { 
            Json::Value image = images[i];
            std::string name = image["name"].asString();
            std::string data = image["data"].asString();
            std::string outImage = Base64Decode(data);
            gridfs.storeFile(outImage.data(), outImage.size(), name, "image/jpg");

            // create thumbnail
            libname = "rcar-thumbnails-" + target;

            int thumbnail_len;
            unsigned char *thumbnail_ptr = NULL; 
            // create 32x32 size thumbnail
            create_image_thumbnail((unsigned char *)outImage.data(), outImage.size(), 
                    32, 32, &thumbnail_ptr, &thumbnail_len);

            if (thumbnail_ptr != NULL && thumbnail_len > 0) {
                mongo::GridFS gridfs_thumbnail(c, libname, "32x32");
                gridfs_thumbnail.storeFile((const char *)thumbnail_ptr, thumbnail_len, name, "image/jpeg");
            }
            free(thumbnail_ptr);

            // create 64x64 thumbnail
            create_image_thumbnail((unsigned char *)outImage.data(), outImage.size(), 
                    64, 64, &thumbnail_ptr, &thumbnail_len);
            if (thumbnail_ptr != NULL && thumbnail_len > 0) {
                mongo::GridFS gridfs_thumbnail(c, libname, "64x64");
                gridfs_thumbnail.storeFile((const char *)thumbnail_ptr, thumbnail_len, name, "image/jpeg");
            }
            free(thumbnail_ptr);
            evhttp_send_reply(req, 200, "OK", NULL);

        }
     } catch (const mongo::DBException &e) {
         Log::ll_info("%s:exception occured with mongodb \n", __func__);
    }
}
