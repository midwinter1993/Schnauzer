#include "RaceAnalyze.h"
#include <cassert>
#include <fstream>
#include <string>
#include "DepGraph.h"
#include "VarMetaData.h"
#include "common/Event.h"
#include "common/auxiliary.h"
#include "common/basic.h"

RaceAnalyze::~RaceAnalyze() {
    if (var_meta_) {
        delete var_meta_;
    }
}

void RaceAnalyze::addRace(const Event *evt_first, const Event *evt_second) {
    assert(evt_first->getAddress() == evt_second->getAddress());
    race_evt_pairs_.emplace(evt_first, evt_second);
}

void RaceAnalyze::addSyncHb(const Event *evt_first, const Event *evt_second) {
    assert(evt_first->getAddress() == evt_second->getAddress());
    sync_hb_evt_pairs_.emplace(evt_first, evt_second);
}

void RaceAnalyze::handleVarAccessEvent(const Event *evt) {
    auto race_evts = var_meta_->getRaceEvents(evt);
    for (auto &r: race_evts) {
        addRace(r, evt);
    }

    int op_ty = evt->getType();
    if (op_ty == Event::OP_LD) {
        var_meta_->putRead(evt);
    } else if (op_ty == Event::OP_ST) {
        var_meta_->putWrite(evt);
    }
}

void RaceAnalyze::handleSyncOpEvent(const Event *evt) {
    if (evt->isSyncAcqEvent()) {
        auto sync_hb_evts = var_meta_->getSyncHbEvents(evt);
        for (auto &e: sync_hb_evts) {
            addSyncHb(e, evt);
        }

        var_meta_->clearSyncRel(evt);
    } else if (evt->isSyncRelEvent()) {
        var_meta_->putSyncRel(evt);
    }

    // Updating must be later.
    var_meta_->updateHB(evt);
}

void RaceAnalyze::analyze(DepGraph *graph) {
    var_meta_ = new VarMeta();
    for (auto it = graph->begin(), end = graph->end(); it != end; ++it) {
        auto *evt = &*it;

        if (evt->isThreadOpEvent() || evt->isSyncOpEvent()) {
            handleSyncOpEvent(evt);
        } else if (evt->isVarAccessEvent() && !graph->isSyncVar(evt->getAddress())) {
            handleVarAccessEvent(evt);
        } else {
            std::cerr << evt->getType() << std::endl;
            assert(0);
        }
    }
}
