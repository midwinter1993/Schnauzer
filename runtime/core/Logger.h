#ifndef __RUNTIME_CORE_LOGGER_H__
#define __RUNTIME_CORE_LOGGER_H__

#include <atomic>
#include <string>
#include <limits>
#include <vector>
#include "common/Event.h"

namespace Logger {

void logVarAccessEvent(instID_t inst, tid_t tid, int op_ty, const void *addr);

void logArrayAccessEvent(instID_t inst,
                         tid_t tid,
                         int op_ty,
                         const void *addr,
                         size_t len);

void logSyncOpEvent(instID_t inst,
                    tid_t tid,
                    int op_ty,
                    const void *sync_var_addr);

void logThreadCreateEvent(instID_t inst,
                          tid_t tid,
                          tid_t remote_tid);

void logThreadJoinEvent(instID_t inst,
                        tid_t tid,
                        tid_t remote_tid);

void logCondWaitEvent(instID_t inst,
                      tid_t tid,
                      const void *mutex_addr,
                      const void *cond_addr);

void logCondAwakeEvent(instID_t inst,
                       tid_t tid,
                       const void *mutex_addr,
                       const void *cond_addr);

void logFuncCallEvent(instID_t inst, tid_t tid);

void startBatch(uint64_t nr);
void startBatchFor(int nr_addr, ...);
void endBatch();
// bool inBatch();

void init();
void reinit();
void save(const std::string &fname);

void enableLogger();
void disableLogger();

bool logLibcMemOp();
bool logMemAlloc();

void logMemAllocEvent(void *addr, size_t sz,
                      const std::vector<uint64_t>* callstack);
}

#endif
