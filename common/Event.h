#ifndef __COMMON_EVENT_H__
#define __COMMON_EVENT_H__

#include <cassert>
#include <fstream>
#include <iostream>
#include "basic.h"

// #define COMPACT_EVENT

class Event {
public:
    static void storeEvent(std::ofstream &fout, const Event &evt,
                           bool is_binary = true);
    static Event loadEvent(std::ifstream &fin, bool is_binary = true);
    static size_t load(Event *buf, std::ifstream &fin);

    static const char *typeStr(op_t op);

    static Event makeVarAccessEvent(instID_t inst,
                                    timestamp_t ts,
                                    tid_t tid,
                                    int ty,
                                    address_t addr);

    static Event makeArrayAccessEvent(instID_t inst,
                                      timestamp_t ts,
                                      tid_t tid,
                                      int ty,
                                      address_t addr,
                                      size_t len);

    static Event makeSyncOpEvent(instID_t inst,
                                 timestamp_t ts,
                                 tid_t tid,
                                 int ty,
                                 address_t addr);

    static Event makeThreadEvent(instID_t inst,
                                 timestamp_t ts,
                                 tid_t tid,
                                 int ty,
                                 tid_t remote_tid);

    static Event makeFuncCallEvent(instID_t inst,
                                  timestamp_t ts,
                                  tid_t tid);


    enum {
        OP_CREATE    = 12,
        OP_JOIN      = 13,
        OP_LD        = 14,
        OP_ST        = 15,
        OP_TRYLOCK   = 16,
        OP_LOCK      = 17,
        OP_UNLOCK    = 18,
        OP_WAIT      = 19,
        OP_AWAKE     = 20,
        OP_SIGNAL    = 21,
        OP_BROADCAST = 22,
        OP_CALL      = 23
    };

private:
    Event() {}

#ifdef COMPACT_EVENT
    Event(instID_t inst,  timestamp_t ts,
          tid_t tid,      int ty,
          address_t addr, size_t len,
          tid_t remote_tid)
        : inst_unique_id_(inst) , op_ty_(ty) , tid_(tid) {
        if (op_ty_ == OP_CREATE || op_ty_ == OP_JOIN) {
            data_.remote_tid_ = remote_tid;
        } else {
            data_.var_info_.var_addr_ = addr;
            data_.var_info_.var_len_=len;
        }
    }
#else
    Event(instID_t inst,  timestamp_t ts,
          tid_t tid,      int ty,
          address_t addr, size_t len,
          tid_t remote_tid)
        : inst_unique_id_(inst)
        , timestamp_(ts)
        , var_addr_(addr)
        , var_len_(len)
        , op_ty_(ty)
        , tid_(tid)
        , remote_tid_(remote_tid) {}

#endif

public:
    instID_t getInstUniqueID() const {
        return inst_unique_id_;
    }

    tid_t getThread() const {
        assert(0 <= tid_ && tid_ < UNDEFINED_TID);
        return tid_;
    }

    int getType() const {
        return op_ty_;
    }

#ifdef COMPACT_EVENT
    address_t getAddress() const {
        assert(op_ty_ != OP_CREATE && op_ty_ != OP_JOIN);
        return data_.var_info_.var_addr_;
    }

    size_t getLength() const {
        assert(isAccessEvent() && data_.var_info_.var_len_ > 0);
        return data_.var_info_.var_len_;
    }

    tid_t getRemoteThread() const {
        assert(op_ty_ == OP_CREATE || op_ty_ == OP_JOIN);
        return data_.remote_tid_;
    }
#else
    timestamp_t getTimeStamp() const { return timestamp_; }

    address_t getAddress() const {
        assert(op_ty_ != OP_CREATE && op_ty_ != OP_JOIN);
        return var_addr_;
    }

    size_t getLength() const {
        assert(isAccessEvent() && var_len_ > 0);
        return var_len_;
    }

    tid_t getRemoteThread() const {
        assert(op_ty_ == OP_CREATE || op_ty_ == OP_JOIN);
        return remote_tid_;
    }
#endif

    std::string stringfy() const;
    void dump() const { std::cerr << stringfy() << "\n"; }

    bool isAccessEvent() const;
    bool isVarAccessEvent() const;
    bool isArrayAccessEvent() const;
    bool isSyncOpEvent() const;
    bool isSyncAcqEvent() const;
    bool isSyncRelEvent() const;
    bool isThreadOpEvent() const;
    bool isFuncCallEvent() const;

    bool isConflict(const Event &evt) const;
    bool isConflict(const Event *evt) const;

    // For race detection
    Event expandArrayAccessEvent(size_t pos) const;
private:
#ifdef COMPACT_EVENT
    instID_t inst_unique_id_;
    union {
        struct {
            address_t var_addr_;
            size_t var_len_;
        } var_info_;
        tid_t remote_tid_;
    } data_;
    int8_t op_ty_;
    tid_t tid_;
#else
    instID_t inst_unique_id_;
    timestamp_t timestamp_;
    address_t var_addr_;
    size_t var_len_;
    int8_t op_ty_;
    tid_t tid_;
    tid_t remote_tid_;
#endif
} /* __attribute__((packed, aligned(4))) */;

#endif /* __COMMON_EVENT_H__ */
