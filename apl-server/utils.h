#ifndef __RCAR_UTILS_H_
#define __RCAR_UTILS_H_

#include <string>
#include "message.h"
#include <math.h>
#include "json/value.h"
#include "json/reader.h"

namespace rcar {
    double get_short_distance(double lat1, double lon1, 
            double lat2, double lon2);
    inline bool time_is_early_than(const std::string &targt, const std::string &src) {
        return true;
    }
    inline bool time_is_late_than(const std::string &targt, const std::string &src) {
        return true;
    }

    #define EARTH_RADIUS  (6378.137)
    #define PI (3.1415926535)

    inline double rad(double d) { return d * PI / 180.0; }
    inline double round(double d) { return floor(d + 0.5); }

    inline double get_distance(double lat1, double lng1, double lat2, double lng2) {
        double radLat1 = rad(lat1);
        double radLat2 = rad(lat2);
        double a = radLat1 - radLat2;
        double b = rad(lng1) - rad(lng2);
        double s = 2 * asin(sqrt(pow(sin(a/2),2) +cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
        s = s * EARTH_RADIUS;
        s = round(s * 10000) / 10000;
        return s;
    }
    bool encode_base64_file(const std::string &file_name, std::string &result);
    bool decode_base64_file(const std::string &data, std::string &file_name);
    bool decode_base64_data(const std::string &data, std::string &result);
    std::string get_linearstring_time(); //YYMMDDHHMMSS
    std::string get_customstring_time();//YY-MM-DD HH:MM:SS
    std::string convertIntToString(int value);

};

#endif
