#ifndef __RUNTIME_CORE_EXECUTOR_H__
#define __RUNTIME_CORE_EXECUTOR_H__

#include <cstdint>
#include <string>
#include "common/basic.h"
#include "common/Config.h"
#include "common/Console.h"

class Executor {
public:
    Executor() { }
    void checkScheduler(const std::string &name) {
        auto sched_name = Config::getRuntimeConfig()->getString("SCHEDULER");
        if (sched_name != name) {
            Console::err("SCHEDULER Not match\n");
            Console::err("THIS: %s\n", name.c_str());
            Console::err("CONFIG: %s\n", sched_name.c_str());
            exit(0);
        }
    }
    virtual ~Executor() { }
    //-----------------------------------------------
    virtual void programStart() { }
    virtual void programExit() { }
    //-----------------------------------------------
    virtual void threadStart() { }
    virtual void threadExit() { }
    //-----------------------------------------------
    virtual void beforeLoad(instID_t inst_id, void *var_addr) { }
    virtual void afterLoad(instID_t inst_id, void *var_addr) { }
    //-----------------------------------------------
    virtual void beforeStore(instID_t inst_id, void *var_addr) { }
    virtual void afterStore(instID_t inst_id, void *var_addr) { }
    //-----------------------------------------------
    virtual void beforeFuncCall(instID_t inst_id) { }
    virtual void afterFuncCall(instID_t inst_id) { }
    //-----------------------------------------------
    // Before thread creating, tid has not been assigned.
    virtual void beforePthreadCreate(instID_t inst_id, pthread_t *tid_ptr) { }
    virtual void afterPthreadCreate(instID_t inst_id, tid_t t) { }
    //-----------------------------------------------
    // virtual void beforePthreadExit(instID_t inst_id) { }
    //-----------------------------------------------
    virtual void beforePthreadJoin(instID_t inst_id, tid_t t) { }
    virtual void afterPthreadJoin(instID_t inst_id, tid_t t) { }
    //-----------------------------------------------
    virtual void afterFork(instID_t inst_id, int pid) { }
    //-----------------------------------------------
    virtual void beforeFree(instID_t inst_id, void *addr) { }
    virtual void afterFree(instID_t inst_id, void *addr) { }
    //-----------------------------------------------
    virtual void beforePthreadMutexLock(instID_t inst_id, pthread_mutex_t* m) { }
    virtual void afterPthreadMutexLock(instID_t inst_id, pthread_mutex_t* m) { }

    virtual void beforePthreadMutexUnlock(instID_t inst_id, pthread_mutex_t* m) { }
    virtual void afterPthreadMutexUnlock(instID_t inst_id, pthread_mutex_t* m) { }

    virtual void beforePthreadMutexTrylock(instID_t inst_id, pthread_mutex_t* m) { }
    virtual void afterPthreadMutexTrylock(instID_t inst_id, pthread_mutex_t* m) { }

    virtual void beforePthreadCondWait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m) { }
    virtual void afterPthreadCondWait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m) { }

    virtual void beforePthreadCondTimedwait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m,const struct timespec* t) { }
    virtual void afterPthreadCondTimedwait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m,const struct timespec* t) { }

    virtual void beforePthreadCondSignal(instID_t inst_id, pthread_cond_t* c) { }
    virtual void afterPthreadCondSignal(instID_t inst_id, pthread_cond_t* c) { }

    virtual void beforePthreadCondBroadcast(instID_t inst_id, pthread_cond_t* c) { }
    virtual void afterPthreadCondBroadcast(instID_t inst_id, pthread_cond_t* c) { }

    virtual void beforePthreadRwlockRdlock(instID_t inst_id, pthread_rwlock_t* m) { }
    virtual void afterPthreadRwlockRdlock(instID_t inst_id, pthread_rwlock_t* m) { }

    virtual void beforePthreadRwlockWrlock(instID_t inst_id, pthread_rwlock_t* m) { }
    virtual void afterPthreadRwlockWrlock(instID_t inst_id, pthread_rwlock_t* m) { }

    virtual void beforePthreadRwlockTryrdlock(instID_t inst_id, pthread_rwlock_t* m) { }
    virtual void afterPthreadRwlockTryrdlock(instID_t inst_id, pthread_rwlock_t* m) { }

    virtual void beforePthreadRwlockTrywrlock(instID_t inst_id, pthread_rwlock_t* m) { }
    virtual void afterPthreadRwlockTrywrlock(instID_t inst_id, pthread_rwlock_t* m) { }

    virtual void beforePthreadRwlockUnlock(instID_t inst_id, pthread_rwlock_t* m) { }
    virtual void afterPthreadRwlockUnlock(instID_t inst_id, pthread_rwlock_t* m) { }

    virtual void beforeMemchr(instID_t inst_id, const void* str,int c,size_t n) { }
    virtual void afterMemchr(instID_t inst_id, const void* str,int c,size_t n) { }

    virtual void beforeMemcmp(instID_t inst_id, const void* str1,const void* str2,size_t n) { }
    virtual void afterMemcmp(instID_t inst_id, const void* str1,const void* str2,size_t n) { }

    virtual void beforeMemcpy(instID_t inst_id, void* dest,const void* src,size_t n) { }
    virtual void afterMemcpy(instID_t inst_id, void* dest,const void* src,size_t n) { }

    virtual void beforeMemmove(instID_t inst_id, void* dest,const void* src,size_t n) { }
    virtual void afterMemmove(instID_t inst_id, void* dest,const void* src,size_t n) { }

    virtual void beforeMemset(instID_t inst_id, void* str,int c,size_t n) { }
    virtual void afterMemset(instID_t inst_id, void* str,int c,size_t n) { }

    virtual void beforeStrcat(instID_t inst_id, char* dest,const char* src) { }
    virtual void afterStrcat(instID_t inst_id, char* dest,const char* src) { }

    virtual void beforeStrncat(instID_t inst_id, char* dest,const char* src,size_t n) { }
    virtual void afterStrncat(instID_t inst_id, char* dest,const char* src,size_t n) { }

    virtual void beforeStrchr(instID_t inst_id, const char* str,int c) { }
    virtual void afterStrchr(instID_t inst_id, const char* str,int c) { }

    virtual void beforeStrcmp(instID_t inst_id, const char* str1,const char* str2) { }
    virtual void afterStrcmp(instID_t inst_id, const char* str1,const char* str2) { }

    virtual void beforeStrncmp(instID_t inst_id, const char* str1,const char* str2,size_t n) { }
    virtual void afterStrncmp(instID_t inst_id, const char* str1,const char* str2,size_t n) { }

    virtual void beforeStrcoll(instID_t inst_id, const char* str1,const char* str2) { }
    virtual void afterStrcoll(instID_t inst_id, const char* str1,const char* str2) { }

    virtual void beforeStrcpy(instID_t inst_id, char* dest,const char* src) { }
    virtual void afterStrcpy(instID_t inst_id, char* dest,const char* src) { }

    virtual void beforeStrncpy(instID_t inst_id, char* dest,const char* src,size_t n) { }
    virtual void afterStrncpy(instID_t inst_id, char* dest,const char* src,size_t n) { }

    virtual void beforeStrcspn(instID_t inst_id, const char* str1,const char* str2) { }
    virtual void afterStrcspn(instID_t inst_id, const char* str1,const char* str2) { }

    virtual void beforeStrerror(instID_t inst_id, int errnum) { }
    virtual void afterStrerror(instID_t inst_id, int errnum) { }

    virtual void beforeStrlen(instID_t inst_id, const char* str) { }
    virtual void afterStrlen(instID_t inst_id, const char* str) { }

    virtual void beforeStrpbrk(instID_t inst_id, const char* str1,const char* str2) { }
    virtual void afterStrpbrk(instID_t inst_id, const char* str1,const char* str2) { }

    virtual void beforeStrrchr(instID_t inst_id, const char* str,int c) { }
    virtual void afterStrrchr(instID_t inst_id, const char* str,int c) { }

    virtual void beforeStrspn(instID_t inst_id, const char* str1,const char* str2) { }
    virtual void afterStrspn(instID_t inst_id, const char* str1,const char* str2) { }

    virtual void beforeStrstr(instID_t inst_id, const char* haystack,const char* needle) { }
    virtual void afterStrstr(instID_t inst_id, const char* haystack,const char* needle) { }

    virtual void beforeStrtok(instID_t inst_id, char* str,const char* delim) { }
    virtual void afterStrtok(instID_t inst_id, char* str,const char* delim) { }

    virtual void beforeStrxfrm(instID_t inst_id, char* dest,const char* src,size_t n) { }
    virtual void afterStrxfrm(instID_t inst_id, char* dest,const char* src,size_t n) { }

};

#endif /* ifndef __RUNTIME_CORE_EXECUTOR_H__ */
