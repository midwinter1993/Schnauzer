#ifndef __ANALYZE_DEPGRAPH_ANALYZE_PASS_H__
#define __ANALYZE_DEPGRAPH_ANALYZE_PASS_H__


class DepGraph;

class AnalyzePass {
public:
    virtual void visit(DepGraph *graph, bool isAsync);
    virtual void analyze(DepGraph *graph) = 0;
};


class DumpPass: public AnalyzePass {
public:
    void analyze(DepGraph *graph) override;
};


class VerifyPass: public AnalyzePass {
public:
    void analyze(DepGraph *graph) override;
};

#endif
