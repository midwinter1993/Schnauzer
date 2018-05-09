#include "Tid.h"
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <map>

#include "Control.h"
#include "common/common.h"
#include "common/Event.h"
#include "common/debug.h"

namespace Tid {

static __thread tid_t __my_tid = UNDEFINED_TID;
static std::atomic<tid_t> __tid_counter{0};
static std::map<pthread_t, tid_t> __pthread_id_table;
static bool __alive_thread_table[MAX_NR_THREAD];

tid_t freshTid() {
    return __tid_counter.fetch_add(1, std::memory_order_seq_cst);
}

tid_t getNewTid() {
    Control::globalLock();
    if (Config::getRuntimeConfig()->getBool("REUSE_TID")) {
        for (tid_t i = 0; i < getNrThread(); ++i) {
            if (!__alive_thread_table[i]) {
                __my_tid = i;
                break;
            }
        }
    }
    if (__my_tid == UNDEFINED_TID) {
        __my_tid = freshTid();
    }

    __pthread_id_table[pthread_self()] = __my_tid;
    __alive_thread_table[__my_tid] = true;
    Control::globalUnlock();

    return __my_tid;
}

tid_t getCurrTid() {
    assert(__my_tid != UNDEFINED_TID && __my_tid < MAX_NR_THREAD);
    return __my_tid;
}

tid_t lookupTid(pthread_t tid) {
    Control::globalLock();
    auto it = __pthread_id_table.find(tid);
    assert(it != __pthread_id_table.end() && "Lookup tid failure");
    Control::globalUnlock();
    return it->second;
}

size_t getMaxNrThread() {
    return MAX_NR_THREAD;
}

size_t getNrThread() {
    return __tid_counter;
}

void registerCurrThread() { getNewTid(); }

void unregisterCurrThread() {
    Control::globalLock();
    __alive_thread_table[getCurrTid()] = false;
    Control::globalUnlock();
}

bool isThreadAlive(tid_t tid) {
    Control::globalLock();
    bool ret = __alive_thread_table[tid];
    Control::globalUnlock();
    return ret;
}

static __thread bool __meta_thread = false;
bool isMetaThread() {
    return __meta_thread;
}

void setMetaThread() {
    __meta_thread = true;
}

void dumpAllThreads() {
    Control::globalLock();
    for (auto pr : __pthread_id_table) {
        Console::info("--- pthread id: %lu: tid: %d---\n", pr.first, pr.second);
        Debug::dumpThreadStacktrace(pr.first);
    }
    Control::globalUnlock();
}

}
