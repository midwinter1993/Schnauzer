#ifndef __ANANYZE_DEPGRAPH_VAR_META_DATA_H__
#define __ANANYZE_DEPGRAPH_VAR_META_DATA_H__

#include <set>
#include <map>
#include "VectorClock.h"
#include "common/Event.h"
#include "common/auxiliary.h"

class Operation {
public:
    Operation(const Operation &op) : id_(cnt_++), evt_(op.getEvent()), vc_(op.getVC()) {}
    Operation(const Event *evt, const VectorClock *vc) : id_(cnt_++), evt_(evt), vc_(*vc) {}

    const VectorClock &getVC() const { return vc_; }
    const Event *getEvent() const { return evt_; }

    // bool operator==(const Operation &op) const { return evt_ == op.getEvent(); }
    bool operator<(const Operation &op) const { return id_ < op.id_; }

private:
    static uint64_t cnt_;
    uint64_t id_;
    const Event *evt_;
    VectorClock vc_;
};

class VarMeta {
public:
    void updateHB(const Event *evt);

    void putRead(const Event *evt);
    void putWrite(const Event *evt);

    void clearSyncRel(const Event *evt);
    void putSyncRel(const Event *evt);

    std::set<const Event*> getRaceEvents(const Event *evt);
    std::set<const Event*> getSyncHbEvents(const Event *evt);
    // std::set<const Event*> getHbEvents(const Event *evt);

private:
    std::set<Operation> *getReadSet(address_t addr);
    std::set<Operation> *getWriteSet(address_t addr);

    std::set<Operation> *getSyncRelSet(address_t addr);

    VectorClock *getThreadVC(tid_t tid);
    VectorClock *getSyncVarVC(address_t addr);
    void handleThreadOpEvent(const Event *evt);
    void handleSyncOpEvent(const Event *evt);

private:
    VectorClock thread_vc_[VectorClock::MAX_THREAD];
    bool is_thread_vc_init_[VectorClock::MAX_THREAD] = {false};
    std::map<address_t, VectorClock> sync_var_vc_table_;
    std::map<address_t, std::set<Operation>> var_rset_table_;
    std::map<address_t, std::set<Operation>> var_wset_table_;

    std::map<address_t, std::set<Operation>> sync_rel_table_;
};

#endif /* ifndef __ANANYZE_DEPGRAPH_VAR_META_DATA_H__ */
