#ifndef __ANALYZE_DEPGRAPH_HEAT_ANALYZE_H__
#define __ANALYZE_DEPGRAPH_HEAT_ANALYZE_H__

#include "AnalyzePass.h"

class HeatAnalyze: public AnalyzePass {
public:
    void analyze(DepGraph *graph) override;

private:
};


#endif /* ifndef __ANALYZE_DEPGRAPH_HEAT_ANALYZE_H__ */
