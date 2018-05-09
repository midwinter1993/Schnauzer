#ifndef __ANALYZE_DETECT_DETECT_PASS_H___
#define __ANALYZE_DETECT_DETECT_PASS_H___

#include "analyze/depgraph/AnalyzePass.h"

class DetectPass: public AnalyzePass {
public:
    void analyze(DepGraph *graph) override;
};

#endif /* ifndef __ANALYZE_DETECT_DETECT_PASS_H___ */
