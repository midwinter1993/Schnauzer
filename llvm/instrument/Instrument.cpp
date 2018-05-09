#include "Instrument.h"

#include <stdio.h>
#include <fstream>
#include <limits>
#include <system_error>

#include <llvm/PassRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include "common/auxiliary.h"
#include "common/Config.h"

// static cl::opt<std::string>
    // MyAction("my-action",
               // cl::desc("Some action"));

// static cl::opt<bool>
    // NeedReplaceFunc("need-replace-func",
               // cl::desc("To replace pthread_create function"));


Instrument::Instrument(): ModulePass(ID) { }

bool Instrument::runOnModule(Module &M) {
    // this->aa_ = &getAnalysis<AliasAnalysis>();
    ModId_ = loadModuleId(M.getName());

    assert(ModId_ != std::numeric_limits<uint64_t>::max()
            && "Faile to load module ID");

    errs().changeColor(raw_ostream::CYAN);
    errs() << "==============================================\n";
    errs() << "INSTR: " << M.getName() << "; ";
    errs() << "MODULE ID: " << ModId_ << ";\n NAME:  " << M.getName() << "\n";
    errs() << "----------------------------------------------\n";
    errs().resetColor();

    Mod_ = &M;
    Ctx_ = &getGlobalContext();
    IRHelper_ = IRHelper(*Mod_);
    Stat_ = std::unique_ptr<Statistic>(new Statistic());

    loadWrapFuncDefs();

    // We must collect the instructions first.
    collectSrcInfo(M);

    instrumentIR(M);
    replaceFunctionCall(M);

    // We must erase the instructions lastly.
    for (auto &inst: RemoveList_) {
        inst->eraseFromParent();
    }

    errs().changeColor(raw_ostream::CYAN);
    errs() << "----------------------------------------------\n";
    errs() << "# LOAD: " << Stat_->nr_instr_ld_ << "\n";
    errs() << "# STORE: " << Stat_->nr_instr_st_ << "\n";
    errs() << "# FORK: " << Stat_->nr_instr_fork_ << "\n";
    // errs() << "# CREATE: " << Stat_->nr_instr_create_ << "\n";
    errs() << "==============================================\n";
    errs().resetColor();

    // Modify the IR code
    return true;
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("instrument", "Instrumentation", false, false);


// To use, run: clang -Xclang -load -Xclang <your-pass>.so <other-args> ...

// This function is of type PassManagerBuilder::ExtensionFn
static void loadPass(const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
    UNUSED(Builder);
    PM.add(new Instrument());
}

// To see the code as it is coming out of the frontend BUG
// static RegisterStandardPasses clangtoolLoader_Ox(PassManagerBuilder::EP_EarlyAsPossible, loadPass);

static RegisterStandardPasses clangtoolLoader_Ox(PassManagerBuilder::EP_OptimizerLast, loadPass);
static RegisterStandardPasses clangtoolLoader_O0(PassManagerBuilder::EP_EnabledOnOptLevel0, loadPass);
