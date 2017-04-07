#include "resource.h"
#include "lib/log.h"
using namespace rcar;


void Resource::error(const char *method, const char *fmt, ...) {
    char buf[128] = { 0 };
    va_list list;
    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    Log::error("<%s>%s:%s", m_name.c_str(), method, buf); 
}

void Resource::log(const char *method, const char *fmt, ...) {
    char buf[128] = { 0 };
    va_list list;
    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    Log::hl_info("<%s>%s:%s", m_name.c_str(), method, buf); 
}

