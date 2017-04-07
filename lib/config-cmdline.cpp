#include "config.h"
#include <algorithm>
#include <string.h>
using namespace rcar;

#if 0
CmdlineConfig::OptionItem optionItems [] = {
    {"version", "v", false, NULL, "Display version number"},
    {"path", "P", true, NULL, "Work path for dlna"},
    {"config-file", "c", true, "rcar.conf", "config file for rcar server"}
};
#endif

int CmdlineConfig::indexOfValidOption(std::string &name) {
    int count = sizeof(m_optionItems)/sizeof(m_optionItems[0]); 
    for (int i = 0; i < count; i++) {
        if (strcmp(name.c_str(), m_optionItems[i].key) == 0) 
            return i;
        if (strcmp(name.c_str(), m_optionItems[i].shortName) == 0) {
            name = m_optionItems[i].key;
            return i;
        }
    }
    return -1;
}

bool CmdlineConfig::parse(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        char* item = argv[i];
        // skip symbol '-' to get option key
        std::string key = item + 1;
        // new option found
        if (item[0] == '-') {
            // check wether the option is validy
            int index = -1;
            if ((index = indexOfValidOption(key)) < 0)
                return false;

            // get option item's value
            std::string value;
            if (m_optionItems[index].hasValue) {
                while (++i < argc) {
                    if (argv[i][0] == '-') {
                        i--;
                        break;
                    }
                    if (value.empty())
                        value = argv[i];
                    else {
                        value += ";";
                        value += argv[i];
                    }
                }
            }
            // check wether there are two sampe options
            if (m_options.find(key) == m_options.end())
                m_options.insert(std::make_pair(key, value));
            else {
                return false;
            }
        }
    }
    return true;
}

const std::string CmdlineConfig::getGeneral(const char *key) {
    std::string val;
    if (m_options.find(key) != m_options.end()) {
        val =  m_options[key];
    }
    return val;
}

const std::string CmdlineConfig::getGeneral(const char *key, const std::string &section) {
    return std::string("");
}

