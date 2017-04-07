#include "config-file.h"
#include <cstdlib>
#include <utility>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "log.h"

using namespace rcar;

#define STREAM_SIZE 255    


void to_lower(std::string &val)
{
//  std::transform(val.begin(), val.end(), val.begin(), tolower);
}

ConfigFile::ConfigFile(const std::string &file)
{
    m_file = file;
    analyze();
}

ConfigFile::~ConfigFile()
{
    std::map<std::string, Section*>::iterator ite = m_sections.begin();
    for (; ite != m_sections.end(); ite++)
        delete ite->second;
    m_sections.clear();

    m_ifile.close();
}

// path = /etc
// file = rcar.conf
void ConfigFile::analyze()
{
    // open config file, throw exception if failed
    m_ifile.open(m_file.c_str());
    if (!m_ifile.good()) {
        throw ConfigFileException("Open file failed");
        return;
    }

    // get all sections 
    char buf[STREAM_SIZE];
    memset(buf, 0, sizeof(buf));
    Section* cur_section = NULL;
    std::string cur_section_name;
    int line_no = 0;
    while (m_ifile.getline(buf, STREAM_SIZE)) {
        line_no++;
        // if new section is found, the last section should be checked
        if (*buf == '[') {
            // a new section is created
            cur_section = get_section(buf, cur_section_name);
            if (!cur_section) {
                throw ConfigFileException("section error");
                return;
            }
            // if the last section exist, save it into sections
            // if there are two same sections, exception should be thrown
            if (m_sections.find(cur_section_name) != m_sections.end()) {
                delete cur_section;
                throw ConfigFileException("section error");
                return;
            }
            else
                m_sections.insert(std::make_pair(cur_section_name, cur_section));
        }
        else {
            // current section must not be null
            if (!cur_section) {
                throw ConfigFileException("no section");
                return;
            }
            // get key and value from current section
            std::string key, val;
            if (get_key_and_value(buf, key, val)) {
                // check wheater there are two same keys
                if (cur_section->find(key) == cur_section->end())
                    cur_section->insert(std::make_pair(key, val));
            }
            else {
                Log::warning("invalid configuration item found(lineno = %d\n",
                        line_no);
                continue;
            }
        }
    } // while(file.getline...)
    memset(buf, 0, sizeof(buf));
}

static void strip(std::string &val, const std::string &patten)
{
    size_t start = val.find_first_not_of(patten);
    size_t end = val.find_last_not_of(patten);
    val = val.substr(start, end - start + 1);
}

static void split(std::string &val, 
                  const std::string &patten, 
                  std::vector<std::string> &result)
{
    char* sub_val = strtok((char*)val.c_str(), patten.c_str());
    while (sub_val) {
        result.push_back(sub_val);
        sub_val = strtok(NULL, patten.c_str());
    }
}

static void split(std::string &val, 
                  const std::string &patten, 
                  std::vector<int> &result)
{
    char* sub_val = strtok((char*)val.c_str(), patten.c_str());
    while (sub_val) {
        result.push_back(atoi(sub_val));
        sub_val = strtok(NULL, patten.c_str());
    }
}

ConfigFile::Section* ConfigFile::get_section(char *buf, std::string &section_name)
{
    int length = strlen(buf); 
    if (buf[0] != '[' || buf[length- 1] != ']')
        return NULL;
    section_name = std::string(buf, 1, length - 2);
    strip(section_name, " ");
    if (section_name.empty()) {
        throw ConfigFileException("section error");
        return NULL;
    }
    return new Section();
}

bool ConfigFile::get_key_and_value(char *buf, std::string &key, std::string &val)
{
    std::string content = buf;
    size_t pos = content.find_first_of("=");
    if (pos == std::string::npos) 
        return false;
    key = content.substr(0, pos);
    val = content.substr(pos + 1, content.size() - pos - 1);
    strip(key, " ");
    strip(val, " ");

    return true;
}

void ConfigFile::get_sections(std::vector<std::string> &sections)
{
    std::map<std::string, Section*>::iterator ite = m_sections.begin();
    for (; ite != m_sections.end(); ite++) 
        sections.push_back(ite->first);
}

bool ConfigFile::get_keys(const std::string &sectionName,
                      std::vector<std::string> &keys)
{
    if (m_sections.find(sectionName) == m_sections.end()) 
        return false;

    Section* section = m_sections[sectionName];
    std::map<std::string, std::string>::iterator ite = section->begin();
    for (; ite != section->end(); ite++)
        keys.push_back(ite->first);

    return true;
}

bool ConfigFile::get_string(const std::string &section_name,
                        const std::string &key,
                        std::string &val)
{
    if (m_sections.find(section_name) != m_sections.end()) {
        Section* section = m_sections[section_name];
        if (section->find(key) != section->end()) {
            val = (*section)[key];
            return true;
        }
    }

    return false;
}

bool ConfigFile::get_strings(const std::string &section,
                         const std::string &key,
                         std::vector<std::string> &val)
{ 
    std::string content;
    if (!get_string(section, key, content))
        return false;
    split(content, ";", val);

    return true;
}

bool ConfigFile::get_int(const std::string &section,
                     const std::string &key,
                     int &val_int )
{
    std::string val;
    if (!get_string(section, key, val))
        return false;
    val_int = (unsigned int)atoi(val.c_str());

    return true;
}

bool ConfigFile::get_ints(const std::string &section,
                      const std::string &key,
                      std::vector<int> &val)
{
    std::string content;
    if (!get_string(section, key, content))
        return false;
    split(content, ";", val);
    return true;
}

bool ConfigFile::get_bool(const std::string &section,
                      const std::string &key,
                      bool &result)
{
    std::string val;
    if (!get_string(section, key, val))
        return false;
    if (val == "true")
        result = true;
    else if (val == "false")
        result = false;
    else
        return false;

    return true;
}
