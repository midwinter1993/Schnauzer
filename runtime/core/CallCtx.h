#ifndef __RUNTIME_CORE_CALL_CTX_H__
#define __RUNTIME_CORE_CALL_CTX_H__

#include <cstdint>
#include <vector>

namespace CallCtx {

void pushCaller(uint64_t func_id);
void popCaller(uint64_t func_id);

const std::vector<uint64_t>* getCallStack();

}

#endif /* ifndef __RUNTIME_CORE_CALL_CTX_H__ */
