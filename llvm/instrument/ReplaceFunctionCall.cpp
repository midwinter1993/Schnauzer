#include "Instrument.h"

bool Instrument::replaceFunctionCall(Module &M) {
    for (auto &F : M) {
        for (auto &B : F) {
            for (auto &I : B) {
                Instruction *inst = &I;
                if (CallInst *call = dyn_cast<CallInst>(inst)) {
                    replaceCall(call);
                }
            }
        }
    }
    // Modify the IR code
    return false;
}

static std::set<std::string> __func_not_wrapper = {
    "pthread_cancel",
    "pthread_yield",
    // "pthread_cleanup_push",
    // "pthread_cleanup_pop",
    "pthread_barrier_init",
    "pthread_barrier_destroy",
    "pthread_barrier_wait",

    "pthread_mutex_destroy",
    "pthread_mutex_init",
    "pthread_cond_destroy",
    "pthread_cond_init",

    "pthread_dummy",
    "pthread_kill",
    "pthread_detach",
    "pthread_equal",
    // "pthread_getconcurrency",
    "pthread_getschedparam",
    "pthread_getspecific",
    "pthread_key_create",
    "pthread_key_delete",
    // "pthread_mutex_getprioceiling",
    // "pthread_mutex_init",
    // "pthread_mutex_destroy",
    // "pthread_mutex_setprioceiling",

    "pthread_once",
    "pthread_rwlock_destroy",
    "pthread_rwlock_init",

    "pthread_self",
    "pthread_setcancelstate",
    "pthread_setcanceltype",
    // "pthread_setconcurrency",
    "pthread_setschedparam",
    "pthread_setspecific",
    "pthread_testcancel",
    "pthread_mutex_consistent_np",
    "pthread_sigmask",
    "pthread_getattr_np",
    "pthread_setname_np",
    "pthread_getname_np",
    "pthread_atfork"
};

void Instrument::replaceCall(CallInst *Call) {
    if (!Call) { return; }
    Function *Func = Call->getCalledFunction();

    if (!Func) { return; }
    auto FuncName = Func->getName();

    if (WrapFuncNames_.count(FuncName)) {
        auto NewCall = IRHelper_.replaceCallWithWrapper(Call, getInstId(Call));
        assert(NewCall && "New Call Instruction");
        // =============== NOTICE =========================
        // We can't erase this instruction because we still
        // iterator over the list.
        // ================================================
        RemoveList_.push_back(Call);
        return;
    }

    //
    // To check pthread functions wrappered well
    //
    if (FuncName.startswith("pthread_attr_") ||
        FuncName.startswith("pthread_condattr_") ||
        FuncName.startswith("pthread_mutexattr_") ||
        FuncName.startswith("pthread_rwlockattr_") ||
        __func_not_wrapper.count(FuncName)) {
        return;
    }

    if(FuncName.startswith("pthread_")) {
        errs() << FuncName << "\n";
        assert(false && "Error pthread function to handle");
    }
}
