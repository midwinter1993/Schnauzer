#include "Logger.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <list>
#include <sstream>
#include <set>
#include "Tid.h"
#include "Utils.h"
#include "common/Config.h"
#include "common/Console.h"
#include "common/Event.h"
#include "common/auxiliary.h"

namespace Logger {

struct MemAllocEvent {
    void *addr_;
    size_t sz_;
    uint64_t *callstack_;
    size_t callstack_sz_;

    MemAllocEvent(void *addr, size_t sz, const std::vector<uint64_t> *callstack)
        : addr_(addr)
        , sz_(sz) {
        callstack_sz_ = callstack->size();
        callstack_ = new uint64_t[callstack_sz_];
        for (size_t i = 0; i < callstack_sz_; ++i) {
            callstack_[i] = (*callstack)[i];
        }
    }

    std::string stringfy() const {
        std::stringstream ss;
        ss << addr_ << ' ' << sz_ << ":";

        for (size_t i = 0; i < callstack_sz_; ++i) {
            ss << callstack_[i] << ' ';
        }
        return ss.str();
    }
};

const uint64_t MAX_NR_EVENTS = _1G_ / sizeof(Event) * 4;
const timestamp_t MAX_NUMBER = std::numeric_limits<timestamp_t>::max();

static __thread timestamp_t __batch_id = MAX_NUMBER;
static __thread timestamp_t __batch_pos = MAX_NUMBER;
static __thread timestamp_t __batch_sz = MAX_NUMBER;

static Event *__events = nullptr;  //[MAX_NR_EVENTS] __attribute__((aligned(64)));

static std::atomic_ullong __mem_alloc_events_pos(0);
const uint64_t MAX_NR_MEM_EVENTS = _1M_ / sizeof(Event) * 500;
static MemAllocEvent *__mem_alloc_events;

#if (__x86_64__)
static std::atomic_ullong __log_sz(0);
#else
static std::atomic_long __log_sz(0);
#endif

std::atomic_bool __logger_enable(false);

// Which thread to log
std::set<tid_t> __log_thread;

// Sample read/write events
bool __sample = false;

// Log libc string and memory operation
bool __log_libc_mem_op = false;

// Use tmpfs (in memory file system)
bool __use_tmpfs = false;

// Log memory allocation
bool __log_mem_alloc = false;

#define COMMON_ARG inst, ts, tid
#define ADDR(addr) (reinterpret_cast<address_t>(addr))


void init() {
    const Subconfig *loggerConfig = Config::getLoggerConfig();
    auto threads = loggerConfig->getVector("LOG_THREAD");
    for (auto t: threads) {
        __log_thread.insert(t);
    }
    __logger_enable = loggerConfig->getBool("ENABLE");
    __sample = loggerConfig->getBool("SAMPLE");
    __log_libc_mem_op = loggerConfig->getBool("LOG_LIBC_MEM_OP");
    __use_tmpfs = loggerConfig->getBool("TMPFS");
    __log_mem_alloc = loggerConfig->getBool("LOG_MEM_ALLOC");

    if (!__logger_enable)
        return;
    size_t align = 64;
    posix_memalign((void **)(&__events), align, MAX_NR_EVENTS * sizeof(Event));

    posix_memalign((void **)(&__mem_alloc_events),
                   align,
                   MAX_NR_MEM_EVENTS * sizeof(MemAllocEvent));
}

//
// Call this method when process is forked.
//
void reinit() {
    // Reset timestamp and close file descriptor
    __log_sz = 0;
}

void enableLogger() {
    __logger_enable = true;
}

void disableLogger() {
    __logger_enable = false;
}

bool logLibcMemOp() {
    return __logger_enable && __log_libc_mem_op;
}

bool logMemAlloc() {
    return __logger_enable && __log_mem_alloc;
}

static bool needLogging() {
    if (!__logger_enable) {
        return false;
    }
    // static bool flag = __log_thread.size() == 0
                     // ? true
                     // : __log_thread.count(Tid::getCurrTid());
    // return flag;
    return true;
}

void startBatch(uint64_t nr) {
    if (!__logger_enable)
        return
    //
    // Sampling not support for batch logging
    //
    assert(!__sample);

    if (!__logger_enable)
        return;
    assert(__batch_id == MAX_NUMBER);
    assert(__batch_pos == MAX_NUMBER);
    assert(__batch_sz == MAX_NUMBER);

    __batch_id = __log_sz.fetch_add(nr, std::memory_order_seq_cst);
    __batch_pos = 0;
    __batch_sz = nr;
    // fprintf(stderr, "%lu %lu %lu\n", __batch_id, __batch_pos, __batch_sz);
}

void startBatchFor(int nr_addr, ...) {
    if (!__logger_enable)
        return
    //
    // Sampling not support for batch logging
    //
    assert(!__sample);

    assert(__batch_id == MAX_NUMBER);
    assert(__batch_pos == MAX_NUMBER);
    assert(__batch_sz == MAX_NUMBER);

    va_list vlist;
    va_start(vlist, nr_addr);
    int nr_valid_addr = 0;
    for (int i = 0; i < nr_addr; ++i) {
        const void *addr = va_arg(vlist, const void *);
        if (!Utils::isOnStack(addr)) {
            nr_valid_addr += 1;
        }
    }
    va_end(vlist);
    // If valid address is less than two, it's of no need for batch logging.
    if (nr_valid_addr > 1) {
        __batch_id = __log_sz.fetch_add(nr_valid_addr, std::memory_order_seq_cst);
        __batch_pos = 0;
        __batch_sz = nr_valid_addr;
    }
}

void endBatch() {
    if (!__logger_enable)
        return
    //
    // Sampling not support for batch logging
    //
    assert(!__sample);

    assert(__batch_pos == __batch_sz);
    __batch_id = MAX_NUMBER;
    __batch_pos = MAX_NUMBER;
    __batch_sz = MAX_NUMBER;
}

static bool inBatch() {
    return __batch_id != MAX_NUMBER;
}

// static timestamp_t logSize() {
    // assert(__logger_enable);
    // return __log_sz.load(std::memory_order_seq_cst);
// }

static timestamp_t fetchAddLogSize() {
    assert(__logger_enable);
    if (__batch_id != MAX_NUMBER) {
        assert(__batch_pos < __batch_sz);
        timestamp_t ts = __batch_id + __batch_pos;
        __batch_pos += 1;
        assert(ts < MAX_NR_EVENTS);
        return ts;
    }
    timestamp_t ts = __log_sz.fetch_add(1, std::memory_order_seq_cst);
    assert(ts < MAX_NR_EVENTS);
    return ts;
}

void logVarAccessEvent(instID_t inst, tid_t tid, int op_ty,
                               const void *addr) {
    if (!needLogging())
        return;

    if (Utils::isOnStack(addr))
        return;

    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeVarAccessEvent(COMMON_ARG, op_ty, ADDR(addr));
}

void logArrayAccessEvent(instID_t inst, tid_t tid, int op_ty,
                                 const void *addr, size_t len) {
    if (!needLogging())
        return;

    if (Utils::isOnStack(addr))
        return;

    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeArrayAccessEvent(COMMON_ARG, op_ty, ADDR(addr), len);
}

void logSyncOpEvent(instID_t inst, tid_t tid, int op_ty,
                            const void *sync_var_addr) {
    if (!needLogging())
        return;

    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeSyncOpEvent(COMMON_ARG, op_ty, ADDR(sync_var_addr));
}

void logThreadCreateEvent(instID_t inst, tid_t tid, tid_t remote_tid) {
    if (!needLogging())
        return;

    assert(inBatch());
    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeThreadEvent(COMMON_ARG, Event::OP_CREATE, remote_tid);
}

void logThreadJoinEvent(instID_t inst, tid_t tid, tid_t remote_tid) {
    if (!needLogging())
        return;

    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeThreadEvent(COMMON_ARG, Event::OP_JOIN, remote_tid);
}

void logCondWaitEvent(instID_t inst, tid_t tid, const void *mutex_addr,
                              const void *cond_addr) {
    if (!needLogging())
        return;

    timestamp_t ts = __log_sz.fetch_add(2, std::memory_order_seq_cst);
    assert(ts + 1 < MAX_NR_EVENTS);

    __events[ts] =
        Event::makeSyncOpEvent(COMMON_ARG, Event::OP_UNLOCK, ADDR(mutex_addr));
    __events[ts + 1] =
        Event::makeSyncOpEvent(inst, ts + 1, tid, Event::OP_WAIT, ADDR(cond_addr));
}

void logCondAwakeEvent(instID_t inst, tid_t tid, const void *mutex_addr,
                               const void *cond_addr) {
    if (!needLogging())
        return;

    timestamp_t ts = __log_sz.fetch_add(2, std::memory_order_seq_cst);
    assert(ts + 1 < MAX_NR_EVENTS);

    __events[ts] = Event::makeSyncOpEvent(COMMON_ARG, Event::OP_LOCK, ADDR(mutex_addr));
    __events[ts + 1] =
        Event::makeSyncOpEvent(inst, ts + 1, tid, Event::OP_AWAKE, ADDR(cond_addr));
}

void logFuncCallEvent(instID_t inst, tid_t tid) {
    if (!needLogging())
        return;

    timestamp_t ts = fetchAddLogSize();
    __events[ts] = Event::makeFuncCallEvent(COMMON_ARG);
}

void logMemAllocEvent(void *addr, size_t sz,
                      const std::vector<uint64_t>* callstack) {
    if (!needLogging())
        return;
    assert(__log_mem_alloc);
    auto pos = __mem_alloc_events_pos.fetch_add(1, std::memory_order_seq_cst);
    auto *mem_evt = new MemAllocEvent(addr, sz, callstack);
    __mem_alloc_events[pos] = *mem_evt;
}

static void flushMemAllocEvents(const std::string fname) {
    assert(__logger_enable && __log_mem_alloc);

    std::ofstream fout;
    Aux::openOstream(fout, fname);

    size_t nr = 0;
    for (size_t i = 0; i < __mem_alloc_events_pos; ++i) {
        fout << __mem_alloc_events[i].stringfy() << "\n";
        nr += 1;
    }
    fout.flush();
    fout.close();
    if (!__use_tmpfs) {
        system(Aux::format("chmod 0664 %s", fname.c_str()).c_str());
    }

    Console::info("Memory Allocation Flush to %s\n", fname.c_str());
    Console::info("#Total Memory Allocation: %lu\n", nr);
}

static void flush(const std::string fname) {
    assert(__logger_enable && __events);

    std::ofstream fout;
    Aux::openOstream(fout, fname, std::ios::binary, true);

    for (uint64_t i = 0; i < __log_sz; ++i) {
        Event &evt = __events[i];
        if (evt.getTimeStamp() != i) {
            Console::redErr("%lu xx %lu ==> TOTAL %lu\n", evt.getTimeStamp(), i, __log_sz.load());
            evt.dump();
            assert(false);
        }
        Event::storeEvent(fout, evt);
    }

    fout.flush();
    fout.close();
    if (!__use_tmpfs) {
        system(Aux::format("chmod 0664 %s", fname.c_str()).c_str());
    }

    unsigned char hash[SHA_DIGEST_LENGTH];  // == 20
    SHA1((const unsigned char *)__events, sizeof(Event) * __log_sz, hash);

    Console::info("Flush to %s\n", fname.c_str());
    Console::info("Log Hash: ");
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        Console::info("%02X", hash[i]);
    }
    Console::info("\n");
    Console::info("#Total thread: %lu\n", Tid::getNrThread());
    Console::info("#Total timestamp: %llu\n", __log_sz.load());

    if (__log_mem_alloc) {
        std::string mem_alloc_log_fname(fname);
        Aux::replaceAll(mem_alloc_log_fname, ".trace", ".mem-trace");
        flushMemAllocEvents(mem_alloc_log_fname);
    }
}

void save(const std::string &fname) {
    Console::info("=========== SAVE ==============\n");

    if (__logger_enable) {
        if (__use_tmpfs) {
            std::string fname_copy(fname);
            std::replace(fname_copy.begin(), fname_copy.end(), '/', '&');
            std::string fname_new = "/dev/shm/" + fname_copy;
            flush(fname_new);
        } else {
            flush(fname);
        }
    } else {
        Console::info("Not logging\n");
    }
    Console::info("===============================\n");
}

}
