#include "config.h"
#include <stdlib.h>

using namespace rcar;

EnvironmentConfig::EnvironmentConfig()
{}

EnvironmentConfig::~EnvironmentConfig()
{}

static const std::string 
getStringVariable(const std::string &key) {
    char* val = getenv(key.c_str());
    if (!val) return val;
    else return std::string("");
}

#if 0
static bool get_int_variable(const std::string &val, 
        unsigned int min, unsigned int max, int &ret)
{
    char* result = getenv(val.c_str());
    if (!result) 
        return false;

    int tval = atoi(result);
    if (tval < 0)
        ret = min;
    else if ((unsigned)tval > max)
        ret = max;
    else if ((unsigned)tval < min)
        ret = min;
    else
        ret = tval;
    return true;
}
static bool get_bool_variable(const std::string &val, bool &result)
{
    char* tval = getenv(val.c_str());
    if (!tval)
        return false;
    std::string item = tval;
    if (item == "true")
        result = true;
    else if (item == "false")
        result = false;
    else
        throw new ConfigurationError("Invalid option value");
    return true;
}
#endif 
const std::string EnvironmentConfig::getGeneral(const char *key) {
    return getStringVariable(key);
}
const std::string EnvironmentConfig::getGeneral(const char *key, const std::string &section) {
    return std::string("");
}
