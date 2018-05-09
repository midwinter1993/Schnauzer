#include "MoneyMngr.h"
#include "common/common.h"
#include <random>
#include <functional>
#include <algorithm>


namespace MoneyMngr {

const money_t MAX_MONEY = 10000000;
static std::vector<money_t> __init_thread_money;
static CacheLineAtomic __thread_money[MAX_NR_THREAD]__attribute__((aligned(64)));
static __thread CacheLineAtomic* __curr_thread_money = nullptr;
static int __min_money = 0;
static int __max_money = 0;


void decCurrThreadMoney(money_t m) {
    if (!__curr_thread_money) {
        auto t = Tid::getCurrTid();
        __curr_thread_money = &__thread_money[t];
    }
    if (__curr_thread_money->get() <= m) {
        __curr_thread_money->set(0);
    } else {
        __curr_thread_money->dec();
    }
}

money_t getCurrThreadMoney() {
    if (!__curr_thread_money) {
        auto t = Tid::getCurrTid();
        __curr_thread_money = &__thread_money[t];
    }
    return __curr_thread_money->get();
}

void clearCurrThreadMoney() {
    if (!__curr_thread_money) {
        auto t = Tid::getCurrTid();
        __curr_thread_money = &__thread_money[t];
    }
    __curr_thread_money->set(0);
}

money_t getThreadMoney(tid_t t) {
    assert(t < Tid::getMaxNrThread());
    return __thread_money[t].get();
}

void setThreadMoney(tid_t t, money_t m) {
    assert(t < Tid::getMaxNrThread());
    __thread_money[t].set(m);
}


void currThreadEnterSync() {
    __curr_thread_money->set(getCurrThreadMoney() + MAX_MONEY);
}

void currThreadLeaveSync() {
    __curr_thread_money->set(getCurrThreadMoney() - MAX_MONEY);
}

bool isMoneyInSync(money_t x) {
    return x >= MAX_MONEY;
}

bool needResetMoney() {
    for (size_t i = 0; i < Tid::getNrThread(); ++i) {
        auto m = getThreadMoney(i);
        if (Tid::isThreadAlive(i) && !isMoneyInSync(m)) {
            return false;
        }
    }
    return true;
}

static money_t randMoney() {
    // return (rand() % 50 + 1) * 10;
    // return rand() % 15 + 1;

    // std::mt19937 rng;
    // rng.seed(std::random_device()());
    // std::uniform_int_distribution<std::mt19937::result_type> dist(1,50); // distribution in range [1, 6]
    // return dist(rng) * 10;

    // std::cout << dist6(rng) << std::endl;
    assert(__min_money > 0 &&
           __max_money > 0 &&
           __max_money / __min_money >= 10);

    // First create an instance of an engine.
    std::random_device rnd_device;

    // Specify the engine and distribution.
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_int_distribution<int> dist(__min_money, __max_money);

    auto gen = std::bind(dist, mersenne_engine);

    return gen();
}

static void printMoney(const std::string &prompt, const std::vector<money_t> &moneys) {
    Console::info("=== %s ===\n", prompt.c_str());
    for (auto m: moneys) {
        Console::info("%ld, ", m);
    }
    Console::info("\n===================\n");
}

static void setInitThreadMoney(std::vector<int64_t> &init_money) {
    assert(0 <= init_money.size() && init_money.size() <= Tid::getMaxNrThread());

    __init_thread_money.resize(Tid::getMaxNrThread());

    std::fill(std::copy(init_money.begin(), init_money.end(),
              __init_thread_money.begin()), __init_thread_money.end(),
              0);

    printMoney("Init money", __init_thread_money);
    for (size_t i = 0; i < Tid::getMaxNrThread(); ++i) {
        if (__init_thread_money[i] == 0) {
            __init_thread_money[i] = randMoney();
        }
    }

    assert(__init_thread_money.size() == Tid::getMaxNrThread());
    printMoney("Init money", __init_thread_money);
}


void randomInitThreadMoney() {
    __init_thread_money.resize(Tid::getMaxNrThread());
    std::generate(__init_thread_money.begin(),
                  __init_thread_money.end(),
                  randMoney);

    assert(__init_thread_money.size() == Tid::getMaxNrThread());

    printMoney("Reset init money", __init_thread_money);
}


void resetThreadMoney() {
    for (size_t i = 0; i < Tid::getMaxNrThread(); ++i) {
        setThreadMoney(i, __init_thread_money[i]);
    }
}

void init() {
    const Subconfig *subconfig = Config::getScsConfig();
    __min_money = subconfig->getNumber("RANDOM_MONEY_MIN", true);
    __max_money = subconfig->getNumber("RANDOM_MONEY_MAX", true);
    bool random_money = subconfig->getBool("RANDOM_MONEY");

    if (!random_money) {
        std::vector<int64_t> init_money = subconfig->getVector("INIT_MONEY");
        setInitThreadMoney(init_money);
    } else {
        randomInitThreadMoney();
    }
    resetThreadMoney();
}

}
