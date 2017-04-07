#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "thread-pool.h"
#include "log.h"

using namespace rcar;

ThreadManager* ThreadManager::get_instance()
{
    static ThreadManager manager;
    return & manager;
}

void ThreadManager::destroy_threadpool(ThreadPool *threadpool)
{
    if (!threadpool || (threadpool->m_state != POOL_VALID)) 
        return;

    if (pthread_mutex_lock(&threadpool->m_pool_mutex) != 0) {
        Log::error("failed to lock mutex in thread manager\n");
        return;
    }
    threadpool->m_state = POOL_EXIT;

    /* wait for threads to exit */
    if (threadpool->m_thread_alive > 0) {
        if (pthread_cond_broadcast(&(threadpool->m_pool_cond)) != 0) {
            pthread_mutex_unlock(&threadpool->m_pool_mutex);
            return;
        }
    }
    while (threadpool->m_thread_alive > 0) {
        if (pthread_cond_wait(&threadpool->m_pool_cond, 
                    &threadpool->m_pool_mutex) != 0) {
            pthread_mutex_unlock(&threadpool->m_pool_mutex);
            return;
        }
    }
    if (pthread_mutex_unlock(&threadpool->m_pool_mutex) != 0) {
       Log::error("failed to unlock mutex in thread manager\n");
       return;
    }

    pthread_mutex_destroy(&(threadpool->m_pool_mutex));
    pthread_cond_destroy(&(threadpool->m_pool_cond));
    pthread_attr_destroy(&(threadpool->m_pool_attr));
    delete threadpool;
    return;
}

ThreadPool* ThreadManager::alloc_threadpool(
        int max_threads, int idle_timeout, void (*handler)(void *))
{
    ThreadPool *threadpool = new ThreadPool();
    threadpool->m_max_threads = max_threads;
    threadpool->m_thread_alive = 0; 
    threadpool->m_thread_idle = 0; 
    threadpool->m_idle_timeout = idle_timeout;
    threadpool->handler = handler; 
    pthread_mutex_init(&(threadpool->m_pool_mutex), NULL);
    if (pthread_cond_init(&(threadpool->m_pool_cond), NULL) != 0) {
        delete threadpool;
        return NULL;
    }

    if (pthread_attr_init(&(threadpool->m_pool_attr)) != 0) {
        delete threadpool;
        return NULL;
    }

    if (pthread_attr_setdetachstate(&(threadpool->m_pool_attr), 
                PTHREAD_CREATE_DETACHED) != 0) {
        delete threadpool;
        return NULL;
    }
    threadpool->m_state = POOL_VALID;

    return threadpool;
}

static void *thread_manager_worker(void *arg)
{
    ThreadPool *threadpool = static_cast<ThreadPool*>(arg);
    bool must_exit = false;
    /* loop looking for work */
    for (;;) {
        if (pthread_mutex_lock(&(threadpool->m_pool_mutex)) != 0) { 
            /* fatal error */
            Log::error("!Fatal: mutex lock failed\n");
            break;
        }
        struct timespec timeout;
        timeout.tv_sec = time(NULL) + threadpool->m_idle_timeout;
        timeout.tv_nsec = 0;
        threadpool->m_thread_idle++;
        while ((threadpool->m_queue.empty()) 
                && (threadpool->m_state != POOL_EXIT)) { 
            /* Sleep, awaiting wakeup */ 
            int retval = pthread_cond_timedwait(&(threadpool->m_pool_cond),
                &(threadpool->m_pool_mutex), &timeout); 
            if (retval == ETIMEDOUT) { 
                must_exit = true;
                break;
            }
        }
        WorkItem item = threadpool->m_queue.front();
        threadpool->m_queue.pop();
        threadpool->m_thread_idle--;
        if (threadpool->m_state == POOL_EXIT) 
            must_exit = true;

        if (pthread_mutex_unlock(&(threadpool->m_pool_mutex)) != 0) {
            /* fatal error */
            Log::error("fatal: mutex unlock failed\n");
            break;
        }
        if (item.data) {
            threadpool->handler(item.data); 
        } else if (must_exit) 
            break;             
    }

    if (pthread_mutex_lock(&(threadpool->m_pool_mutex)) != 0) {
        /* fatal error */
        Log::error("fatal: mutex lock failed\n");
        return NULL;
    }
    threadpool->m_thread_alive--;
    if (threadpool->m_thread_alive == 0) {
        /* signal that all threads are finished */
        pthread_cond_broadcast(&threadpool->m_pool_cond); 
    } 
    if (pthread_mutex_unlock(&(threadpool->m_pool_mutex)) != 0) {
        /* fatal error */
        Log::error("fatal: mutex unlock failed\n");
        return NULL;
    }
    return NULL;
}

bool ThreadManager::dispatch(ThreadPool *threadpool, void *usr_data)
{
    if (!threadpool) {
        return false;
    }

    // lock the threadpool 
    if (pthread_mutex_lock(&(threadpool->m_pool_mutex)) != 0) {
        Log::error("!mutex lock failed\n");
        return false;
    }

    if (threadpool->m_state != POOL_VALID) {
        if (pthread_mutex_unlock(&(threadpool->m_pool_mutex)) != 0) {
            Log::error("!mutex unlock failed\n");
            return false;
        }
        return false;
    }
    WorkItem item;
    item.data = usr_data;
    // add timer here
    threadpool->m_queue.push(item);
    if ((threadpool->m_thread_idle == 0) && 
            (threadpool->m_thread_alive < threadpool->m_max_threads)) { 
        // start a new thread 
        pthread_t thr_id;
        if (pthread_create(&thr_id, &(threadpool->m_pool_attr),
                thread_manager_worker, threadpool) != 0) {
            Log::error("!pthread_create failed\n");
        } else {
            threadpool->m_thread_alive++; 
        }
    }
    pthread_cond_signal(&(threadpool->m_pool_cond)); 

    if (pthread_mutex_unlock(&(threadpool->m_pool_mutex)) != 0) {
        Log::error("!mutex unlock failed\n");
        return false;
    }
    return true;
}
