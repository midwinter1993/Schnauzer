#ifndef __RUNTIME_SCHED_SUPERVISOR_H__
#define __RUNTIME_SCHED_SUPERVISOR_H__

namespace Supervisor {

typedef void (*SuperviseFunc)();

void start(SuperviseFunc f);

void stop();

}

#endif /* ifndef __RUNTIME_SCHED_SUPERVISOR_H__ */
