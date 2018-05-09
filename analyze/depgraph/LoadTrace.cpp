#include <fstream>
#include <map>
#include "DepGraph.h"
#include "common/Event.h"
#include "common/auxiliary.h"

class TraceParser {
public:
    TraceParser(const std::string &trace_name) {
        Aux::openIstream(fin_, trace_name);
        fin_.seekg(0, std::ios::end);
        file_sz_ = fin_.tellg();
        fin_.seekg(0, std::ios::beg);
    }

    ~TraceParser() {
        if (fin_.is_open()) {
            fin_.close();
        }
    }

    bool isEOF() {
        return fin_.tellg() == file_sz_;
    }

    Event nextEvent() {
        Event evt = Event::loadEvent(fin_);
#ifndef COMPACT_EVENT
        assert(assert_cnt_ == evt.getTimeStamp());
#endif
        ++assert_cnt_;
        return evt;
    }

private:
    std::ifstream fin_;
    std::streamoff file_sz_;
    timestamp_t assert_cnt_ = 0;
};

void DepGraph::loadTrace(const std::string &trace_name) {
    trace_name_ = trace_name;
    TraceParser parser(trace_name);

    std::map<address_t, tid_t> last_accessor;
    std::set<address_t> has_written;

    while (!parser.isEOF()) {
        auto evt = parser.nextEvent();
        auto op_ty = evt.getType();

        if (evt.isSyncOpEvent()) {
            // Find all the mutex variables.
            sync_vars_.insert(evt.getAddress());
            addTotalEvent(evt);
        } else if (evt.isAccessEvent()) {
            // For array access, we expand it with single variable access events
            for (size_t l = 0; l < evt.getLength(); ++l) {
                auto evt_expand = evt.expandArrayAccessEvent(l);
                addTotalEvent(evt_expand);
                // Find all shared variables.
                auto addr = evt_expand.getAddress();

                if (op_ty == Event::OP_ST)
                    has_written.insert(addr);

                if (shared_vars_.count(addr))
                    continue;

                auto it = last_accessor.find(addr);
                if (it != last_accessor.end()) {
                    if (it->second != evt_expand.getThread()) {
                        shared_vars_.insert(addr);
                    }
                } else {
                    last_accessor[addr] = evt_expand.getThread();
                }
            }
        } else if (evt.isThreadOpEvent()) {
            // std::cout << "HAHA " << evt.stringfy() << std::endl;
            addTotalEvent(evt);
        } else if (evt.isFuncCallEvent()) {
        } else {
            assert("Unknown event type" && 0);
        }
    }

    // Save # of total events for PCT
    tot_nr_events_for_pct_ = tot_events_.size();

    // Remove read-only
    for (auto it = shared_vars_.begin(); it != shared_vars_.end();) {
        if (!has_written.count(*it))
            it = shared_vars_.erase(it);
        else
            ++it;
    }

    // Remove thread-local
    tot_events_.remove_if([this](const Event &evt) {
        return (evt.getType() == Event::OP_LD || evt.getType() == Event::OP_ST) &&
               !shared_vars_.count(evt.getAddress());
    });

    std::cerr << "# TOTAL EVENT : " << tot_nr_events_for_pct_ << "\n";
    std::cerr << "# TOTAL EVENT FILTERED: " << tot_events_.size() << "\n";
    std::cerr << "# SHARED VARS: " << shared_vars_.size() << "\n";
    std::cerr << "# SYNC VARS: " << getSyncVars().size() << "\n";
}
