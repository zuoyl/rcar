#include "log.h"
#include <pthread.h>
#include <fstream>
#include <cstdio>
#include <stddef.h>
#include <time.h>

using namespace rcar;

#define LOG_BUFSIZE 255
static pthread_mutex_t log_mutex_;
static int log_level_ = 0;;
static std::fstream log_file_;
static bool log_initialized_ = false;

static const char* log_level_text[] = {
    "Info",
    "Info",
    "Debug",
    "Warning",
    "Error",
};


static void log_write_buf(int level, const char *buf)
{
    if (level < LL_INFO1 || level > LL_ERROR) 
        return;
    // get local time
    time_t local_time;
    struct tm *time_block;
    time(&local_time);
    time_block = localtime(&local_time);
    char time_buf[64] = { 0 };
    sprintf(time_buf, "%d/%d/%d %d:%d:%d",
            time_block->tm_mon + 1,
            time_block->tm_mday,
            1900 + time_block->tm_year,
            time_block->tm_hour,
            time_block->tm_min,
            time_block->tm_sec);

    // get log level
    const char *log_hdr = log_level_text[level]; 

    // output
    pthread_mutex_lock(&log_mutex_);
    log_file_ << time_buf << " ";
    log_file_ << log_hdr << " ";
    log_file_ << buf;
    log_file_ << std::endl;
    log_file_.flush();
    pthread_mutex_unlock(&log_mutex_);
}


bool Log::initialize(std::string &file_name, int level)
{
    if (file_name.empty())
        return false;
    // open the log file
    log_file_.open(file_name.c_str(), 
            std::ios::out | std::ios::app);
    if (!log_file_.is_open())
        return false;
    // set current log level
    log_level_ = level;
    // create mutex
    pthread_mutex_init(&log_mutex_, NULL);

    log_initialized_ = true;
    return true;
}

void Log::info(int level, char* fmt, ...) 
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (level >= log_level_) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(level, buf);
        va_end(list);
    }
}

void Log::ll_info(const char* fmt, ...)
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (log_level_ <= LL_INFO1) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(LL_INFO1, buf);
        va_end(list);
    }
}
void Log::hl_info(const char* fmt, ...)
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (log_level_ <= LL_INFO2) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(LL_INFO2, buf);
        va_end(list);
    }
}
void Log::dbg(const char* fmt, ...) 
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (log_level_ <= LL_DBG) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(LL_DBG, buf);
        va_end(list);
    }
}
void Log::warning(const char* fmt, ...) 
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (log_level_ <= LL_WARNING) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(LL_WARNING, buf);
        va_end(list);
    }
}
void Log::error(const char* fmt, ...)
{
    char buf[LOG_BUFSIZE] = { 0 };
    va_list list;

    if (log_level_ <= LL_ERROR) {
        va_start(list, fmt);
        vsprintf(buf, fmt, list);
        log_write_buf(LL_ERROR, buf);
        va_end(list);
    }
}

void Log::finalize() 
{
    if (log_initialized_) {
        log_file_.close();
        pthread_mutex_destroy(&log_mutex_);
    }
    log_initialized_ = false;
}
