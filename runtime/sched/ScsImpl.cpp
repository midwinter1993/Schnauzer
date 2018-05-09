#include "ScsImpl.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <atomic>
#include <vector>
#include "MoneyMngr.h"
#include "common/common.h"
#include "runtime/core/CacheLineAtomic.h"
#include "runtime/core/Tid.h"
#include "runtime/core/Utils.h"
#include "runtime/core/Control.h"
#include "Supervisor.h"

#define USE_LOGIC_CLOCK
// #define USE_REAL_CLOCK

namespace ScsImpl {

struct SchedParam {
    bool sync_only_;
    bool random_money_;
    int reset_interval_;

    SchedParam()
        : sync_only_(true)
        , random_money_(false)
        , reset_interval_(0) { }
};

static SchedParam __sched_param;
static pthread_cond_t __cond_var;
static pthread_mutex_t __mutex;
static std::atomic_llong __epoch(0);

static void initSchedParam() {
    const Subconfig *subconfig    = Config::getScsConfig();
    __sched_param.sync_only_      = subconfig->getBool("SYNC_ONLY");
    __sched_param.random_money_   = subconfig->getBool("RANDOM_MONEY");
    __sched_param.reset_interval_ = subconfig->getNumber("RESET_INTERVAL", true);
    if (__sched_param.reset_interval_ == 0) {
        __sched_param.reset_interval_ = std::numeric_limits<int>::max();
    }
}

static void supervise();

void schedInit() {
    srand(time(NULL));

    pthread_mutex_init(&__mutex, NULL);
    pthread_cond_init(&__cond_var, NULL);

    initSchedParam();
    MoneyMngr::init();
    Supervisor::start(supervise);
}

void schedCleanup() {
    Supervisor::stop();
    //
    // __mutex may be locked by other threads
    //
    // pthread_mutex_destroy(&__mutex);
    // pthread_cond_destroy(&__cond_var);
}


#ifdef USE_LOGIC_CLOCK

static void startNewEpoch() {
    MoneyMngr::resetThreadMoney();
    __epoch.fetch_add(1, std::memory_order_seq_cst);
    pthread_cond_broadcast(&__cond_var);
}

static void checkMoney() {
    if (MoneyMngr::needResetMoney()) {
        startNewEpoch();
    }
}

static void supervise() {
    static uint64_t epoch = 1;

    pthread_mutex_lock(&__mutex);
    startNewEpoch();

    epoch += 1;

    if (__sched_param.random_money_ && epoch % __sched_param.reset_interval_ == 0) {
        MoneyMngr::randomInitThreadMoney();
    }
    pthread_mutex_unlock(&__mutex);
}

static void blockCurrThread() {
    assert(MoneyMngr::getCurrThreadMoney() == 0 && "Blocked thread has no money");

    // Console::info("Block %d\n", Tid::getCurrTid());
    auto last_epoch = __epoch.load(std::memory_order_seq_cst);
    do {
        pthread_cond_wait(&__cond_var, &__mutex);
    } while (last_epoch == __epoch.load(std::memory_order_seq_cst));
}

void beforeSyncOperation() {
    usleep(1000);
    pthread_mutex_lock(&__mutex);
    // checkMoney();

    if (MoneyMngr::getCurrThreadMoney() == 0) {
        blockCurrThread();
    }

    // assert(is_pos_even(get_curr_thread_money()));
    MoneyMngr::decCurrThreadMoney();
    // MoneyMngr::decCurrThreadMoney(10);
    MoneyMngr::currThreadEnterSync();
    pthread_mutex_unlock(&__mutex);
}

void afterSyncOperation() {
    pthread_mutex_lock(&__mutex);
    // checkMoney();

    MoneyMngr::currThreadLeaveSync();
    pthread_mutex_unlock(&__mutex);
}

void beforeFuncCall() {
    if (__sched_param.sync_only_) {
        return;
    }
    pthread_mutex_lock(&__mutex);
    // checkMoney();

    if (MoneyMngr::getCurrThreadMoney() == 0) {
        blockCurrThread();
    }

    MoneyMngr::decCurrThreadMoney();
    pthread_mutex_unlock(&__mutex);
}

void afterFuncCall() {
    if (__sched_param.sync_only_) {
        return;
    }
    pthread_mutex_lock(&__mutex);
    // checkMoney();

    if (MoneyMngr::getCurrThreadMoney() == 0) {
        blockCurrThread();
    }

    MoneyMngr::decCurrThreadMoney();
    pthread_mutex_unlock(&__mutex);
}

#endif // #define USE_LOGIC_CLOCK

#ifdef USE_REAL_CLOCK
static void supervise() {
    static int cnt = 0;
    cnt += 1;

    pthread_mutex_lock(&__mutex);
    __epoch.fetch_add(1, std::memory_order_seq_cst);

    MoneyMngr::resetThreadMoney();

    if (__sched_param.random_money_ &&
        cnt % __sched_param.reset_interval_ == 0) {
        MoneyMngr::randomInitThreadMoney();
    }

    __epoch.fetch_add(1, std::memory_order_seq_cst);
    pthread_cond_broadcast(&__cond_var);
    pthread_mutex_unlock(&__mutex);
}

static void blockCurrThread() {
    pthread_mutex_lock(&__mutex);
    // Console::info("Block %d\n", Tid::getCurrTid());
    while (__epoch.load(std::memory_order_seq_cst) % 2 == 1) {
        pthread_cond_wait(&__cond_var, &__mutex);
    }
    pthread_mutex_unlock(&__mutex);
}

void beforeSyncOperation() {
    if (__epoch % 2 == 1 || MoneyMngr::getCurrThreadMoney() == 0) {
        blockCurrThread();
    }
    MoneyMngr::decCurrThreadMoney();
}

void afterSyncOperation() {
}

void beforeFuncCall() {
    if (__epoch % 2 == 1 || MoneyMngr::getCurrThreadMoney() == 0) {
        blockCurrThread();
    }
    MoneyMngr::decCurrThreadMoney();
}

void afterFuncCall() {
}
#endif // #define USE_REAL_CLOCK

}
