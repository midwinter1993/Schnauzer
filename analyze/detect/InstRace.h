#ifndef __COMMON_INST_RACE_H__
#define __COMMON_INST_RACE_H__

#include <string>
#include <list>
#include <map>
#include "common/common.h"
#include "common/Event.h"


class InstRacePair {
public:
    // static InstRacePair build(const Event *first_evt, const Event *second_evt);
    InstRacePair(const Event *first_evt, const Event *second_evt);

public:
    instID_t firstInst() const { return first_inst_id_; }
    instID_t secondInst() const { return second_inst_id_; }
    bool isHarmful() const { return is_harmful_; }

    bool operator == (const InstRacePair &other) const;
    bool operator < (const InstRacePair &other) const;

    std::string stringfy() const;

    friend std::ostream& operator << (std::ostream &out, const InstRacePair &inst_pair);

private:
    InstRacePair() { }

    instID_t first_inst_id_;
    instID_t second_inst_id_;
    char first_op_;
    char second_op_;
    bool is_harmful_;
};

class InstRaceTable {
public:
    bool addRace(const Event *first_evt, const Event *second_evt);
    bool dumpFile(std::ofstream &fout);
    size_t size() const { return race_table_.size(); }

private:
    struct RuntimeRace {
        const Event *first_evt_;
        const Event *second_evt_;

        RuntimeRace() {
        }
        RuntimeRace(const Event *first_evt, const Event *second_evt) {
            assert(first_evt->getAddress() == second_evt->getAddress());
            first_evt_ = first_evt;
            second_evt_ = second_evt;
        }

        std::string stringfy() const {
            return Aux::format("%d:%lu:%lu %d:%lu:%lu %lu",
                               first_evt_->getThread(),
                               first_evt_->getTimeStamp(),
                               first_evt_->getInstUniqueID(),
                               second_evt_->getThread(),
                               second_evt_->getTimeStamp(),
                               second_evt_->getInstUniqueID(),
                               first_evt_->getAddress());
        }
    };

    std::map<InstRacePair, std::list<RuntimeRace>> race_table_;
};

#endif /* ifndef __COMMON_INST_RACE_H__ */
