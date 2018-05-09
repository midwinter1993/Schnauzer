#include "Control.h"
#include <stdlib.h>
#include <atomic>
#include <mutex>

// static Lock __global_lock;

namespace Control {

#define USE_GLOBAL_TICK

static std::recursive_mutex __global_lock;

void globalLock() {
    __global_lock.lock();
}

void globalUnlock() {
    __global_lock.unlock();
}

static std::atomic_bool __is_finished(false);
static std::atomic_bool __is_started(false);

bool isStarted() {
    return __is_started;
}

void setStarted() {
    __is_started = true;
}

bool isFinished() {
    return __is_finished;
}

void setFinished() {
    __is_finished = true;
}

#if (__x86_64__)
static std::atomic_ullong __global_tick;
#else
static std::atomic_long __global_tick;
#endif

timestamp_t getGlobalTick() {
    return __global_tick.load(std::memory_order_seq_cst);
}

timestamp_t incGlobalTick() {
    return __global_tick.fetch_add(1, std::memory_order_seq_cst);
}

}
