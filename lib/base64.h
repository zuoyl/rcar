#include "modp_b64.h"
#include <string>

inline std::string& Base64Encode(std::string& s) {
    std::string x(modp_b64_encode_len(s.size()), '\0');
    int d = modp_b64_encode(const_cast<char*>(x.data()), s.data(), s.size());
    x.erase(d, std::string::npos);
    s.swap(x);
    return s;
}

inline std::string& Base64Decode(std::string& s) {
    std::string x(modp_b64_decode_len(s.size()), '\0');
    int d = modp_b64_decode(const_cast<char*>(x.data()), s.data(), s.size());
    if (d < 0) {
        x.clear();
    } else {
        x.erase(d, std::string::npos);
    }
    s.swap(x);
    return s;
}


