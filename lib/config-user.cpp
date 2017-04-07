#include "config.h"
#include "config-file.h"
#include <limits.h>
#include <assert.h>
#include <algorithm>

using namespace rcar;
static std::string GENERAL_SECTION = "general";

UserConfig::UserConfig(const std::string& config_file) {
    try {
        m_configFile = new ConfigFile(config_file);
    } catch (...) {
        throw ConfigException("config file missed");
    }
}

UserConfig::~UserConfig() {
    if (m_configFile)
        delete m_configFile;
    m_configFile = nullptr;
}

const std::string UserConfig::getGeneral(const char *key) {
    std::string val;
    m_configFile->get_string(GENERAL_SECTION, key, val);
    return val;
}

const std::string  UserConfig::getGeneral(const char* key, const std::string &section) {
    std::string val;
    m_configFile->get_string(section, key, val);
    return val;
}
