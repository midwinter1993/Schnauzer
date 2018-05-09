#ifndef __RUNTIME_SCHED_MONEY_MNGR_H__
#define __RUNTIME_SCHED_MONEY_MNGR_H__

#include <vector>

#include "runtime/core/CacheLineAtomic.h"
#include "runtime/core/Tid.h"

namespace MoneyMngr {

typedef CacheLineAtomic::value_t money_t;

void currThreadEnterSync();
void currThreadLeaveSync();

void decCurrThreadMoney(money_t m = 1);
void clearCurrThreadMoney();
money_t getCurrThreadMoney();

// money_t getThreadMoney(tid_t t);
// void setThreadMoney(tid_t t, money_t m);

void init();
bool needResetMoney();
void resetThreadMoney();

// void setInitThreadMoney(const std::vector<money_t> &moneys);
void randomInitThreadMoney();

}

#endif /* ifndef __RUNTIME_SCHED_MONEY_MNGR_H__ */
