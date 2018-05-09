#include "VarMetaData.h"
#include <cassert>
#include <map>

uint64_t Operation::cnt_ = 0;

void VarMeta::updateHB(const Event* evt) {
    if (evt->isThreadOpEvent()) {
        handleThreadOpEvent(evt);
    } else if (evt->isSyncOpEvent()) {
        handleSyncOpEvent(evt);
    } else {
        std::cerr << evt->getType() << std::endl;
        assert(0);
    }
}

void VarMeta::putRead(const Event *evt) {
    VectorClock *curr_thread_vc = getThreadVC(evt->getThread());
    var_rset_table_[evt->getAddress()].emplace(evt, curr_thread_vc);
}

void VarMeta::putWrite(const Event *evt) {
    VectorClock *curr_thread_vc = getThreadVC(evt->getThread());
    ///////////////////////////////
    // Note: Not ***SOUND*** here
    ///////////////////////////////
    auto addr = evt->getAddress();
    var_rset_table_[addr].clear();
    var_wset_table_[addr].clear();

    var_wset_table_[addr].emplace(evt, curr_thread_vc);
}

void VarMeta::clearSyncRel(const Event *evt) {
    sync_rel_table_[evt->getAddress()].clear();
}

void VarMeta::putSyncRel(const Event *evt) {
    assert(evt->isSyncRelEvent());
    VectorClock *curr_thread_vc = getThreadVC(evt->getThread());
    sync_rel_table_[evt->getAddress()].emplace(evt, curr_thread_vc);
}

std::set<const Event*> VarMeta::getRaceEvents(const Event *evt){
    VectorClock *curr_thread_vc = getThreadVC(evt->getThread());
    std::set<const Event*> ret;
    if (evt->getType() == Event::OP_LD) {
        auto wset_ptr = getWriteSet(evt->getAddress());
        if (wset_ptr) {
            for (auto &op: *wset_ptr) {
                assert(op.getEvent()->getType() == Event::OP_ST);
                if (op.getVC().isConcurrentWith(curr_thread_vc)) {
                    ret.insert(op.getEvent());
                }
            }
        }
    } else if (evt->getType() == Event::OP_ST) {
        auto rset_ptr = getReadSet(evt->getAddress());
        if (rset_ptr) {
            for (auto &op: *rset_ptr) {
                assert(op.getEvent()->getType() == Event::OP_LD);
                if (op.getVC().isConcurrentWith(curr_thread_vc)) {
                    ret.insert(op.getEvent());
                }
            }
        }
        auto wset_ptr = getWriteSet(evt->getAddress());
        if (wset_ptr) {
            for (auto &op: *wset_ptr) {
                assert(op.getEvent()->getType() == Event::OP_ST);
                if (op.getVC().isConcurrentWith(curr_thread_vc)) {
                    ret.insert(op.getEvent());
                }
            }
        }
    } else {
        assert("Error Event for Detection" && false);
    }
    return ret;
}

std::set<const Event*> VarMeta::getSyncHbEvents(const Event *evt){
    assert(evt->isSyncAcqEvent());
    std::set<const Event*> ret;

    auto sync_rel_set_ptr = getSyncRelSet(evt->getAddress());
    if (sync_rel_set_ptr) {
        for (auto &op: *sync_rel_set_ptr) {
            assert(op.getEvent()->isSyncRelEvent());
            ret.insert(op.getEvent());
        }
    }
    return ret;
}


// std::set<const Event*> VarMeta::getHbEvents(const Event *evt){
    // VectorClock *curr_thread_vc = getThreadVC(evt->getThread());
    // std::set<const Event*> ret;
    // if (evt->getType() == Event::OP_LD) {
        // auto wset_ptr = getWriteSet(evt->getAddress());
        // if (wset_ptr) {
            // for (auto &op: *wset_ptr) {
                // assert(op.getEvent()->getType() == Event::OP_ST);
                // if (!op.getVC().isConcurrentWith(curr_thread_vc)) {
                    // ret.insert(op.getEvent());
                // }
            // }
        // }
    // } else if (evt->getType() == Event::OP_ST) {
        // auto rset_ptr = getReadSet(evt->getAddress());
        // if (rset_ptr) {
            // for (auto &op: *rset_ptr) {
                // assert(op.getEvent()->getType() == Event::OP_LD);
                // if (!op.getVC().isConcurrentWith(curr_thread_vc)) {
                    // ret.insert(op.getEvent());
                // }
            // }
        // }
    // } else {
        // assert("Error Event for Detection" && false);
    // }
    // return ret;
// }

///////////////////////////////////////////////////////////////////////////////

void VarMeta::handleThreadOpEvent(const Event *evt) {
    int op_ty = evt->getType();
    tid_t curr_tid = evt->getThread();
    VectorClock *curr_thread_vc = getThreadVC(curr_tid);
    if (op_ty == Event::OP_CREATE) {
        tid_t remote_tid = evt->getRemoteThread();
        VectorClock *remote_thread_vc = getThreadVC(remote_tid);
        remote_thread_vc->merge(curr_thread_vc);
        curr_thread_vc->incTick(curr_tid);
    } else if (op_ty == Event::OP_JOIN) {
        tid_t remote_tid = evt->getRemoteThread();
        VectorClock *remote_thread_vc = getThreadVC(remote_tid);
        curr_thread_vc->merge(remote_thread_vc);
        remote_thread_vc->incTick(remote_tid);
    }
}

void VarMeta::handleSyncOpEvent(const Event *evt) {
    int op_ty = evt->getType();
    tid_t curr_tid = evt->getThread();
    VectorClock *curr_thread_vc = getThreadVC(curr_tid);
    address_t var_addr = evt->getAddress();

    if (op_ty == Event::OP_LOCK || op_ty == Event::OP_TRYLOCK) {
        auto *lk_vc = getSyncVarVC(var_addr);
        curr_thread_vc->merge(lk_vc);
    } else if (op_ty == Event::OP_UNLOCK) {
        auto *lk_vc = getSyncVarVC(var_addr);
        lk_vc->merge(curr_thread_vc);
        curr_thread_vc->incTick(curr_tid);
    } else if (op_ty == Event::OP_WAIT) {
        // pass
    } else if (op_ty == Event::OP_SIGNAL) {
        // Happens-before the waiter
        auto *lk_vc = getSyncVarVC(var_addr);
        lk_vc->merge(curr_thread_vc);
        curr_thread_vc->incTick(curr_tid);
    } else if (op_ty == Event::OP_AWAKE) {
        // Happens-after the signaler
        auto *lk_vc = getSyncVarVC(var_addr);
        curr_thread_vc->merge(lk_vc);
    } else if (op_ty == Event::OP_BROADCAST) {
        // Happens-before the waiters
        auto *lk_vc = getSyncVarVC(var_addr);
        lk_vc->merge(curr_thread_vc);
        curr_thread_vc->incTick(curr_tid);
    }
}

VectorClock* VarMeta::getThreadVC(tid_t tid) {
    if (!is_thread_vc_init_[tid]) {
        is_thread_vc_init_[tid] = true;
        thread_vc_[tid].incTick(tid);
    }
    return &thread_vc_[tid];
}

VectorClock* VarMeta::getSyncVarVC(address_t addr) {
    return &sync_var_vc_table_[addr];
}

std::set<Operation>* VarMeta::getReadSet(address_t addr) {
    auto it = var_rset_table_.find(addr);
    if (it == var_rset_table_.end())
        return nullptr;
    return &var_rset_table_[addr];
}

std::set<Operation>* VarMeta::getWriteSet(address_t addr) {
    auto it = var_wset_table_.find(addr);
    if (it == var_wset_table_.end())
        return nullptr;
    return &var_wset_table_[addr];
}

std::set<Operation>* VarMeta::getSyncRelSet(address_t addr) {
    auto it = sync_rel_table_.find(addr);
    if (it == sync_rel_table_.end())
        return nullptr;
    return &sync_rel_table_[addr];
}
