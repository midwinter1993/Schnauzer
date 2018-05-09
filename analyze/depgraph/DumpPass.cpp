#include <iostream>

#include "AnalyzePass.h"
#include "DepGraph.h"
#include "common/Event.h"

void DumpPass::analyze(DepGraph *graph) {
    auto tids = graph->getThreadIds();

    for (auto tid: tids) {
        auto &nd_list = graph->getThreadEvents(tid);

        std::cerr << "=======Thread: " << tid << "=========\n";
        for(auto it = nd_list.begin(); it != nd_list.end(); ++it) {
            const Event &evt = it->getEvent();
            evt.dump();
        }
    }
}


