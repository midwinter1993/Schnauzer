#include "HeatAnalyze.h"
#include <list>
#include <map>
#include <sstream>
#include "DepGraph.h"
#include "common/Event.h"
#include "common/basic.h"

class CodeBlock {
public:
    static std::string stringfy(const CodeBlock &cb) {
        std::stringstream ss;
        ss << "Start: " << cb.getStartInst() << " # Inst: " << cb.getNrInst()
           << " Heat: " << cb.getTotalHeat() << " Avg heat: " << cb.getAverageHeat();
        return ss.str();
    }

public:
    CodeBlock(instID_t s) : start_inst_(s), total_heat_(1), nr_inst_(1) {}

    void addHeat(size_t h) {
        total_heat_ += h;
        nr_inst_ += 1;
    }

    instID_t getStartInst() const { return start_inst_; }
    size_t getNrInst() const { return nr_inst_; }
    size_t getTotalHeat() const { return total_heat_; }
    double getAverageHeat() const { return (double)total_heat_ / nr_inst_; }

    bool isHot() const { return false; }
    bool isCold() const { return !isHot(); }

    bool operator<(const CodeBlock &cb) const {
        return start_inst_ < cb.getStartInst();
    }

private:
    instID_t start_inst_;
    size_t total_heat_;
    size_t nr_inst_;
};

void HeatAnalyze::analyze(DepGraph *graph) {
    std::map<instID_t, size_t> inst_exec_nr_table;

    for (auto it = graph->begin(), end = graph->end(); it != end; ++it) {
        const Event &evt = *it;
        if (!evt.isAccessEvent())
            continue;

        auto inst = evt.getInstUniqueID();
        inst_exec_nr_table[inst] += 1;
    }

    std::set<CodeBlock> cblocks;
    auto build_block = [&cblocks](instID_t s) -> CodeBlock* {
        auto ret = cblocks.insert(CodeBlock{s});
        return const_cast<CodeBlock*>(&(*ret.first));
    };

    auto tids = graph->getThreadIds();
    for (auto tid : tids) {
        auto &nd_list = graph->getThreadEvents(tid);

        auto *blk = build_block(nd_list.front().getEvent().getInstUniqueID());
        for (auto it = nd_list.begin(); it != nd_list.end(); ++it) {
            const Event &evt = it->getEvent();
            if (evt.isSyncOpEvent() || evt.isThreadOpEvent()) {
                blk = build_block(evt.getInstUniqueID());
            } else if (evt.isAccessEvent()) {
                blk->addHeat(inst_exec_nr_table[evt.getInstUniqueID()]);
            }
        }
    }

    for (auto &cb : cblocks) {
        std::cout << CodeBlock::stringfy(cb) << std::endl;
    }
}
