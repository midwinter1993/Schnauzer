#include <cassert>
#include <atomic>
#include <iostream>
#include <thread>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

#include "common/Console.h"
#include "common/Config.h"
#include "common/debug.h"
#include "common/auxiliary.h"
#include "Control.h"
#include "Tid.h"
#include "Logger.h"
#include "Executor.h"
#include "Utils.h"
#include "CallCtx.h"

extern Executor *exe;


extern "C" {

static bool need_instrument() {
    return Control::isStarted() && !Control::isFinished() && !Tid::isMetaThread();
}

#define DO_INSTRUMENT(stmt)           \
    do {                              \
        if (need_instrument()) { stmt; } \
    } while(false)

/////////////////////////////////////////////////
void _before_exit() {
    static bool has_clean_up = false;
    // if (!need_instrument()) {
        // return;
    // }

    if (has_clean_up)
        return;
    has_clean_up = true;
    Control::setFinished();
    exe->programExit();
}

void exit_handler(int) {
    _before_exit();
    exit(0);
}

void _before_main() {
    // Control::setStarted();

    if (std::atexit(_before_exit) != 0) {
        Console::err("Registration atexit failed\n");
        exit(0);
    }

    // To change the thread id dispatcher,
    // we must register each thread created.
    Tid::registerCurrThread();

    exe->programStart();
    exe->threadStart();
}

//-----------------------------------------------

void _before_load(uint64_t inst_id, void *var_addr) {
    Control::incGlobalTick();
    if (Utils::isOnStack(var_addr))
        return;
    DO_INSTRUMENT(exe->beforeLoad(inst_id, var_addr));
}

void _after_load(uint64_t inst_id, void *var_addr) {
    if (Utils::isOnStack(var_addr))
        return;
    DO_INSTRUMENT(exe->afterLoad(inst_id, var_addr));
}

//-----------------------------------------------

void _before_store(uint64_t inst_id, void *var_addr) {
    Control::incGlobalTick();
    if (Utils::isOnStack(var_addr))
        return;
    DO_INSTRUMENT(exe->beforeStore(inst_id, var_addr));
}

void _after_store(uint64_t inst_id, void *var_addr) {
    if (Utils::isOnStack(var_addr))
        return;
    DO_INSTRUMENT(exe->afterStore(inst_id, var_addr));
}

void _before_func_call(uint64_t inst_id) {
    CallCtx::pushCaller(inst_id);
    DO_INSTRUMENT(exe->beforeFuncCall(inst_id));
}

void _after_func_call(uint64_t inst_id) {
    CallCtx::popCaller(inst_id);
    DO_INSTRUMENT(exe->afterFuncCall(inst_id));
}

void _enter_func(uint64_t func_id) {
    UNUSED(func_id);
    // CallCtx::pushFunction(func_id);
}

void _exit_func(uint64_t func_id) {
    UNUSED(func_id);
    // CallCtx::popFunction(func_id);
}

//-----------------------------------------------

typedef void* (*thread_routine_t)(void*);

struct new_thread_args_pack_t {
    thread_routine_t routine_;
    void *arg_;
    std::atomic<tid_t> tid_;
};

void *new_thread_start_stub(void *arg) {
    tid_t my_tid = Tid::getNewTid();
    new_thread_args_pack_t *pack = (new_thread_args_pack_t *)arg;
    pack->tid_.store(my_tid, std::memory_order_seq_cst);

    // Console::info("Thread Start Stub\n");

    DO_INSTRUMENT(exe->threadStart());
    void *ret = pack->routine_(pack->arg_);

    DO_INSTRUMENT(exe->threadExit());
    Tid::unregisterCurrThread();

    return ret;
}

int pthread_create_wrapper(uint64_t inst_id, pthread_t *thread, pthread_attr_t *attr,
                           thread_routine_t routine, void *arg) {
    //
    // Until creating the first thread, we start instrumentation.
    //
    Control::setStarted();

    static bool dump_create = Config::getRuntimeConfig()->getBool("DUMP_CREATE");
    if (dump_create) {
        Debug::dumpThreadStacktrace(0);
    }
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadCreate(inst_id, thread));
    // =============== NOTICE ==========
    // Yoooo, we never delete this pack.
    // Let it leak.
    // =================================
    new_thread_args_pack_t *pack =
        (new_thread_args_pack_t *)malloc(sizeof(new_thread_args_pack_t));

    pack->routine_ = routine;
    pack->arg_ = arg;
    pack->tid_ = UNDEFINED_TID;

    int ret = pthread_create(thread, attr, new_thread_start_stub, pack);

    // Spawn thread will register itself and write pack->tid_
    while (pack->tid_.load(std::memory_order_seq_cst) == UNDEFINED_TID) {
        pthread_yield();
    }

    DO_INSTRUMENT(exe->afterPthreadCreate(inst_id, pack->tid_));
    return ret;
}

void  pthread_exit_wrapper(uint64_t inst_id, void *arg) {
    UNUSED(inst_id);
    DO_INSTRUMENT(exe->threadExit());
    pthread_exit(arg);
}

//-----------------------------------------------

int pthread_join_wrapper(uint64_t inst_id, pthread_t t, void **arg) {
    Control::incGlobalTick();

    tid_t remote_tid = Tid::lookupTid(t);
    DO_INSTRUMENT(exe->beforePthreadJoin(inst_id, remote_tid));

    int ret = pthread_join(t, arg);

    DO_INSTRUMENT(exe->afterPthreadJoin(inst_id, remote_tid));
    return ret;
}

//-----------------------------------------------
void* malloc_wrapper(uint64_t inst_id, size_t size) {
    UNUSED(inst_id);
    static bool log_mem_alloc = Logger::logMemAlloc();

    void *ret = malloc(size);

    if (log_mem_alloc) {
        CallCtx::pushCaller(inst_id);
        Logger::logMemAllocEvent(ret, size, CallCtx::getCallStack());
        CallCtx::popCaller(inst_id);
    }
    return ret;
}

void* calloc_wrapper(uint64_t inst_id, size_t n, size_t size) {
    UNUSED(inst_id);
    static bool log_mem_alloc = Logger::logMemAlloc();

    void *ret = calloc(n, size);

    if (log_mem_alloc) {
        CallCtx::pushCaller(inst_id);
        Logger::logMemAllocEvent(ret, n * size, CallCtx::getCallStack());
        CallCtx::popCaller(inst_id);
    }

    return ret;
}

void* realloc_wrapper(uint64_t inst_id, void *ptr, size_t new_size) {
    UNUSED(inst_id);
    static bool log_mem_alloc = Logger::logMemAlloc();

    void *ret = realloc(ptr, new_size);

    if (log_mem_alloc) {
        CallCtx::pushCaller(inst_id);
        Logger::logMemAllocEvent(ret, new_size, CallCtx::getCallStack());
        CallCtx::popCaller(inst_id);
    }

    return ret;
}

void free_wrapper(uint64_t inst_id, void *ptr) {
    static bool free_mem = Config::getRuntimeConfig()->getBool("FREE_MEM");
    if (free_mem) {
        DO_INSTRUMENT(exe->beforeFree(inst_id, ptr));
        free(ptr);
        DO_INSTRUMENT(exe->afterFree(inst_id, ptr));
    }
}

void _after_fork(uint64_t inst_id, int pid) {
    DO_INSTRUMENT(exe->afterFork(inst_id, pid));
}

//-----------------------------------------------
int pthread_mutex_destroy_wrapper(uint64_t inst_id, pthread_mutex_t* m)  {
    DO_INSTRUMENT(exe->beforePthreadMutexLock(inst_id, m));
    int ret = pthread_mutex_destroy(m);
    DO_INSTRUMENT(exe->afterPthreadMutexLock(inst_id, m));
    return ret;
}

int pthread_mutex_lock_wrapper(uint64_t inst_id, pthread_mutex_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadMutexLock(inst_id, m));
    int ret = pthread_mutex_lock(m);
    DO_INSTRUMENT(exe->afterPthreadMutexLock(inst_id, m));
    return ret;
}

int pthread_mutex_unlock_wrapper(uint64_t inst_id, pthread_mutex_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadMutexUnlock(inst_id, m));
    int ret = pthread_mutex_unlock(m);
    DO_INSTRUMENT(exe->afterPthreadMutexUnlock(inst_id, m));
    return ret;
}

int pthread_mutex_trylock_wrapper(uint64_t inst_id, pthread_mutex_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadMutexTrylock(inst_id, m));
    int ret = pthread_mutex_trylock(m);
    DO_INSTRUMENT(exe->afterPthreadMutexTrylock(inst_id, m));
    return ret;
}

int pthread_cond_wait_wrapper(uint64_t inst_id, pthread_cond_t* c, pthread_mutex_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadCondWait(inst_id, c, m));
    int ret = pthread_cond_wait(c, m);
    DO_INSTRUMENT(exe->afterPthreadCondWait(inst_id, c, m));
    return ret;
}

int pthread_cond_timedwait_wrapper(uint64_t inst_id, pthread_cond_t* c, pthread_mutex_t* m, const struct timespec* t)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadCondTimedwait(inst_id, c, m, t));
    int ret = pthread_cond_timedwait(c, m, t);
    DO_INSTRUMENT(exe->afterPthreadCondTimedwait(inst_id, c, m, t));
    return ret;
}

int pthread_cond_signal_wrapper(uint64_t inst_id, pthread_cond_t* c)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadCondSignal(inst_id, c));
    int ret = pthread_cond_signal(c);
    DO_INSTRUMENT(exe->afterPthreadCondSignal(inst_id, c));
    return ret;
}

int pthread_cond_broadcast_wrapper(uint64_t inst_id, pthread_cond_t* c)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadCondBroadcast(inst_id, c));
    int ret = pthread_cond_broadcast(c);
    DO_INSTRUMENT(exe->afterPthreadCondBroadcast(inst_id, c));
    return ret;
}

int pthread_rwlock_rdlock_wrapper(uint64_t inst_id, pthread_rwlock_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadRwlockRdlock(inst_id, m));
    int ret = pthread_rwlock_rdlock(m);
    DO_INSTRUMENT(exe->afterPthreadRwlockRdlock(inst_id, m));
    return ret;
}

int pthread_rwlock_wrlock_wrapper(uint64_t inst_id, pthread_rwlock_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadRwlockWrlock(inst_id, m));
    int ret = pthread_rwlock_wrlock(m);
    DO_INSTRUMENT(exe->afterPthreadRwlockWrlock(inst_id, m));
    return ret;
}

int pthread_rwlock_tryrdlock_wrapper(uint64_t inst_id, pthread_rwlock_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadRwlockTryrdlock(inst_id, m));
    int ret = pthread_rwlock_tryrdlock(m);
    DO_INSTRUMENT(exe->afterPthreadRwlockTryrdlock(inst_id, m));
    return ret;
}

int pthread_rwlock_trywrlock_wrapper(uint64_t inst_id, pthread_rwlock_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadRwlockTrywrlock(inst_id, m));
    int ret = pthread_rwlock_trywrlock(m);
    DO_INSTRUMENT(exe->afterPthreadRwlockTrywrlock(inst_id, m));
    return ret;
}

int pthread_rwlock_unlock_wrapper(uint64_t inst_id, pthread_rwlock_t* m)  {
    Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforePthreadRwlockUnlock(inst_id, m));
    int ret = pthread_rwlock_unlock(m);
    DO_INSTRUMENT(exe->afterPthreadRwlockUnlock(inst_id, m));
    return ret;
}

void* memchr_wrapper(uint64_t inst_id, const void* str, int c, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeMemchr(inst_id, str, c, n));
    void* ret = memchr(str, c, n);
    DO_INSTRUMENT(exe->afterMemchr(inst_id, str, c, n));
    return ret;
}

int memcmp_wrapper(uint64_t inst_id, const void* str1, const void* str2, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeMemcmp(inst_id, str1, str2, n));
    int ret = memcmp(str1, str2, n);
    DO_INSTRUMENT(exe->afterMemcmp(inst_id, str1, str2, n));
    return ret;
}

void* memcpy_wrapper(uint64_t inst_id, void* dest, const void* src, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeMemcpy(inst_id, dest, src, n));
    void* ret = memcpy(dest, src, n);
    DO_INSTRUMENT(exe->afterMemcpy(inst_id, dest, src, n));
    return ret;
}

void* memmove_wrapper(uint64_t inst_id, void* dest, const void* src, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeMemmove(inst_id, dest, src, n));
    void* ret = memmove(dest, src, n);
    DO_INSTRUMENT(exe->afterMemmove(inst_id, dest, src, n));
    return ret;
}

void* memset_wrapper(uint64_t inst_id, void* str, int c, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeMemset(inst_id, str, c, n));
    void* ret = memset(str, c, n);
    DO_INSTRUMENT(exe->afterMemset(inst_id, str, c, n));
    return ret;
}

char* strcat_wrapper(uint64_t inst_id, char* dest, const char* src)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrcat(inst_id, dest, src));
    char* ret = strcat(dest, src);
    DO_INSTRUMENT(exe->afterStrcat(inst_id, dest, src));
    return ret;
}

char* strncat_wrapper(uint64_t inst_id, char* dest, const char* src, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrncat(inst_id, dest, src, n));
    char* ret = strncat(dest, src, n);
    DO_INSTRUMENT(exe->afterStrncat(inst_id, dest, src, n));
    return ret;
}

char* strchr_wrapper(uint64_t inst_id, const char* str, int c)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrchr(inst_id, str, c));
    char* ret = strchr(str, c);
    DO_INSTRUMENT(exe->afterStrchr(inst_id, str, c));
    return ret;
}

int strcmp_wrapper(uint64_t inst_id, const char* str1, const char* str2)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrcmp(inst_id, str1, str2));
    int ret = strcmp(str1, str2);
    DO_INSTRUMENT(exe->afterStrcmp(inst_id, str1, str2));
    return ret;
}

int strncmp_wrapper(uint64_t inst_id, const char* str1, const char* str2, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrncmp(inst_id, str1, str2, n));
    int ret = strncmp(str1, str2, n);
    DO_INSTRUMENT(exe->afterStrncmp(inst_id, str1, str2, n));
    return ret;
}

int strcoll_wrapper(uint64_t inst_id, const char* str1, const char* str2)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrcoll(inst_id, str1, str2));
    int ret = strcoll(str1, str2);
    DO_INSTRUMENT(exe->afterStrcoll(inst_id, str1, str2));
    return ret;
}

char* strcpy_wrapper(uint64_t inst_id, char* dest, const char* src)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrcpy(inst_id, dest, src));
    char* ret = strcpy(dest, src);
    DO_INSTRUMENT(exe->afterStrcpy(inst_id, dest, src));
    return ret;
}

char* strncpy_wrapper(uint64_t inst_id, char* dest, const char* src, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrncpy(inst_id, dest, src, n));
    char* ret = strncpy(dest, src, n);
    DO_INSTRUMENT(exe->afterStrncpy(inst_id, dest, src, n));
    return ret;
}

size_t strcspn_wrapper(uint64_t inst_id, const char* str1, const char* str2)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrcspn(inst_id, str1, str2));
    size_t ret = strcspn(str1, str2);
    DO_INSTRUMENT(exe->afterStrcspn(inst_id, str1, str2));
    return ret;
}

char* strerror_wrapper(uint64_t inst_id, int errnum)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrerror(inst_id, errnum));
    char* ret = strerror(errnum);
    DO_INSTRUMENT(exe->afterStrerror(inst_id, errnum));
    return ret;
}

size_t strlen_wrapper(uint64_t inst_id, const char* str)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrlen(inst_id, str));
    size_t ret = strlen(str);
    DO_INSTRUMENT(exe->afterStrlen(inst_id, str));
    return ret;
}

char* strpbrk_wrapper(uint64_t inst_id, const char* str1, const char* str2)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrpbrk(inst_id, str1, str2));
    char* ret = strpbrk(str1, str2);
    DO_INSTRUMENT(exe->afterStrpbrk(inst_id, str1, str2));
    return ret;
}

char* strrchr_wrapper(uint64_t inst_id, const char* str, int c)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrrchr(inst_id, str, c));
    char* ret = strrchr(str, c);
    DO_INSTRUMENT(exe->afterStrrchr(inst_id, str, c));
    return ret;
}

size_t strspn_wrapper(uint64_t inst_id, const char* str1, const char* str2)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrspn(inst_id, str1, str2));
    size_t ret = strspn(str1, str2);
    DO_INSTRUMENT(exe->afterStrspn(inst_id, str1, str2));
    return ret;
}

char* strstr_wrapper(uint64_t inst_id, const char* haystack, const char* needle)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrstr(inst_id, haystack, needle));
    char* ret = strstr(haystack, needle);
    DO_INSTRUMENT(exe->afterStrstr(inst_id, haystack, needle));
    return ret;
}

char* strtok_wrapper(uint64_t inst_id, char* str, const char* delim)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrtok(inst_id, str, delim));
    char* ret = strtok(str, delim);
    DO_INSTRUMENT(exe->afterStrtok(inst_id, str, delim));
    return ret;
}

size_t strxfrm_wrapper(uint64_t inst_id, char* dest, const char* src, size_t n)  {
    // Control::incGlobalTick();
    DO_INSTRUMENT(exe->beforeStrxfrm(inst_id, dest, src, n));
    size_t ret = strxfrm(dest, src, n);
    DO_INSTRUMENT(exe->afterStrxfrm(inst_id, dest, src, n));
    return ret;
}


}
