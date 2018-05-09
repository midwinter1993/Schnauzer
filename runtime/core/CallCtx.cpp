#include "CallCtx.h"

#include <cassert>
#include <vector>

#include "common/auxiliary.h"
#include "common/Console.h"

namespace CallCtx {

static __thread std::vector<uint64_t> *__call_stack = nullptr;

void pushCaller(uint64_t inst_id) {
    if (!__call_stack) {
        __call_stack = new std::vector<uint64_t>;
    }
    __call_stack->push_back(inst_id);
}

void popCaller(uint64_t inst_id) {
    UNUSED(inst_id);
    if (!__call_stack) {
        __call_stack = new std::vector<uint64_t>;
    }
    assert(!__call_stack->empty());
    if ( __call_stack->back() != inst_id) {
        for (auto i : *__call_stack) {
            Console::info("%lu ", i);
        }
        Console::info(" \n%lu\n", inst_id);
        assert(0);
    }
    __call_stack->pop_back();
}

const std::vector<uint64_t>* getCallStack() {
    return __call_stack;
}


}
