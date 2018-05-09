#ifndef __RUNTIME_SCS_SC_SCHEDULER_H__
#define __RUNTIME_SCS_SC_SCHEDULER_H__

#include "runtime/core/Executor.h"
#include "runtime/core/Logger.h"

class ScScheduler: public Executor {
public:
    ScScheduler();
    ~ScScheduler() {}
    //-----------------------------------------------
    void programStart() override;
    void programExit() override;
    //-----------------------------------------------
    void threadStart() override;
    void threadExit() override;
    //-----------------------------------------------
    void beforeLoad(instID_t inst_id, void *var_addr) override;
    //-----------------------------------------------
    void beforeStore(instID_t inst_id, void *var_addr) override;
    //-----------------------------------------------
    void beforeFree(instID_t inst_id, void *var) override;
    //-----------------------------------------------
    void beforeFuncCall(instID_t inst_id) override;
    void afterFuncCall(instID_t inst_id) override;
    //-----------------------------------------------
    void beforePthreadCreate(instID_t inst_id, pthread_t *tid_ptr) override;
    void afterPthreadCreate(instID_t inst_id, tid_t t) override;

    void beforePthreadJoin(instID_t inst_id, tid_t t) override;
    void afterPthreadJoin(instID_t inst_id, tid_t t) override;
    //-----------------------------------------------
    void beforePthreadMutexLock(instID_t inst_id, pthread_mutex_t * m) override;
    void afterPthreadMutexLock(instID_t inst_id, pthread_mutex_t * m) override;
    //-----------------------------------------------
    void beforePthreadMutexTrylock(instID_t inst_id, pthread_mutex_t * m) override;
    void afterPthreadMutexTrylock(instID_t inst_id, pthread_mutex_t * m) override;
    //-----------------------------------------------
    void beforePthreadMutexUnlock(instID_t inst_id, pthread_mutex_t * m) override;
    void afterPthreadMutexUnlock(instID_t inst_id, pthread_mutex_t * m) override;
    //-----------------------------------------------
    void beforePthreadCondWait(instID_t inst_id, pthread_cond_t * c,pthread_mutex_t * m) override;
    void afterPthreadCondWait(instID_t inst_id, pthread_cond_t * c,pthread_mutex_t * m) override;
    //-----------------------------------------------
    void beforePthreadCondTimedwait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m,const struct timespec* t) override;
    void afterPthreadCondTimedwait(instID_t inst_id, pthread_cond_t* c,pthread_mutex_t* m,const struct timespec* t) override;
    //-----------------------------------------------
    void beforePthreadCondSignal(instID_t inst_id, pthread_cond_t * c) override;
    void afterPthreadCondSignal(instID_t inst_id, pthread_cond_t * c) override;
    //-----------------------------------------------
    void beforePthreadCondBroadcast(instID_t inst_id, pthread_cond_t * c) override;
    void afterPthreadCondBroadcast(instID_t inst_id, pthread_cond_t * c) override;
    //-----------------------------------------------
    void afterFork(instID_t inst_id, int pid) override;
    //-----------------------------------------------
    void beforeMemchr(instID_t inst_id, const void* str,int c,size_t n) override;
    void beforeMemcmp(instID_t inst_id, const void* str1,const void* str2,size_t n) override;
    void beforeMemcpy(instID_t inst_id, void* dest,const void* src,size_t n) override;
    void beforeMemmove(instID_t inst_id, void* dest,const void* src,size_t n) override;
    void beforeMemset(instID_t inst_id, void* str,int c,size_t n) override;
    void beforeStrcat(instID_t inst_id, char* dest,const char* src) override;
    void beforeStrncat(instID_t inst_id, char* dest,const char* src,size_t n) override;
    void beforeStrchr(instID_t inst_id, const char* str,int c) override;
    void beforeStrcmp(instID_t inst_id, const char* str1,const char* str2) override;
    void beforeStrncmp(instID_t inst_id, const char* str1,const char* str2,size_t n) override;
    void beforeStrcoll(instID_t inst_id, const char* str1,const char* str2) override;
    void beforeStrcpy(instID_t inst_id, char* dest,const char* src) override;
    void beforeStrncpy(instID_t inst_id, char* dest,const char* src,size_t n) override;
    void beforeStrcspn(instID_t inst_id, const char* str1,const char* str2) override;
    void beforeStrerror(instID_t inst_id, int errnum) override;
    void beforeStrlen(instID_t inst_id, const char* str) override;
    void beforeStrpbrk(instID_t inst_id, const char* str1,const char* str2) override;
    void beforeStrrchr(instID_t inst_id, const char* str,int c) override;
    void beforeStrspn(instID_t inst_id, const char* str1,const char* str2) override;
    void beforeStrstr(instID_t inst_id, const char* haystack,const char* needle) override;
    void beforeStrtok(instID_t inst_id, char* str,const char* delim) override;
    void beforeStrxfrm(instID_t inst_id, char* dest,const char* src,size_t n) override;

private:
};

#endif /* ifndef __RUNTIME_PROFILE_PROFILER_H__ */
