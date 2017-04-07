#ifndef _CONFIG_IMP_H_
#define _CONFIG_IMP_H_

#include <string>
#include <map>
#include <stdexcept>
#include "config-file.h"

namespace rcar {
    class ConfigException : public std::runtime_error {
        public:
            explicit ConfigException(const std::string &except):
                std::runtime_error(except) {}
    };
 
    class Configuration {
        public:
            virtual ~Configuration(){}
            virtual const std::string getGeneral(const char* key) = 0;
            virtual const std::string getGeneral(const char* key, const  std::string &section) = 0;
    };

    class UserConfig : public Configuration {
        public:
            UserConfig(const std::string& configFile);
            ~UserConfig();
            const std::string getGeneral(const char* key);
            const std::string getGeneral(const char* key, const  std::string &section);
        private:
            ConfigFile *m_configFile;
    };

    class EnvironmentConfig : public Configuration {
        public:
            EnvironmentConfig();
            ~EnvironmentConfig();
            const std::string getGeneral(const char* key);
            const std::string getGeneral(const char* key, const  std::string &section);
    };

    class CmdlineConfig : public Configuration {
        public:
            struct OptionItem {
                const char* key;
                const char* shortName;
                bool hasValue;
                const char* defaultValue;
                const char* description;
            };
        public:
            CmdlineConfig(OptionItem *optionItems):m_optionItems(optionItems){}
            CmdlineConfig() = delete;
            ~CmdlineConfig() {}
            bool parse(int argc, char *argv[]);     
            const std::string getGeneral(const char* key);
            const std::string getGeneral(const char* key, const  std::string &section);
        private:
            int indexOfValidOption(std::string &name);

            struct OptionItem *m_optionItems;
            std::map<std::string, std::string> m_options;
    };

}  // namespace  
#endif // _CONFIG_IMP_H_
