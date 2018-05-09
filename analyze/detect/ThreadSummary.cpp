#include "ThreadSummary.h"

#include <map>

namespace ThreadSummary {

static std::map<int, SummaryItem> __thread_summary_table;

SummaryItem& getThreadSummary(int tid) {
    return __thread_summary_table[tid];
}

yaml::YamlValue* asYaml() {
    yaml::YamlValue *y = new yaml::YamlValue();
    for (auto &i: __thread_summary_table) {
        y->addYaml(std::to_string(i.first), i.second.asYaml());
    }
    return y;
}

}
