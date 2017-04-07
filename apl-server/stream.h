#ifndef __RCAR_STREAM_H_
#define __RCAR_STREAM_H_

#include <iostream>
#include <cstdio>

struct evbuffer;

namespace rcar {
    class Stream {
        public:
            enum mode {in, out};
            Stream(struct evbuffer *buf, int mode);
            ~Stream();
            Stream& operator << (const std::string &val);
            Stream& operator << (const char *val);
            Stream& operator << (int val);
            Stream& operator >> (std::string &val);
            Stream& operator >> (int &val);
            Stream& operator >> (char &val);
        private:
            struct evbuffer *m_buf;
            int m_buf_size;
            int m_mode;
    };

}; // namespace 
#endif // __RCAR_STREAM_H_
            
    
