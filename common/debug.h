#ifndef __COMMON_DEBUG_H__
#define __COMMON_DEBUG_H__

#include <pthread.h>
#include <signal.h>

namespace Debug {

int dumpThreadStacktrace(pthread_t threadId);

const char* stacktrace();

// void signalHandler(int signr, siginfo_t *info, void *secret);
// void setupSignalHandler();

}

#endif /* ifndef __DEBUG_H__ */
