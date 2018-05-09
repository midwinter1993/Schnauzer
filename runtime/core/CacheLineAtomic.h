#ifndef __RUNTIME_CORE_CACHE_LINE_ATOMIC_H__
#define __RUNTIME_CORE_CACHE_LINE_ATOMIC_H__

#include <string.h>
#include <atomic>

//
// Atomic counter as clock for each thread;
// Avoid false sharing
//
#define CACHE_LINE_SZ 64

struct CacheLineAtomic {
    typedef uint64_t value_t;

    std::atomic<value_t> val_;
    uint8_t padding[CACHE_LINE_SZ - sizeof(val_)];

    CacheLineAtomic() {
        val_ = 0;
        memset(padding, 0, CACHE_LINE_SZ - sizeof(val_));
    }

    value_t inc() { return val_.fetch_add(1, std::memory_order_seq_cst); }

    value_t dec() { return val_.fetch_add(-1, std::memory_order_seq_cst); }

    value_t clearBit(size_t idx) {
        return val_.fetch_and(~(1U << idx), std::memory_order_seq_cst);
    }

    value_t setBit(size_t idx) {
        return val_.fetch_or(1U << idx, std::memory_order_seq_cst);
    }

    value_t set(value_t val) {
        return val_.exchange(val, std::memory_order_seq_cst);
    }

    value_t get() { return val_.load(std::memory_order_seq_cst); }

    // Act as a bool atomic
    value_t setFalse() { return val_.exchange(0, std::memory_order_seq_cst); }

    value_t setTrue() { return val_.exchange(1, std::memory_order_seq_cst); }
};

#endif /* ifndef __RUNTIME_UTIL_CACHE_LINE_ATOMIC_H__ */
