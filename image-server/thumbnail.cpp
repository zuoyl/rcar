#include "handler.h"
#include "Epeg.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

static int max_dimension = 0; // > 0 means we reduce max(w,h) to max_dimension, with aspect preserved
static int inset_flag = 0; // Ensure specified dimensions will be covered
static int thumb_quality = 85; // Quality value from 1 to 100
static char *thumb_comment = NULL;

void rcar::create_image_thumbnail(unsigned char*data, long len, 
        int thumb_height, int thumb_width,
        unsigned char **thumbnail_data, int *thumbnail_len) {
   int c;
   char *p;

   Epeg_Image *im = epeg_memory_open(data, len);
   if (im) {
       Epeg_Thumbnail_Info info;
       int w, h;
       
       const char *com = epeg_comment_get(im);
       epeg_thumbnail_comments_get(im, &info);
       epeg_size_get(im, &w, &h);
       if (thumb_width < 0) { // This means we want %thumb_width of w
           thumb_width = w * (-thumb_width) / 100;
       }
       if (thumb_height < 0) { // This means we want %thumb_height of h
           thumb_height = h * (-thumb_height) / 100;
       }

       if (max_dimension > 0) {
           if (w > (h ^ inset_flag)) {
               thumb_width = max_dimension;
               thumb_height = max_dimension * h / w;
           } else {
               thumb_height = max_dimension;
               thumb_width = max_dimension * w / h;
           }
       } else if (inset_flag) {
           thumb_width = MAX(thumb_width, thumb_height * w / h);
           thumb_height = MAX(thumb_height, thumb_width * h / w);
       }
   
       epeg_decode_size_set(im, thumb_width, thumb_height);
       epeg_quality_set(im, thumb_quality);
       epeg_thumbnail_comments_enable (im, 1);
       epeg_comment_set(im, thumb_comment);
       epeg_memory_output_set(im, thumbnail_data, thumbnail_len);
       epeg_encode(im);
       epeg_close(im);
   }
   return;
}
