#include "InstRace.h"

#include <sstream>

InstRacePair::InstRacePair(const Event *first_evt, const Event *second_evt) {
    assert(first_evt->isVarAccessEvent());

    first_inst_id_ = first_evt->getInstUniqueID();
    second_inst_id_ = second_evt->getInstUniqueID();

    first_op_ = first_evt->getType() == Event::OP_LD ? 'r' : 'w';
    second_op_ = second_evt->getType() == Event::OP_LD ? 'r' : 'w';

    if (first_inst_id_ > second_inst_id_) {
        std::swap(first_inst_id_, second_inst_id_);
        std::swap(first_op_, second_op_);
    }

    is_harmful_ = true;
}

std::string InstRacePair::stringfy() const {
    return Aux::format("%lu:%c %lu:%c", first_inst_id_, first_op_,
                                        second_inst_id_, second_op_);
}

// InstRacePair InstRacePair::build(const Event *first_evt, const Event *second_evt) {
    // return InstRacePair{first_evt, second_evt};
// }


bool InstRacePair::operator == (const InstRacePair &other) const {
    return firstInst() == other.firstInst() && secondInst() == other.secondInst();
}

bool InstRacePair::operator < (const InstRacePair &other) const {
    return firstInst() < other.firstInst() ||
           (firstInst() == other.firstInst() && secondInst() < other.secondInst());
}

std::ostream& operator << (std::ostream &out, const InstRacePair &race_inst_pair) {
    out << race_inst_pair.first_inst_id_ << ' ' << race_inst_pair.second_inst_id_;
    // out << ' ' << race_inst_pair.first_op_ << ' ' << race_inst_pair.second_op_;
    return out;
}


bool InstRaceTable::addRace(const Event *first_evt, const Event *second_evt) {
    InstRacePair inst_race{first_evt, second_evt};

    RuntimeRace run_race{first_evt, second_evt};

    race_table_[inst_race].emplace_back(first_evt, second_evt);

    return true;
}

bool InstRaceTable::dumpFile(std::ofstream &fout) {
    for (auto &entry: race_table_) {
        fout << entry.first.stringfy();
        fout << "\n--- " << entry.second.size() << " ---\n";
        for (auto &run_race: entry.second) {
            fout << run_race.stringfy() << "\n";
        }
    }

    return true;
}
