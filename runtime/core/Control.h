#ifndef __RUNTIME_CORE_COMTROL_H__
#define __RUNTIME_CORE_COMTROL_H__

#include "Lock.h"
#include "common/basic.h"


namespace Control {

bool isStarted();
void setStarted();
bool isFinished();
void setFinished();

void globalLock();
void globalUnlock();


timestamp_t getGlobalTick();
timestamp_t incGlobalTick();

}
#endif
