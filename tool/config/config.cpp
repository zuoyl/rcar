
#include "zkeeper.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <string.h>
#include "config-file.h"

struct OptionItem {
    const char* key;
    const char* shortName;
    bool hasValue;
    const char* val;
    const char* description;
} optionItems [] = {
    {"host", "h", true, "127.0.0.1", "specify zookeeper host"},
    {"list", "l", false, nullptr, "list all items in config server"},
    {"file", "f", true, nullptr, "config file for config server"}
};

static std::map<std::string, std::string> _options;

void usage() {
    std::cout << "rcar-config usage:" << std::endl;
    for (int i = 0; i < sizeof(optionItems)/sizeof(optionItems[0]); i++) {
            struct OptionItem *item = &optionItems[i];
            std::cout << "  -" << item->key << " :\t" << item->description  << std::endl;
    }
}

static int isValidOptions(std::string &name) {
    int count = sizeof(optionItems)/sizeof(optionItems[0]); 
    for (int i = 0; i < count; i++) {
        if (strcmp(name.c_str(), optionItems[i].key) == 0) 
            return i;
        if (strcmp(name.c_str(), optionItems[i].shortName) == 0) {
            name = optionItems[i].key;
            return i;
        }
    }
    return -1;
}

static bool createOptions(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        char* item = argv[i];
        // skip symbol '-' to get option key
        std::string key = item + 1;
        // new option found
        if (item[0] == '-') {
            // check wether the option is validy
            int index = -1;
            if ((index = isValidOptions(key)) < 0)
                return false;

            // get option item's value
            std::string value;
            if (optionItems[index].hasValue) {
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
            if (_options.find(key) == _options.end())
                _options.insert(std::make_pair(key, value));
            else {
                return false;
            }
        }
    }
    return !_options.empty();
}

static std::string getOptionValue(const char *key) {
    if (_options.find(key) != _options.end()) {
        return _options[key];
    }
    return std::string("");
}

static bool isOptionExist(const std::string &key) {
    return !(_options.find(key) == _options.end());
}

static void listAllConfigItems(rcar::ZooKeeper &keeper);
static bool updateConfigItems(rcar::ZooKeeper &keeper, const std::string &file);

int main(int argc, char *argv[]) {
    if (!createOptions(argc, argv)) {
        std::cout << "syntax error" << std::endl;
        usage();
        return 0;
    }
    std::string hosts = getOptionValue("host");
    std::string configFile = getOptionValue("file");

    if (hosts.empty()) { 
        std::cout << "syntax error, hosts are not specfied" << std::endl;
        return 0;
    }

    rcar::ZooKeeper zkeeper;
    zkeeper.setDebugLevel(ZOO_LOG_LEVEL_WARN);
    if (!zkeeper.initialize(hosts)) {
        std::cout << "Failed to initialize zooKeeper" << std::endl;
        return 0;
    }
 
    // get option value
    if (isOptionExist("list")) {
        listAllConfigItems(zkeeper);
    } else if (!configFile.empty()) {
        if (!updateConfigItems(zkeeper, configFile)) 
            std::cout << "Failed to load all configuration items from server" << std::endl;
    } else 
        usage();

    return 0;
}

void listAllConfigItems(rcar::ZooKeeper &keeper) {
    std::cout << "### Configuration items for rcar" << std::endl;

    std::vector<std::string> serverList;
    serverList.push_back("rcar-apl-server");
    serverList.push_back("rcar-image-server");
    serverList.push_back("rcar-msg-server");


    std::vector<std::string> rootConfigs;
    keeper.getChildNode("/", rootConfigs);
    for (std::vector<std::string>::iterator i = rootConfigs.begin();
            i != rootConfigs.end(); i++) {

        if (find(serverList.begin(), serverList.end(), *i) == serverList.end())
            continue;

        std::cout << "Configuration for " << "/" << *i << std::endl;  

        std::vector<std::string> childConfigs;
        std::string childItem = "/" + *i;
        keeper.getChildNode(childItem, childConfigs);

        // get child configs
        for (std::vector<std::string>::iterator j = childConfigs.begin();
                j != childConfigs.end(); j++) {
            std::vector<std::string> childConfigs;
            std::string item = childItem + "/" + *j;
            std::string val = keeper.getNode(item);
            std::cout << "\t" << item << " = " << val << std::endl;
        }
    }
}

bool updateConfigItems(rcar::ZooKeeper &keeper, const std::string &file) {
    try {
        rcar::ConfigFile configFile(file);

        // get all section
        std::vector<std::string> sections;
        configFile.get_sections(sections);

        // for each section, add option 
        std::vector<std::string>::iterator i;
        for (i = sections.begin(); i != sections.end(); ++i) {
            std::string rootName = "/" + *i;
            // check wether section exist
            if (!keeper.isNodeExist(rootName)) {
                keeper.createNode(rootName, "rcar");
            }
            // for each key
            std::vector<std::string> keys;
            configFile.get_keys(*i, keys);
            std::vector<std::string>::iterator j;
            for (j = keys.begin(); j != keys.end(); ++j) {
                std::string val;
                configFile.get_string(*i, *j, val);
                if (!val.empty()) {
                    std::string item = rootName + "/" + *j; 
                    if (!keeper.isNodeExist(item))
                        keeper.createNode(item, val);
                    else 
                        keeper.setNode(item, val);
                }
            }
        }
        return true;
    } catch (...) {
        std::cout << "Failed to load configuration file: " << file << std::endl;
        return false;
    }

}

