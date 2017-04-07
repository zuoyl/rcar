#include <limits.h>
#include <cstdio>
#include <string.h>
#include "stream.h"
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/http_struct.h>

using namespace rcar;

Stream::Stream(struct evbuffer *buf, int mode) 
    :m_buf(buf),m_mode(mode){}

Stream::~Stream(void)
{}

Stream& Stream::operator << (const std::string &val)
{
    if (!val.empty())
        evbuffer_add(m_buf, val.c_str(), val.size());
    return *this;
}
Stream& Stream::operator << (const char *val)
{
    if (val)
        evbuffer_add(m_buf, val, strlen(val));
    return *this;

}
Stream& Stream::operator << (int val)
{
    evbuffer_add(m_buf, &val, sizeof(val));
    return *this;
}

Stream& Stream::operator >> (std::string &val)
{
    return *this;
}
Stream& Stream::operator >> (int &val)
{
    return *this;
}
Stream& Stream::operator >> (char &val)
{
    return *this;
}
