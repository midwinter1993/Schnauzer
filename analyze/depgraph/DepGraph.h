#ifndef __ANALYZE_DEPGRAPH_DEPGRAPH_H__
#define __ANALYZE_DEPGRAPH_DEPGRAPH_H__

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "VectorClock.h"
#include "common/Event.h"
#include "common/auxiliary.h"

/**
 * Node in dependence graph.
 */
struct DGNode {
public:
    DGNode(const Event *evt) : evt_(evt) {}

    /**
     * Add happens-before edge.
     * nd happens-before current node.
     */
    void addHbMe(DGNode *nd) {
        vec_hb_me_.push_back(nd);
    }

    void delHbMe(DGNode *nd) {
        auto it = std::find(vec_hb_me_.begin(), vec_hb_me_.end(), nd);
        UNUSED(it);
        assert(it != vec_hb_me_.end());
        vec_hb_me_.erase(std::remove(vec_hb_me_.begin(), vec_hb_me_.end(), nd),
                         vec_hb_me_.end());
    }

    /**
     * Add happens-after edge.
     * nd happens-after current node.
     */
    void addHaMe(DGNode *nd) { vec_ha_me_.push_back(nd); }

    void delHaMe(DGNode *nd) {
        auto it = std::find(vec_ha_me_.begin(), vec_ha_me_.end(), nd);
        UNUSED(it);
        assert(it != vec_ha_me_.end());
        vec_ha_me_.erase(std::remove(vec_ha_me_.begin(), vec_ha_me_.end(), nd),
                         vec_ha_me_.end());
    }

    const std::vector<DGNode *> &getHbMe() const {
        return vec_hb_me_;
    }

    const std::vector<DGNode *> &getHaMe() const {
        return vec_ha_me_;
    }

    const Event &getEvent() {
        return *evt_;
    }

    const Event *evt_;
    std::vector<DGNode *> vec_hb_me_;
    std::vector<DGNode *> vec_ha_me_;
};

class AnalyzePass;

/**
 * Dependence graph.
 */
class DepGraph {
public:
    static DepGraph *buildGraph(const std::string &trace_name);

    typedef std::list<DGNode> ThreadEvents;

public:
    std::vector<int> getThreadIds() const;

    ThreadEvents& getThreadEvents(tid_t tid);

    void acceptPass(AnalyzePass &pass, bool isAsync = false);

    std::list<const Event*> filterVarWithSync(address_t addr) const;

    std::string getTraceName() const {
        return trace_name_;
    }

    std::string getRaceName() const {
        std::string race_name(trace_name_);
        Aux::replaceLast(race_name, "trace", "race");
        return race_name;
    }

    std::list<Event>::iterator begin() {
        return tot_events_.begin();
    }

    std::list<Event>::iterator end() {
        return tot_events_.end();
    }

    size_t getNrThread() const {
        return graph_.size();
    }

    const std::set<address_t>& getSharedVars() const {
        return shared_vars_;
    }

    const std::set<address_t>& getSyncVars() const {
        return sync_vars_;
    }

    bool isSharedVar(address_t addr) const {
        return shared_vars_.count(addr);
    }

    bool isSyncVar(address_t addr) const {
        return sync_vars_.count(addr);
    }

    // const std::set<address_t> &getRacedVars() const { return raced_vars_; }

private:
    void addTotalEvent(const Event &evt) {
        tot_events_.push_back(evt);
    }

    void addThreadEvent(tid_t tid, const Event *evt) {
        graph_[tid].emplace_back(evt);
    }

    void addRelation(DGNode *last_nd, const VectorClock &last_vc,
                     DGNode *curr_nd, const VectorClock &curr_vc);
    void addRelation(DGNode *last_nd, DGNode *curr_nd );

    void loadTrace(const std::string &trace_name);

    void buildGraphImpl();

    std::string trace_name_;
    std::list<Event> tot_events_;
    std::map<tid_t, ThreadEvents> graph_;
    std::set<address_t> shared_vars_;
    std::set<address_t> sync_vars_;
    size_t tot_nr_events_for_pct_;
};

#endif
