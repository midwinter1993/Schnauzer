#ifndef __LLVM_INSTRUMENT_UTILS_IRHELPER_H__
#define __LLVM_INSTRUMENT_UTILS_IRHELPER_H__

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#if (__clang_minor__ == 4)
#include <llvm/DebugInfo.h>
#elif (__clang_minor__ == 8)
#include <llvm/IR/DebugInfo.h>
#endif

#include <sstream>

using namespace llvm;

// static std::vector<std::string> split(const std::string &s,
                                      // const std::string &delim) {
    // std::vector<std::string> v_ret;
    // std::string::size_type pos, prev_pos = 0;

    // do {
        // pos = s.find(delim, prev_pos);
        // v_ret.push_back(s.substr(prev_pos, pos - prev_pos));
        // prev_pos = pos + delim.size();
    // } while (pos != std::string::npos);
    // return v_ret;
// }

// static std::string snakeToCamel(const std::string &name) {
    // assert(name.size());

    // auto NameSegments = split(name, "_");
    // std::stringstream ss;
    // for (auto &N : NameSegments) {
        // N[0] = std::toupper(N[0]);
        // ss << N;
    // }

    // auto ret = ss.str();
    // ret[0] = std::tolower(ret[0]);
    // return ret;
// }

template <typename T>
struct traits {
    static const int type_idx = 0;
};

template <>
struct traits<void> {
    static const int type_idx = 1;
};

template <>
struct traits<void *> {
    static const int type_idx = 2;
};

template <>
struct traits<uint64_t> {
    static const int type_idx = 3;
};

template <>
struct traits<int32_t> {
    static const int type_idx = 4;
};

class IRHelper {
public:
    static std::string getSrcLineInfo(Function *F) {
        std::stringstream ss;
        if (!F->isDeclaration()) {
            auto SP = F->getSubprogram();
            ss << SP->getFilename().str() << ' ';
            ss << SP->getLine() << ": ",
            ss << SP->getName().str();
        } else {
            ss << "Unknown Function";
        }

        return ss.str();
    }
    static std::string getSrcLineInfo(Instruction *I) {
        uint64_t step_cnt = 0;
        while (I) {
            DILocation *loc = I->getDebugLoc();
            if (loc) {
                std::stringstream ss;
                ss << loc->getFilename().str() << ":" << loc->getLine();
                if (step_cnt) {
                    ss << "-" << step_cnt;
                }
                return ss.str();
            }
            step_cnt += 1;
            I = I->getPrevNode();
        }
        return "Unknown";
    }

public:
    IRHelper(Module &M) : Mod_(&M), Ctx_(&M.getContext()) {}
    IRHelper() {}

    template <typename... Arg>
    Constant *createFunction(const std::string &name) {
        return createFunctionImpl(name, traits<Arg>::type_idx..., 0);
    }

    Constant *createWrapperFunction(const Function *Func) {
        assert(Func);
        FunctionType *FuncType = Func->getFunctionType();
        Type *RetType = FuncType->getReturnType();

        std::vector<Type *> ParamTypes;
        ParamTypes.push_back(Type::getInt64Ty(*Ctx_));
        ParamTypes.insert(ParamTypes.end(), FuncType->param_begin(),
                          FuncType->param_end());

        assert(ParamTypes.size() == FuncType->getNumParams() + 1);
        assert(Func->getName().size());
        std::string wrapper_func_name = Func->getName().str() + "_wrapper";
        return Mod_->getOrInsertFunction(
            wrapper_func_name, FunctionType::get(RetType, ParamTypes, false));
    }

    ConstantInt *getConstantInt(uint64_t v) {
        return ConstantInt::get(Type::getInt64Ty(*Ctx_), v, false);
    }

    CallInst *createCall(Constant *func, ...) {
        std::vector<Value *> args;
        va_list vlist;
        va_start(vlist, func);
        while (1) {
            Value *val = va_arg(vlist, Value *);
            if (val == nullptr)
                break;

            args.push_back(val);
        }
        va_end(vlist);
        return CallInst::Create(func, args);
    }

    CallInst *replaceCallWithWrapper(CallInst *OldCall, Value *InstID) {
        Function *Func = OldCall->getCalledFunction();
        if (!Func) {
            return nullptr;
        }

        Constant *ReplaceFunc = createWrapperFunction(Func);

        if (ReplaceFunc != nullptr) {
            std::vector<Value *> args;
            args.push_back(InstID);
            args.insert(args.end(), OldCall->arg_begin(), OldCall->arg_end());
            CallInst *NewCall =
                CallInst::Create(ReplaceFunc, args, "", OldCall);

            NewCall->setCallingConv(OldCall->getCallingConv());
            NewCall->setAttributes(OldCall->getAttributes());
            NewCall->setDebugLoc(OldCall->getDebugLoc());
            if (OldCall->isTailCall()) {
                NewCall->setTailCall();
            }

            if (!OldCall->use_empty()) {
                OldCall->replaceAllUsesWith(NewCall);
                NewCall->takeName(OldCall);
            }

            return NewCall;
        }
        return nullptr;
    }

    CallInst *replaceCall(CallInst *OldCall, const std::string &NewCallName) {
        Function *Func = OldCall->getCalledFunction();
        if (!Func) {
            return NULL;
        }

        Constant *ReplaceFunc = Mod_->getOrInsertFunction(
            NewCallName, Func->getFunctionType(), Func->getAttributes());

        if (ReplaceFunc != NULL) {
            std::vector<Value *> args(OldCall->arg_begin(), OldCall->arg_end());
            CallInst *NewCall =
                CallInst::Create(ReplaceFunc, args, "", OldCall);

            NewCall->setCallingConv(OldCall->getCallingConv());
            NewCall->setAttributes(OldCall->getAttributes());
            NewCall->setDebugLoc(OldCall->getDebugLoc());
            if (OldCall->isTailCall()) {
                NewCall->setTailCall();
            }

            if (!OldCall->use_empty()) {
                OldCall->replaceAllUsesWith(NewCall);
                NewCall->takeName(OldCall);
            }

            return NewCall;
        }
        return nullptr;
    }

    BitCastInst *createCast(Value *ptr) {
        return new BitCastInst(ptr, Type::getInt8PtrTy(*Ctx_));
    }

    Instruction *createString(const std::string &s) {
        GlobalVariable *Gptr = createStringImpl(s);
        BitCastInst *Int8Ptr = createCast(Gptr);
        return Int8Ptr;
    }

    void insertBefore(Instruction *I, Instruction *i) {
#if (__clang_minor__ == 4)
        I->getParent()->getInstList().insert(I, i);
#elif (__clang_minor__ == 8)

        I->getParent()->getInstList().insert(I->getIterator(), i);
#endif
    }

    void insertAfter(Instruction *I, Instruction *i) {
#if (__clang_minor__ == 4)
        I->getParent()->getInstList().insertAfter(I, i);
#elif (__clang_minor__ == 8)
        I->getParent()->getInstList().insertAfter(I->getIterator(), i);
#endif
    }

    void insertAfter(Instruction *I, std::vector<Instruction *> Ilist) {
        for (auto it = Ilist.rbegin(), end = Ilist.rend(); it != end; ++it) {
            insertAfter(I, *it);
        }
    }

    void insertBefore(Instruction *I, std::vector<Instruction *> Ilist) {
        for (auto *i : Ilist) {
            insertBefore(I, i);
        }
    }

private:
    Module *Mod_;
    LLVMContext *Ctx_;

    Constant *createFunctionImpl(const std::string &name, int idx, ...) {
        static Type *TypeArray[] = {
            nullptr,
            Type::getVoidTy(*Ctx_),
            Type::getInt8PtrTy(*Ctx_),
            Type::getInt64Ty(*Ctx_),
            Type::getInt32Ty(*Ctx_)
        };

        std::vector<Type *> ParamTypes;
        Type *RetType = TypeArray[idx];

        va_list vlist;
        va_start(vlist, idx);
        while (1) {
            int idx = va_arg(vlist, int);
            if (idx == 0)
                break;
            ParamTypes.push_back(TypeArray[idx]);
        }
        va_end(vlist);

        return Mod_->getOrInsertFunction(
            name, FunctionType::get(RetType, ParamTypes, false));
    }

    GlobalVariable *createStringImpl(const std::string &s) {
        static std::map<std::string, GlobalVariable *> GstringMap;

        auto item = GstringMap.find(s);
        if (item != GstringMap.end())
            return item->second;

        // Create the type of global string variable: ArrayType
        ArrayType *ArrayTy_0 = ArrayType::get(
            IntegerType::get(Mod_->getContext(), 8), s.size() + 1);

        // Define the global string variable of
        GlobalVariable *Gptr = new GlobalVariable(
            *Mod_, ArrayTy_0, true, GlobalValue::InternalLinkage, 0);
        Gptr->setAlignment(1);

        // initilize global string variable with a constant string
        Gptr->setInitializer(ConstantDataArray::getString(*Ctx_, s));

        GstringMap[s] = Gptr;
        return Gptr;
    }
};

#endif /* ifndef __IRHELPER_H__ */
