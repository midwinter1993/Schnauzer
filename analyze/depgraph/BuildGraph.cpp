#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include "AnalyzePass.h"
#include "DepGraph.h"
#include "VarMetaData.h"
#include "VectorClock.h"
#include "common/Event.h"
#include "common/auxiliary.h"

void DepGraph::addRelation(DGNode *last_nd, DGNode *curr_nd ){
    address_t addr = last_nd->getEvent().getAddress();
    assert(addr == curr_nd->getEvent().getAddress());
    UNUSED(addr);

    curr_nd->addHbMe(last_nd);
    last_nd->addHaMe(curr_nd);
}

void DepGraph::buildGraphImpl() {
    VarMeta *var_meta = new VarMeta();
    // std::map<const Event*, DGNode*> event_node_table;
    for (auto it = this->begin(), end = this->end(); it != end; ++it) {
        auto *evt = &*it;

        // Filter store/load on mutex variables.
        if (evt->isVarAccessEvent() && sync_vars_.count(evt->getAddress())) {
            continue;
        }

        // Create graph node
        // DGNode *curr_nd = new DGNode(evt);
        addThreadEvent(evt->getThread(), evt);
        // event_node_table.emplace(evt, curr_nd);

        // if (evt->isThreadOpEvent() || evt->isSyncOpEvent()) {
            // var_meta->updateHB(evt);
        // } else if (evt->isVarAccessEvent()) {
            // std::set<const Event*>hb_evts = var_meta->getHbEvents(evt);
            // for (auto hb_evt: hb_evts) {
                // addRelation(event_node_table[hb_evt], curr_nd);
            // }

            // int op_ty = evt->getType();
            // if (op_ty == Event::OP_LD) {
                // var_meta->putRead(evt);
            // } else if (op_ty == Event::OP_ST) {
                // var_meta->putWrite(evt);
            // }
        // } else {
            // std::cerr << evt->getType() << std::endl;
            // assert(0);
        // }
    }
    delete var_meta;
}

DepGraph *DepGraph::buildGraph(const std::string &trace_name) {
    DepGraph *graph = new DepGraph();

    graph->loadTrace(trace_name);
    graph->buildGraphImpl();

    std::cerr << "# THREAD: " << graph->getNrThread() << "\n";

    VerifyPass verify;
    graph->acceptPass(verify);

    std::cerr << "---\n";
    return graph;
}
