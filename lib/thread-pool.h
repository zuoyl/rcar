#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__ 

#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <queue>

namespace rcar {
    enum ThreadPoolState {
        POOL_INVALID,
        POOL_VALID,
        POOL_EXIT,
    };
    struct WorkItem {
        void *data;
        struct timeval time_queued;
    };
    struct ThreadPool {
        pthread_mutex_t m_pool_mutex;
        pthread_cond_t m_pool_cond;
        pthread_attr_t m_pool_attr;
        ThreadPoolState m_state;
        int m_max_threads;
        int m_thread_alive;
        int m_thread_idle;
        int m_idle_timeout;
        void (*handler)(void *);
        std::queue<WorkItem> m_queue;
    };

    class ThreadManager {
        public:
            static ThreadManager* get_instance();
            ThreadPool* alloc_threadpool(int max_threads, int idle_timeout, void (*handler)(void*));
            void destroy_threadpool(ThreadPool *threadPool);
            bool  dispatch(ThreadPool *threadpool, void *usr_data);
        private:
            ThreadManager(){}
            ~ThreadManager(){}
    };

}; // namespace 
#endif
