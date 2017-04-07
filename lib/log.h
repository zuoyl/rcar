#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <cstdarg>

namespace rcar {
    enum {
        LL_INFO1,
        LL_INFO2,
        LL_DBG,
        LL_WARNING,
        LL_ERROR
    };

    class Log { 
        public:
            static bool initialize(std::string &file_name, int level);
            static void info(int level, char* fmt, ...); 
            static void ll_info(const char* fmt, ...);
            static void hl_info(const char* fmt, ...);
            static void dbg(const char* fmt, ...); 
            static void warning(const char* fmt, ...); 
            static void error(const char* fmt, ...);
            static void finalize(); 
    };
}; // namespace
#endif // _LOG_H_
