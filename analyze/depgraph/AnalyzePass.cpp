//==============================
// Skeleton of analysis pass.
//==============================
/*
void XXXPass::analyze(DepGraph *graph) {
    auto tids = graph->getThreadIds();

    for (auto tid: tids) {
        auto &nd_list = graph->getThreadNodeList(tid);
        auto it = nd_list.begin();

        while (!it.isNull()) {
            auto &nd = *it;
            const Event &evt = nd.getEvent();

            ++it;
        }
    }
}
*/
//==============================

#include "AnalyzePass.h"

#include <future>

void AnalyzePass::visit(DepGraph *graph, bool isAsync) {
    if (isAsync)
        std::async(std::launch::async, &AnalyzePass::analyze, this, graph);
    else
        analyze(graph);
}
