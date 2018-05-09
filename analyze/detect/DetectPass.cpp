#include "DetectPass.h"
#include <cassert>
#include <string>
#include "analyze/depgraph/DepGraph.h"
#include "analyze/depgraph/RaceAnalyze.h"
#include "common/Event.h"
#include "InstRace.h"
#include "common/auxiliary.h"
#include "common/Console.h"
#include "common/Config.h"
#include "ThreadSummary.h"

// #define HARMFUL_FILTER

void DetectPass::analyze(DepGraph *graph) {
    RaceAnalyze ra;
    graph->acceptPass(ra);

    std::set<address_t> race_vars;

    InstRaceTable race_table;
    for (auto &r: ra.getTotalRaceEventPair()) {
        assert(r.first->getAddress() == r.second->getAddress());
        race_vars.insert(r.first->getAddress());
        race_vars.insert(r.second->getAddress());

        race_table.addRace(r.first, r.second);

        auto &first_ts = ThreadSummary::getThreadSummary(r.first->getThread());
        auto &second_ts = ThreadSummary::getThreadSummary(r.second->getThread());


        if (r.first->getTimeStamp() < r.second->getTimeStamp()) {
            int64_t d = (int64_t)r.second->getTimeStamp() - (int64_t)r.first->getTimeStamp();
            first_ts.incItem("before_event_race_distance", d);
            second_ts.incItem("after_event_race_distance", d);

            first_ts.incItem("nr_before_event_race", 1);
            second_ts.incItem("nr_after_event_race", 1);
        } else {
            int64_t d = (int64_t)r.first->getTimeStamp() - (int64_t)r.second->getTimeStamp();
            first_ts.incItem("after_event_race_distance", d);
            second_ts.incItem("before_event_race_distance", d);

            first_ts.incItem("nr_after_event_race", 1);
            second_ts.incItem("nr_before_event_race", 1);
        }
    }

    for (auto &sync: ra.getTotalSyncHbEventPair()) {
        assert(sync.first->getAddress() == sync.second->getAddress());

        auto &first_ts = ThreadSummary::getThreadSummary(sync.first->getThread());
        auto &second_ts = ThreadSummary::getThreadSummary(sync.second->getThread());


        if (sync.first->getTimeStamp() < sync.second->getTimeStamp()) {
            int64_t d = (int64_t)sync.second->getTimeStamp() - (int64_t)sync.first->getTimeStamp();

            first_ts.incItem("before_sync_hb_distance", d);
            second_ts.incItem("after_sync_hb_distance", d);

            first_ts.incItem("nr_before_sync_hb", 1);
            second_ts.incItem("nr_after_sync_hb", 1);
        } else {
            int64_t d = (int64_t)sync.first->getTimeStamp() - (int64_t)sync.second->getTimeStamp();
            first_ts.incItem("after_sync_hb_distance", d);
            second_ts.incItem("before_sync_hb_distance", d);

            first_ts.incItem("nr_after_sync_hb", 1);
            second_ts.incItem("nr_before_sync_hb", 1);
        }
    }

    for (auto it = graph->begin(), end = graph->end(); it != end; ++it) {
        auto *evt = &*it;
        ThreadSummary::getThreadSummary(evt->getThread()).incItem("nr_event", 1);
    }


    std::ofstream fout;
    Aux::openOstream(fout, graph->getRaceName());

    race_table.dumpFile(fout);

    printf("# Raced Variables %lu\n", race_vars.size());
    printf("# Raced Event Pairs %lu\n", ra.getTotalRaceEventPair().size());
    printf("# Raced Static Instruction Pairs %lu\n", race_table.size());

    ThreadSummary::asYaml()->save(Config::getConfig(false).summaryPath());
    // ThreadSummary::asYaml()->dump();
}
