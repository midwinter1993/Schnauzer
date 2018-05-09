#include "Utils.h"
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <string>
#include "Control.h"
#include "Tid.h"
#include "common/auxiliary.h"
#include "common/debug.h"
#include "common/Console.h"

namespace Utils {

bool checkCore(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores) {
        assert("core_id error\n" && false);
        return false;
    }
    return true;
}

void stickAllThreadToCore(int core_id) {
    checkCore(core_id);

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset)) {
        Console::info("Set Affinity error\n");
    }
}

int getThreadStickedCore() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset)) {
        Console::info("Get Affinity error\n");
    }

    int core_id = -1;
    for (int i = 0; i < CPU_SETSIZE; ++i) {
        if (CPU_ISSET(i, &cpuset)) {
            fprintf(stderr, "%d\n", i);
            core_id = i;
        }
    }
    return core_id;
}

void stickCurrThreadToCore(int core_id) {
    checkCore(core_id);

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset)) {
        Console::info("Set thread affinity error\n");
    }
}

static uintptr_t getRsp() {
    uintptr_t addr;
    asm volatile("mov %%rsp, %0" : "=r"(addr));
    return addr;
}

bool isOnStack(const void *addr) {
    return reinterpret_cast<uintptr_t>(addr) > getRsp();
}

// sleep for t us
void safeSleep(int64_t t) {
    usleep(t);
    // size_t cnt = 0;
    // const size_t MAX_TIME = t * 10;
    // while (cnt < MAX_TIME) {
        // pthread_yield();
        // cnt += 1;
    // }
}

void segfaultHandler(int) {
    // Stop all the threads.
    Control::setFinished();
    // Dump thread stacktrace.
    Tid::dumpAllThreads();
    // thread_stacktrace(0);
    exit(1);
}

}
