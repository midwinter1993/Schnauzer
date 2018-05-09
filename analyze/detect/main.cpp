#include <iostream>
#include "DetectPass.h"
#include "analyze/depgraph/DepGraph.h"
#include "common/auxiliary.h"
#include "common/basic.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: analyze trace";
        exit(0);
    }
    std::string trace_fname(argv[1]);
    assert(Aux::endswith(trace_fname, ".trace"));

    if (Aux::fileSize(trace_fname) > 3* _1G_) {
        std::cerr << "Trace file too LARGE\n";
        return 0;
    }

    DepGraph *graph = DepGraph::buildGraph(trace_fname);

    assert(graph->getNrThread() < VectorClock::MAX_THREAD);

    DetectPass dp;
    graph->acceptPass(dp);

    std::cerr << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    // DumpPass dump;
    // graph->acceptPass(dump);

    return 0;
}
