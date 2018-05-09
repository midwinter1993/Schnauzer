#include "Instrument.h"
#include "common/auxiliary.h"

bool Instrument::instrumentIR(Module &M) {
    for (auto &F: M) {
        instrFunctionEntrance(&F);
        for (auto &B: F) {
            for (auto &I: B) {
                Instruction *inst = &I;
                if (LoadInst *ld = dyn_cast<LoadInst>(inst)) {
                    instrLoad(ld);
                } else if (StoreInst *st = dyn_cast<StoreInst>(inst)) {
                    instrStore(st);
                } else if (CallInst *call = dyn_cast<CallInst>(inst)) {
                    instrCall(call);
                }
            }
        }
    }
    // Modify the IR code
    return true;
}

bool Instrument::specialFunction(Function *F) {
    if (!F) {
        return false;
    }
    static std::set<std::string> specials = {
        "_before_main",
        "_before_exit",
        "_before_func_call",
        "_after_func_call",
        "_before_load",
        "_after_load",
        "_before_store",
        "_after_store",
        "_enter_func",
        "_exit_func",
        "_after_fork"
    };

    //
    // llvm intrinsics for memory functions are not special
    // such as memcpy for llvm.memory.xxx.xxx
    //
    std::string FuncName = F->getName();
    for (auto &n : WrapFuncNames_) {
        if (Aux::contain(FuncName, n) && !Aux::startswith(n, "pthread_")) {
            return false;
        }
    }
    // llvm intrinsics or ends with 'wrapper' are special
    if (Aux::startswith(FuncName, "llvm") ||
        specials.count(FuncName) ||
        Aux::contain(FuncName, ".") ||
        Aux::endswith(FuncName, "wrapper")) {
        return true;
    }
    return false;
}

// =============================================================
// Instrument Function Entrance
// =============================================================
void Instrument::instrFunctionEntrance(Function *F) {

    if (F->getName() == "main") {
        Constant *EntranceFunc = IRHelper_.createFunction<void>("_before_main");
        auto *FirstInst = &(*(F->begin()->begin()));

        Instruction *CallInst = IRHelper_.createCall(EntranceFunc, nullptr);
        IRHelper_.insertBefore(FirstInst, CallInst);
    } else if (!specialFunction(F) && !F->isDeclaration()) {
        // Constant *EntranceFunc =
            // IRHelper_.createFunction<void, uint64_t>("_enter_func");
        // auto *FirstInst = &(*(F->begin()->begin()));

        // Instruction *CallInst = IRHelper_.createCall(EntranceFunc,
                                                     // getFuncId(F),
                                                     // nullptr);
        // IRHelper_.insertBefore(FirstInst, CallInst);
    }
}

// =============================================================
// Instrument Function Call
// =============================================================

void Instrument::instrCall(CallInst *Call) {

    Function *Func = Call->getCalledFunction();
    if (!Func || specialFunction(Func)) {
        return;
    }

    std::string FuncName = Func->getName().str();

    if (WrapFuncNames_.count(FuncName) && Aux::startswith(FuncName, "pthread_")) {
        return;
    }

    if (FuncName == "fork") {
        Stat_->nr_instr_fork_ += 1;
        Constant *AfterForkFunc =
            IRHelper_.createFunction<void, uint64_t, int32_t>("_after_fork");
        IRHelper_.insertAfter(Call,
                              IRHelper_.createCall(AfterForkFunc,
                                                   getInstId(Call),
                                                   Call,
                                                   nullptr));
        return;
    }
    // errs() << "INSTR Call " << FuncName << "\n";
    Constant *BeforeFunc =
        IRHelper_.createFunction<void, uint64_t>("_before_func_call");
    IRHelper_.insertBefore(
        Call, IRHelper_.createCall(BeforeFunc, getInstId(Call), nullptr));

    Constant *AfterFunc =
        IRHelper_.createFunction<void, uint64_t>("_after_func_call");
    IRHelper_.insertAfter(
        Call, IRHelper_.createCall(AfterFunc, getInstId(Call), nullptr));

    // auto FuncId = getFuncId(Func);
    // if (FuncId) {
        // Constant *ExitFunc = IRHelper_.createFunction<void, uint64_t>("_exit_func");
        // IRHelper_.insertAfter(Call, IRHelper_.createCall(ExitFunc,
                                                         // FuncId,
                                                         // nullptr));
    // }
}

// =============================================================
// Instrument Load and Store
// =============================================================

void Instrument::instrLoad(LoadInst *LD) {
    Stat_->nr_instr_ld_ += 1;
    LD->setVolatile(true);

    BitCastInst *cast = IRHelper_.createCast(LD->getPointerOperand());
    IRHelper_.insertBefore(LD, cast);

    Constant *BeforeFunc =
        IRHelper_.createFunction<void, uint64_t, void *>("_before_load");
    IRHelper_.insertBefore(
        LD, IRHelper_.createCall(BeforeFunc, getInstId(LD), cast, nullptr));

    Constant *AfterFunc =
        IRHelper_.createFunction<void, uint64_t, void *>("_after_load");
    IRHelper_.insertAfter(
        LD, IRHelper_.createCall(AfterFunc, getInstId(LD), cast, nullptr));
}

void Instrument::instrStore(StoreInst *ST) {
    Stat_->nr_instr_st_ += 1;
    ST->setVolatile(true);

    BitCastInst *cast = IRHelper_.createCast(ST->getPointerOperand());
    IRHelper_.insertBefore(ST, cast);

    Constant *BeforeFunc =
        IRHelper_.createFunction<void, uint64_t, void *>("_before_store");
    IRHelper_.insertBefore(
        ST, IRHelper_.createCall(BeforeFunc, getInstId(ST), cast, nullptr));

    Constant *AfterFunc =
        IRHelper_.createFunction<void, uint64_t, void *>("_after_store");
    IRHelper_.insertAfter(
        ST, IRHelper_.createCall(AfterFunc, getInstId(ST), cast, nullptr));
}
