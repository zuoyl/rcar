#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>

namespace rcar {
    class ConfigFileException : public std::runtime_error {
        public:
                explicit ConfigFileException(const std::string &except):
                    std::runtime_error(except) {}
    };

    class ConfigFile {
        public:
            ConfigFile(const std::string &file);
            ~ConfigFile();

            void get_sections(std::vector<std::string> &sections);
            bool get_keys(const std::string &section, std::vector<std::string> &keys);
            bool get_string(const std::string &section,
                           const std::string &key,
                           std::string &val);
            bool get_strings(const std::string &section,
                            const std::string &key,
                            std::vector<std::string> &val);
            bool get_int(const std::string &section,
                        const std::string &key,
                        int &val);
            bool get_ints(const std::string &section,
                         const std::string &key,
                         std::vector<int> &val);
            bool get_bool(const std::string &section,
                         const std::string &key,
                         bool &result);
        private:
            typedef std::map<std::string, std::string> Section;
            std::map<std::string, Section*> m_sections;
            std::string m_path;
            std::string m_file;
            std::ifstream m_ifile;
        private:
            void analyze();
            Section* get_section(char *buf, std::string &sectionName);
            bool get_key_and_value(char *buf, std::string &key, std::string &val);
    };

    }; //namespace 
#endif // _CONFIG_FILE_H_
