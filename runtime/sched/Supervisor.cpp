#include "Supervisor.h"

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <atomic>

#include "runtime/core/Tid.h"
#include "runtime/core/Control.h"
#include "runtime/core/Utils.h"
#include "common/common.h"

namespace Supervisor {

static std::atomic_bool __running(true);
static SuperviseFunc __supervise_func = nullptr;
static int __sleep_time = 0;
static pthread_t __supervisor_thd = 0;

static void* superviseLoop(void*) {
    assert(__supervise_func && __sleep_time);
    static volatile timestamp_t last_tick = 0;

    Tid::setMetaThread();

    // Ignore all signals for this thread
    sigset_t sig_set;
    sigfillset(&sig_set);
    if (pthread_sigmask(SIG_SETMASK, &sig_set, NULL) != 0) {
        Console::info("pthread_sigmask error\n");
    }

    Console::info("Supervisor start...\n");
    while (__running) {
        timestamp_t curr_tick = Control::getGlobalTick();

        if (last_tick == curr_tick) {
            __supervise_func();
        }

        last_tick = curr_tick;
        // sleep_time ms
        Utils::safeSleep(__sleep_time * 100);
    }
    Console::info("Supervisor end...\n");
    return NULL;
}

void start(SuperviseFunc f) {
    __supervise_func = f;
    __sleep_time = Config::getSupervisorConfig()->getNumber("SLEEP_TIME", true);
    if (pthread_create(&__supervisor_thd, NULL, superviseLoop, NULL)) {
        Console::err("Create supervisor error\n");
        exit(1);
    }
}

void stop() {
    Console::info("Stop supervisor\n");
    __running = false;
    if (__supervisor_thd != 0) {
        pthread_join(__supervisor_thd, NULL);
    }
}

}
