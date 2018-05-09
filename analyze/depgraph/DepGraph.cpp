#include "DepGraph.h"
#include <stdio.h>
#include <cassert>
#include <unordered_map>
#include "AnalyzePass.h"

std::vector<int> DepGraph::getThreadIds() const {
    std::vector<int> tids;
    for (auto &item : graph_) {
        tids.push_back(item.first);
    }
    return tids;
}

DepGraph::ThreadEvents &DepGraph::getThreadEvents(tid_t tid) {
    auto it = graph_.find(tid);
    assert(it != graph_.end());
    return it->second;
}

void DepGraph::acceptPass(AnalyzePass &pass, bool isAsync) {
    pass.visit(this, isAsync);
}

std::list<const Event*> DepGraph::filterVarWithSync(address_t addr) const {
    std::list<const Event*> ret;
    for (auto &evt: tot_events_) {
        int op_ty = evt.getType();
        if (op_ty == Event::OP_LD || op_ty == Event::OP_ST) {
            if (evt.getAddress() == addr)
                ret.push_back(&evt);
        } else {
            ret.push_back(&evt);
        }
    }
    return ret;
}
