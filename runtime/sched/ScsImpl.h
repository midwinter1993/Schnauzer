#ifndef __RUNTIME_SCHED_SCS_IMPL_H__
#define __RUNTIME_SCHED_SCS_IMPL_H__

// enum SyncType {
    // ACQUIRE,
    // RELEASE,
    // CREATE,
    // JOIN,
    // LOCK,
    // UNLOCK,
    // TRYLOCK,
    // WAIT,
    // SIGNAL,
    // BROADCAST
// };
namespace ScsImpl {

void schedInit();
void schedCleanup();

void beforeSyncOperation();
void afterSyncOperation();

// void beforeSyncSrcInit();
// void afterSyncSrcInit();

// void beforeSyncSrcDestroy();
// void afterSyncSrcDestroy();

void beforeFuncCall();
void afterFuncCall();

}

#endif
