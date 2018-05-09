#ifndef __LLVM_INSTRUMENT_INSTRUMENT_H__
#define	__LLVM_INSTRUMENT_INSTRUMENT_H__

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#if (__clang_minor__ == 4)
    #include <llvm/DebugInfo.h>
#elif (__clang_minor__ == 8)
    #include <llvm/IR/DebugInfo.h>
#endif

#include "llvm/utils/IRHelper.h"

using namespace llvm;

class Instrument: public ModulePass {
public:
    static char ID;

    Instrument();
    virtual bool runOnModule(Module &M);
private:

    uint64_t computeUniqueId(uint64_t ModId, uint64_t Id);
    uint64_t computeModId(uint64_t Id);
    uint64_t computeInstId(uint64_t Id);

    void collectSrcInfo(Module &M);

    ConstantInt* getFuncId(Function *F);
    ConstantInt* getInstId(Instruction *I);

    bool removeFree(Module &M);
    bool instrumentIR(Module &M);
    bool locateInstruction(Module &M);
    bool replaceFunctionCall(Module &M);

    bool specialFunction(Function *F);
    void instrFunctionEntrance(Function *F);
    void instrLoad(LoadInst *LD);
    void instrStore(StoreInst *ST);
    void instrCall(CallInst *Call);
    void replaceCall(CallInst *Call);

    uint64_t loadModuleId(const std::string &ModName);

    void loadWrapFuncDefs();

    struct Statistic {
        size_t nr_instr_ld_ = 0;
        size_t nr_instr_st_ = 0;
        size_t nr_instr_fork_ = 0;
        size_t nr_instr_create_ = 0;
    };

private:
    Module *Mod_;
    LLVMContext *Ctx_;
    IRHelper IRHelper_;
    uint64_t ModId_;
    std::map<Instruction*, uint64_t> InstIdMap_;
    std::map<Function*, uint64_t> FuncIdMap_;
    std::vector<CallInst*> RemoveList_;
    std::unique_ptr<Statistic> Stat_;
    std::set<std::string> WrapFuncNames_;
};

#endif
