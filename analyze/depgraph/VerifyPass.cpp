#include <cassert>
#include <iostream>

#include "AnalyzePass.h"
#include "DepGraph.h"
#include "common/Event.h"

static void verifyDependence(const Event &before_evt, const Event &after_evt) {
    // Check whether address is same.
    if (before_evt.getAddress() != after_evt.getAddress()) {
        std::cerr << "Invalid Dependence with different address\n";
        assert(0);
    }

    // Check before_evt happens before after_evt
#ifndef COMPACT_EVENT
    if (before_evt.getTimeStamp() >= after_evt.getTimeStamp()) {
        std::cerr << "Invalid Dependence with different address\n";
        assert(0);
    }
#endif

    // Check whether thread is different.
    if (before_evt.getThread() == after_evt.getThread()) {
        std::cerr << "Dependence for program order\n";
        assert(0);
    }

    // Check redundant dependence, such as, rar.
    if (before_evt.getType() == Event::OP_ST) {
        if (after_evt.getType() != Event::OP_LD &&
            after_evt.getType() != Event::OP_ST) {
            std::cerr << "Unknow Dependence with Store\n";
            assert(0);
        }
    } else if (before_evt.getType() == Event::OP_LD) {
        if (after_evt.getType() != Event::OP_ST) {
            std::cerr << "Unknow Dependence with Load\n";
            assert(0);
        }
    } else if (before_evt.getType() == Event::OP_LOCK) {
        if (after_evt.getType() != Event::OP_UNLOCK) {
            std::cerr << "Unknow Dependence with LOCK\n";
#ifndef COMPACT_EVENT
            std::cerr << before_evt.getAddress() << ' '
                      << before_evt.getTimeStamp() << "\n";
            std::cerr << after_evt.getAddress() << ' '
                      << after_evt.getTimeStamp() << "\n";
#endif
            // assert(0);
        }
    } else if (before_evt.getType() == Event::OP_UNLOCK) {
        if (after_evt.getType() != Event::OP_LOCK) {
            std::cerr << "Unknow Dependence:" << after_evt.getType()
                      << " with UNLOCK\n";
#ifndef COMPACT_EVENT
            std::cerr << "Timestamp: " << after_evt.getTimeStamp() << "\n";
#endif
            // assert(0);
        }
    }
}

void VerifyPass::analyze(DepGraph *graph) {
    auto tids = graph->getThreadIds();

    for (auto tid : tids) {
        auto &nd_list = graph->getThreadEvents(tid);

        for (auto it = nd_list.begin(); it != nd_list.end(); ++it) {
            auto &nd = *it;
            const Event &evt = nd.getEvent();

            if (evt.isThreadOpEvent()) {
                assert(nd.getHbMe().empty());
                assert(nd.getHaMe().empty());
                continue;
            }

            auto hb_vec = nd.getHbMe();
            if (!hb_vec.empty()) {
                for (auto nd_ptr : hb_vec) {
                    const Event &other_evt = nd_ptr->getEvent();
                    verifyDependence(other_evt, evt);
                }
            }

            auto ha_vec = nd.getHaMe();
            if (!ha_vec.empty()) {
                for (auto nd_ptr : ha_vec) {
                    const Event &other_evt = nd_ptr->getEvent();
                    verifyDependence(evt, other_evt);
                }
            }
        }
    }
    // std::cerr << "Verification Pass\n";
}
