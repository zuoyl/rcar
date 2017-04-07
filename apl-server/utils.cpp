#include "utils.h"
#include <math.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <sstream>
#include "mongo/client/dbclient.h"
#include<dirent.h>
#include "db.h"


#define TWO_PI 6.28318530712 //2*PI
#define PI_180 0.01745329252 //PI/180

using namespace boost::uuids;
namespace archive  = boost::archive;

bool rcar::encode_base64_file(const std::string &file_name, std::string &result) {
    // open the file and read out all data
    std::ifstream ifs(file_name, std::ios::in|std::ios::binary);
    if (!ifs.is_open())
        return false;
    size_t size = ifs.tellg();
    char *buf = new char[size];
    ifs.read(buf, size);
    ifs.close();
    
    typedef archive::iterators::base64_from_binary<
        archive::iterators::transform_width<const char*, 6, 8> > base64_enc;
    std::stringstream os;
    std::copy(base64_enc(buf), base64_enc(buf + size),
            std::ostream_iterator<char>(os));
    result = os.str();
    delete []buf;
    return true;
}

// decode data encoded as base64 and save the result into file
bool rcar::decode_base64_file(const std::string &data, std::string &file_name) {
    // decode data data
    std::stringstream os;
    typedef archive::iterators::transform_width<
        archive::iterators::binary_from_base64<const char*>, 8, 6, char > base64_dec;
    std::copy(base64_dec(data.c_str()), base64_dec(data.size()),
            std::ostream_iterator<char>(os));
    // create the file and save data
    std::ofstream file(file_name, std::ofstream::out | std::ofstream::binary);
    if (!file.is_open())
        return false;
    file.write(os.str().c_str(), os.str().size());
    file.close();
    return true;
}

struct is_base64_char {
    bool operator()(char x) {
        return boost::is_any_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+/=")(x);
    }
};

bool rcar::decode_base64_data(const std::string &data, std::string &result) {
    // decode data data
    std::stringstream os;
    typedef archive::iterators::transform_width<
            archive::iterators::binary_from_base64<
                std::string::const_iterator>, 8, 6> base64_dec;
    try {
        std::copy(base64_dec(data.begin()), base64_dec(data.end()),
                std::ostream_iterator<char>(os));
        result = os.str();
        return true;
    } catch (...) {
        return false;
    }
}

std::string rcar::get_linearstring_time() {
    using namespace boost::posix_time;
    std::stringstream ss;

    const ptime now = second_clock::local_time();
    time_facet* facet = new time_facet("%Y%m%d%H%M%S");
    ss.imbue(std::locale(ss.getloc(),facet));
    ss << now;
    //delete facet;
    return ss.str();
}

std::string rcar::get_customstring_time() {
    using namespace boost::posix_time;
    std::stringstream ss;

    const ptime now = second_clock::local_time();
    time_facet* facet = new time_facet("%Y-%m-%d %H:%M:%S");
    ss.imbue(std::locale(ss.getloc(),facet));
    ss << now;
    //delete facet;
    return ss.str();
}

std::string rcar::convertIntToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

double rcar::get_short_distance(double lat1, double lon1, double lat2, double lon2) {
    double rad_lon1 = lon1 * PI_180;
    double rad_lat1 = lat1 * PI_180;
    double rad_lon2 = lon2 * PI_180;
    double rad_lat2 = lat2 * PI_180;
    double dis_rad_lon = rad_lon1 - rad_lon2;

    if (dis_rad_lon > PI)
        dis_rad_lon = TWO_PI - dis_rad_lon;
    else if (dis_rad_lon < -PI)
        dis_rad_lon = TWO_PI + dis_rad_lon;
    else {}

    double dx = EARTH_RADIUS * cos(rad_lat1) * dis_rad_lon;
    double dy = EARTH_RADIUS * (rad_lat1 - rad_lat2);
    double distance = sqrt(dx*dx + dy*dy);

    return distance;
}
