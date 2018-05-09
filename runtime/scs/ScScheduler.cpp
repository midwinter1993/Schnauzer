#include "ScScheduler.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include "common/common.h"
#include "common/debug.h"
#include "runtime/core/Control.h"
#include "runtime/core/Logger.h"
#include "runtime/core/Tid.h"
#include "runtime/core/Utils.h"
#include "runtime/sched/ScsImpl.h"

// void exit_handler(int) {
// _before_exit();
// exit(0);
// }

extern Executor *exe;
void segfault_handler(int) {
    Console::err("SEGFAULT\n");
    Debug::dumpThreadStacktrace(0);
    Logger::save(Config::getConfig().tracePath() + ".tmp");
    exit(1);
}

ScScheduler::ScScheduler() {
    checkScheduler("SCS");
}

void ScScheduler::programStart() {
    Console::info("===========================\n");
    Console::info("Let's SCS %s\n", Aux::getProgName().c_str());
    Console::info("===========================\n");
    Logger::init();

    // Stick all thread to cpu core 1
    // stick_thread_to_core(1);

    ScsImpl::schedInit();

    // signal(SIGINT, exit_handler);
    if (Config::getRuntimeConfig()->getBool("SEGFAULT_HANDLE")) {
        signal(SIGSEGV, segfault_handler);
    }
}

void ScScheduler::programExit() {
    // sleep(1);
    static std::atomic_bool done(false);
    if (!done) {
        done = true;
    } else {
        return;
    }

    ScsImpl::schedCleanup();
    Console::info("Sched cleanup finish\n");

    Logger::save(Config::getConfig().tracePath());
    Console::info("===========================\n");
    Console::info("Happy ending for %s\n", Aux::getProgName().c_str());
    Console::info("===========================\n");

}

//-----------------------------------------------

void ScScheduler::threadStart() {
    Console::info("Thread Start %d\n", Tid::getCurrTid());
}

void ScScheduler::threadExit() {
    Console::info("Thread Exit %d\n", Tid::getCurrTid());

}

#define LOG_ARG inst_id, Tid::getCurrTid()

//-----------------------------------------------

void ScScheduler::beforeLoad(instID_t inst_id, void *var_addr) {
    if (Utils::isOnStack(var_addr))
        return;
    Logger::logVarAccessEvent(LOG_ARG, Event::OP_LD, var_addr);
}

void ScScheduler::beforeStore(instID_t inst_id, void *var_addr) {
    if (Utils::isOnStack(var_addr))
        return;
    // ScsImpl::beforeFuncCall();
    Logger::logVarAccessEvent(LOG_ARG, Event::OP_ST, var_addr);
}

void ScScheduler::beforeFree(instID_t inst_id, void *var) {
    // Free as write event to check data race
    Logger::logVarAccessEvent(LOG_ARG, Event::OP_ST, var);
}

void ScScheduler::beforeFuncCall(instID_t inst_id) {
    UNUSED(inst_id);
    // Logger::logFuncCallEvent(LOG_ARG);
    ScsImpl::beforeFuncCall();
}

void ScScheduler::afterFuncCall(instID_t inst_id) {
    UNUSED(inst_id);

    ScsImpl::afterFuncCall();
}
//-----------------------------------------------

// static __thread timestamp_t __ts = 0;
void ScScheduler::beforePthreadCreate(instID_t inst_id, pthread_t *tid_ptr) {
    UNUSED(inst_id);
    UNUSED(tid_ptr);

    ScsImpl::beforeSyncOperation();
    Logger::startBatch(1);
    // __ts = Logger::fetchAddLogSize();
}

void ScScheduler::afterPthreadCreate(instID_t inst_id, tid_t t) {
    ScsImpl::afterSyncOperation();
    Logger::logThreadCreateEvent(LOG_ARG, t);
    Logger::endBatch();
}

//-----------------------------------------------

void ScScheduler::beforePthreadJoin(instID_t inst_id, tid_t t) {
    UNUSED(inst_id);
    UNUSED(t);

    ScsImpl::beforeSyncOperation();
}

void ScScheduler::afterPthreadJoin(instID_t inst_id, tid_t t) {
    ScsImpl::afterSyncOperation();
    Logger::logThreadJoinEvent(LOG_ARG, t);
}

//-----------------------------------------------

void ScScheduler::beforePthreadMutexLock(instID_t inst_id,
                                                pthread_mutex_t *m) {
    UNUSED(inst_id);
    UNUSED(m);
    ScsImpl::beforeSyncOperation();
}

void ScScheduler::afterPthreadMutexLock(instID_t inst_id,
                                               pthread_mutex_t *m) {
    ScsImpl::afterSyncOperation();
    Logger::logSyncOpEvent(LOG_ARG, Event::OP_LOCK, m);
}

//-----------------------------------------------

void ScScheduler::beforePthreadMutexTrylock(instID_t inst_id,
                                                   pthread_mutex_t *m) {
    UNUSED(inst_id);
    UNUSED(m);

    ScsImpl::beforeSyncOperation();
}

void ScScheduler::afterPthreadMutexTrylock(instID_t inst_id,
                                                  pthread_mutex_t *m) {
    ScsImpl::afterSyncOperation();
    Logger::logSyncOpEvent(LOG_ARG, Event::OP_TRYLOCK, m);
}

//-----------------------------------------------

void ScScheduler::beforePthreadMutexUnlock(instID_t inst_id,
                                                  pthread_mutex_t *m) {
    ScsImpl::beforeSyncOperation();
    Logger::logSyncOpEvent(LOG_ARG, Event::OP_UNLOCK, m);
}

void ScScheduler::afterPthreadMutexUnlock(instID_t inst_id,
                                                 pthread_mutex_t *m) {
    UNUSED(inst_id);
    UNUSED(m);

    ScsImpl::beforeSyncOperation();
}
//-----------------------------------------------

void ScScheduler::beforePthreadCondWait(instID_t inst_id,
                                               pthread_cond_t *c,
                                               pthread_mutex_t *m) {
    ScsImpl::beforeSyncOperation();
    // WAIT semantic: unlock, wait
    Logger::logCondWaitEvent(LOG_ARG, m, c);
}

void ScScheduler::afterPthreadCondWait(instID_t inst_id,
                                              pthread_cond_t *c,
                                              pthread_mutex_t *m) {
    ScsImpl::afterSyncOperation();
    Logger::logCondAwakeEvent(LOG_ARG, m, c);
}

void ScScheduler::beforePthreadCondTimedwait(instID_t inst_id,
                                                    pthread_cond_t *c,
                                                    pthread_mutex_t *m,
                                                    const struct timespec* t) {
    UNUSED(t);

    ScsImpl::beforeSyncOperation();
    Logger::logCondWaitEvent(LOG_ARG, m, c);
}

void ScScheduler::afterPthreadCondTimedwait(instID_t inst_id,
                                                   pthread_cond_t *c,
                                                   pthread_mutex_t *m,
                                                   const struct timespec* t) {
    UNUSED(t);

    ScsImpl::afterSyncOperation();
    Logger::logCondAwakeEvent(LOG_ARG, m, c);
}

//-----------------------------------------------

void ScScheduler::beforePthreadCondSignal(instID_t inst_id,
                                                 pthread_cond_t *c) {
    ScsImpl::beforeSyncOperation();
    Logger::logSyncOpEvent(LOG_ARG, Event::OP_SIGNAL, c);
}

void ScScheduler::afterPthreadCondSignal(instID_t inst_id,
                                                pthread_cond_t *c) {
    UNUSED(inst_id);
    UNUSED(c);

    ScsImpl::afterSyncOperation();
}

//-----------------------------------------------

void ScScheduler::beforePthreadCondBroadcast(instID_t inst_id,
                                                    pthread_cond_t *c) {
    ScsImpl::beforeSyncOperation();
    Logger::logSyncOpEvent(LOG_ARG, Event::OP_BROADCAST, c);
}

void ScScheduler::afterPthreadCondBroadcast(instID_t inst_id,
                                                   pthread_cond_t *c) {
    UNUSED(inst_id);
    UNUSED(c);

    ScsImpl::afterSyncOperation();
}

//-----------------------------------------------

void ScScheduler::afterFork(instID_t inst_id, int pid) {
    UNUSED(inst_id);

    if (pid == 0) {
        Logger::reinit();
    } else {
        Console::info("FORK son pid %d\n", pid);
    }
}

//-----------------------------------------------

void ScScheduler::beforeMemchr(instID_t inst_id, const void* str,int c,size_t n) {
    UNUSED(c);

    if (!Logger::logLibcMemOp())
        return;
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str, n);
}

void ScScheduler::beforeMemcmp(instID_t inst_id, const void* str1,const void* str2,size_t n) {
    if (!Logger::logLibcMemOp())
        return;
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n);
    Logger::endBatch();
}

void ScScheduler::beforeMemcpy(instID_t inst_id, void* dest,const void* src,size_t n) {
    if (!Logger::logLibcMemOp())
        return;
    Logger::startBatchFor(2, dest, src);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest, n);
    Logger::endBatch();
}

void ScScheduler::beforeMemmove(instID_t inst_id, void* dest,const void* src,size_t n) {
    if (!Logger::logLibcMemOp())
        return;
    Logger::startBatchFor(2, dest, src);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest, n);
    Logger::endBatch();
}

void ScScheduler::beforeMemset(instID_t inst_id, void* str,int c,size_t n) {
    UNUSED(c);
    // if (!Logger::logLibcMemOp())
        // return;
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, str, n);
}

void ScScheduler::beforeStrcat(instID_t inst_id, char* dest,const char* src) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_src = strlen(src);
    size_t n_dest = strlen(dest);
    Logger::startBatchFor(3, dest, src, dest + n_dest);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n_src);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, dest, n_dest);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest + n_dest, n_src);
    Logger::endBatch();
}

void ScScheduler::beforeStrncat(instID_t inst_id, char* dest,const char* src,size_t n) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_dest = strlen(dest);
    Logger::startBatchFor(3, dest, src, dest + n_dest);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, dest, n_dest);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest + n_dest, n);
    Logger::endBatch();
}

void ScScheduler::beforeStrchr(instID_t inst_id, const char* str,int c) {
    UNUSED(c);
    if (!Logger::logLibcMemOp())
        return;
    size_t n = strlen(str);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str, n);
}

void ScScheduler::beforeStrcmp(instID_t inst_id, const char* str1,const char* str2) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_src1 = strlen(str1);
    size_t n_src2 = strlen(str2);
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n_src1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n_src2);
    Logger::endBatch();
}

void ScScheduler::beforeStrncmp(instID_t inst_id, const char* str1,const char* str2,size_t n) {
    UNUSED(n);
    if (!Logger::logLibcMemOp())
        return;
    size_t n_src1 = strlen(str1);
    size_t n_src2 = strlen(str2);
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n_src1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n_src2);
    Logger::endBatch();
}

void ScScheduler::beforeStrcoll(instID_t inst_id, const char* str1,const char* str2) {
    UNUSED(inst_id);
    UNUSED(str1);
    UNUSED(str2);
}

void ScScheduler::beforeStrcpy(instID_t inst_id, char* dest,const char* src) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n = strlen(src);
    Logger::startBatchFor(2, dest, src);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest, n);
    Logger::endBatch();
}

void ScScheduler::beforeStrncpy(instID_t inst_id, char* dest,const char* src,size_t n) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_src = strlen(src);
    n = n_src > n ? n_src : n;
    Logger::startBatchFor(2, dest, src);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, src, n);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_ST, dest, n);
    Logger::endBatch();
}

void ScScheduler::beforeStrcspn(instID_t inst_id, const char* str1,const char* str2) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_str1 = strlen(str1);
    size_t n_str2 = strlen(str2);
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n_str1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n_str2);
    Logger::endBatch();
}

void ScScheduler::beforeStrerror(instID_t inst_id, int errnum) {
    UNUSED(inst_id);
    UNUSED(errnum);
}

void ScScheduler::beforeStrlen(instID_t inst_id, const char* str) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n = strlen(str);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str, n);
}

void ScScheduler::beforeStrpbrk(instID_t inst_id, const char* str1,const char* str2) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_str1 = strlen(str1);
    size_t n_str2 = strlen(str2);
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n_str1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n_str2);
    Logger::endBatch();
}

void ScScheduler::beforeStrrchr(instID_t inst_id, const char* str,int c) {
    UNUSED(c);

    if (!Logger::logLibcMemOp())
        return;
    size_t n = strlen(str);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str, n);
}

void ScScheduler::beforeStrspn(instID_t inst_id, const char* str1,const char* str2) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_str1 = strlen(str1);
    size_t n_str2 = strlen(str2);
    Logger::startBatchFor(2, str1, str2);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str1, n_str1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, str2, n_str2);
    Logger::endBatch();
}

void ScScheduler::beforeStrstr(instID_t inst_id, const char* haystack,const char* needle) {
    if (!Logger::logLibcMemOp())
        return;
    size_t n_1 = strlen(needle);
    size_t n_2 = strlen(haystack);
    Logger::startBatchFor(2, haystack, needle);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, needle, n_1);
    Logger::logArrayAccessEvent(LOG_ARG, Event::OP_LD, haystack, n_2);
    Logger::endBatch();

}

void ScScheduler::beforeStrtok(instID_t inst_id, char* str,const char* delim) {
    UNUSED(inst_id);
    UNUSED(str);
    UNUSED(delim);
}

void ScScheduler::beforeStrxfrm(instID_t inst_id, char* dest,const char* src,size_t n) {
    UNUSED(inst_id);
    UNUSED(dest);
    UNUSED(src);
    UNUSED(n);
}
