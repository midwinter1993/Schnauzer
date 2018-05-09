#ifndef __ANALYZE_DEPGRAPH_RACE_ANALYZE_H__
#define __ANALYZE_DEPGRAPH_RACE_ANALYZE_H__

#include "AnalyzePass.h"
#include "common/Event.h"
#include "VarMetaData.h"
#include <set>

class RaceAnalyze: public AnalyzePass {
public:
    typedef std::pair<const Event*, const Event*> RaceEventPair;
    typedef std::set<RaceEventPair> RaceEventPairSet;

    typedef std::pair<const Event*, const Event*> SyncHbEventPair;
    typedef std::set<RaceEventPair> SyncHbEventPairSet;

    virtual ~RaceAnalyze();
public:
    void analyze(DepGraph *graph) override;

    // RaceInstPairSet& getRaceInstPairSet() { return race_insts_; }
    RaceEventPairSet& getTotalRaceEventPair() {
        return race_evt_pairs_;
    }

    SyncHbEventPairSet& getTotalSyncHbEventPair() {
        return sync_hb_evt_pairs_;
    }

private:

    void addRace(const Event *evt_first, const Event *evt_second);
    void addSyncHb(const Event *evt_first, const Event *evt_second);
    // void handleThreadOpEvent(const Event *evt);
    void handleVarAccessEvent(const Event *evt);
    void handleSyncOpEvent(const Event *evt);

    RaceEventPairSet race_evt_pairs_;
    SyncHbEventPairSet sync_hb_evt_pairs_;

    VarMeta *var_meta_;
};

#endif /* ifndef __ANALYZE_DEPGRAPH_RACE_ANALYZE_H__ */
