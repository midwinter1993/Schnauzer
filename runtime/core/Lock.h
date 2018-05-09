#ifndef __RUNTIME_CORE_LOCK_H__
#define __RUNTIME_CORE_LOCK_H__

#include <pthread.h>
#include <atomic>
#include <mutex>

#define Lock MutexLock

class PLock {
public:
    PLock() { pthread_mutex_init(&lk_, NULL); }

    void lock() { pthread_mutex_lock(&lk_); }

    void unlock() { pthread_mutex_unlock(&lk_); }

private:
    pthread_mutex_t lk_;
};

class MutexLock {
public:
    void lock() { lk_.lock(); }

    void unlock() { lk_.unlock(); }

private:
    std::mutex lk_;
};

class SpinLock {
public:
    void lock() {
        while (f.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() { f.clear(std::memory_order_release); }

private:
    std::atomic_flag f = ATOMIC_FLAG_INIT;
};

#endif /* ifndef __RUNTIME_UTILS_LOCK_H__ */
