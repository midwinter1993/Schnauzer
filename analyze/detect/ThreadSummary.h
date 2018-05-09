#ifndef __ANALYZE_DETECT_THREAD_SUMMARY_H__
#define __ANALYZE_DETECT_THREAD_SUMMARY_H__

#include <cstddef>
#include <set>
#include <string>
#include "common/Yaml.h"

namespace ThreadSummary {

static std::set<std::string> __item_names = {
    "nr_event",

    "nr_before_event_race",
    "nr_after_event_race",
    "before_event_race_distance",
    "after_event_race_distance",

    "nr_before_sync_hb",
    "nr_after_sync_hb",
    "before_sync_hb_distance",
    "after_sync_hb_distance",
};

class SummaryItem {
public:
    SummaryItem(): yaml_(new yaml::YamlValue()) {
        assert(yaml_);

        for (auto &item_name: __item_names) {
            yaml_->addNumber(item_name, 0);
        }
    }

    yaml::YamlValue* asYaml() const {
       return yaml_;
    }

    void incItem(const std::string &item_name, int64_t v) {
        assert(__item_names.count(item_name));
        assert(v > 0);
        auto old_v = yaml_->getNumber(item_name);
        yaml_->setNumber(item_name, old_v + v);
    }

private:
    yaml::YamlValue *yaml_;
};

SummaryItem& getThreadSummary(int tid);

yaml::YamlValue* asYaml();

}

#endif /* ifndef __ANALYZE_DETECT_THREAD_SUMMARY_H__ */
