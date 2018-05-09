#ifndef __RUNTIME_CORE_UTILS_H__
#define __RUNTIME_CORE_UTILS_H__

#include <cstdint>

namespace Utils {

void stickAllThreadToCore(int core_id);

int getThreadStickedCore();

// void stickCurrThreadToCore(int core_id);

bool isOnStack(const void *addr);
// sleep for t us
void safeSleep(int64_t t);

void segfaultHandler(int);

}

#endif /* ifndef __RUNTIME_CORE_UTILS_H__ */
