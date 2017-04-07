#include "key-manager.h"

using namespace rcar;

static const std::string CarUserApiKey = "lRsBpphYIr8elR0Yi4lXlG3H";
static const std::string CarUserSecretKey = "H06EYUkVXPah53vSgCN0ubeY5fssDPvt";
static const std::string CarSellerApiKey = "7UtGbOMybZt2XvgRDSjMQg8R";
static const std::string CarSellerSecretKey = "SIEOiEugGk06w7cwzhLjDd6HId2M2KRg";
 
std::string KeyManager::getApiKey() {
    if (m_to == "user") return CarUserApiKey;
    else return CarSellerApiKey;
}

std::string KeyManager::getSecretKey() {
    if (m_to == "user") return CarUserSecretKey;
    else return CarSellerSecretKey;
}
