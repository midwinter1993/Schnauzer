#ifndef __RUNTIME_UTILS_TID_H__
#define __RUNTIME_UTILS_TID_H__

#include <pthread.h>
#include "common/Event.h"

namespace Tid {

#define MAX_NR_THREAD 50


size_t getMaxNrThread();

size_t getNrThread();

tid_t getCurrTid();

tid_t getNewTid();
tid_t lookupTid(pthread_t tid);
bool isThreadAlive(tid_t tid);

void registerCurrThread();
void unregisterCurrThread();


void dumpAllThreads();

bool isMetaThread();
void setMetaThread();

}

#endif /* ifndef __RUNTIME_UTILS_TID_DISPATCH_H__ */
