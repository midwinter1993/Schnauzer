/*
 *  Debug_GetCallstack.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 06.04.12.
 *  code under LGPL
 *
 */

/*
 About the POSIX solution:

 Initially, I wanted to implement something similar as suggested
 here <http://stackoverflow.com/a/4778874/133374>, i.e. getting
 somehow the top frame pointer of the thread and unwinding it
 manually (the linked source is derived from Apples `backtrace`
 implementation, thus might be Apple-specific, but the idea is
 generic).

 However, to have that safe (and the source above is not and
 may even be broken anyway), you must suspend the thread while
 you access its stack. I searched around for different ways to
 suspend a thread and found:
  - http://stackoverflow.com/questions/2208833/how-do-i-suspend-another-thread-not-the-current-one
  - http://stackoverflow.com/questions/6367308/sigstop-and-sigcont-equivalent-in-threads
  - http://stackoverflow.com/questions/2666059/nptl-sigcont-and-thread-scheduling
 Basically, there is no really good way. The common hack, also
 used by the Hotspot JAVA VM (<http://stackoverflow.com/a/2221906/133374>),
 is to use signals and sending a custom signal to your thread via
 `pthread_kill` (<http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_kill.html>).

 So, as I would need such signal-hack anyway, I can have it a bit
 simpler and just use `backtrace` inside the called signal handler
 which is executed in the target thread (as also suggested here:
 <http://stackoverflow.com/a/6407683/133374>). This is basically
 what this implementation is doing.

 If you are also interested in printing the backtrace, see:
 - backtrace_symbols_str() in Debug_extended_backtrace.cpp
 - DumpCallstack() in Debug_DumpCallstack.cpp
 */




// When implementing iterating over threads on Mac, this might be useful:
// http://llvm.org/viewvc/llvm-project/lldb/trunk/tools/darwin-threads/examine-threads.c?view=markup

// For getting the callback, maybe libunwind can be useful: http://www.nongnu.org/libunwind/

#include "debug.h"

#include <string>
#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "auxiliary.h"
#include "basic.h"
#include "Console.h"
#include "SimpleString.h"

namespace Debug {

#define CALLSTACK_SIG SIGUSR2

static pthread_t caller_tid_ = 0;
static pthread_t target_tid_ = 0;


static void signalHandler(int sig, siginfo_t *info, void *secret) {
    UNUSED(sig);
    UNUSED(info);
    UNUSED(secret);

	pthread_t curr_tid = pthread_self();
	if(curr_tid != target_tid_) { return; }

	stacktrace();

	// continue calling thread
	pthread_kill(caller_tid_, CALLSTACK_SIG);
}

static void setupSignalHandler() {
	struct sigaction sa;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signalHandler;
	sigaction(CALLSTACK_SIG, &sa, NULL);
}

int dumpThreadStacktrace(pthread_t tid) {
	if(tid == 0 || tid == pthread_self()) {
        Console::info("%s", stacktrace());
        return 0;
    }

	caller_tid_ = pthread_self();
	target_tid_ = tid;

	setupSignalHandler();

	// call _callstack_signal_handler in target thread
	if(pthread_kill(tid, CALLSTACK_SIG) != 0) {
		// something failed ...
		return 0;
    }

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, CALLSTACK_SIG);

    // wait for CALLSTACK_SIG on this thread
    sigsuspend(&mask);

	return 0;
}

static __thread SimpleString *__stacktrace_buf = nullptr;

#define stacktraceOut(...) __stacktrace_buf->append(__VA_ARGS__)

#define stacktraceClear() __stacktrace_buf->clear()

#define stacktraceStr() __stacktrace_buf->str()

// #define stacktrace stacktraceImpl2
static const char* stacktraceImpl1();
static const char* stacktraceImpl2();

const char* stacktrace() {
    if (!__stacktrace_buf) {
        __stacktrace_buf = new SimpleString(_1K_ *2);
    }
    return stacktraceImpl2();
}


//
// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0
//
static const char* stacktraceImpl1() {
    stacktraceClear();
    stacktraceOut("stack trace:\n");

    // storage array for stack trace address data
    const unsigned int max_frames = 64;
    void **addr_list = (void **)malloc(sizeof(void *) * (max_frames + 1));

    // retrieve current stack addresses
    int addr_len = backtrace(addr_list, sizeof(addr_list) / sizeof(void *));

    if (addr_len == 0) {
        stacktraceOut("  <empty, possibly corrupt>\n");
        return stacktraceStr();
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char **sym_list = backtrace_symbols(addr_list, addr_len);

    // allocate string which will be filled with the demangled function name
    size_t func_name_sz = 256;
    char *func_name = (char *)malloc(func_name_sz);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addr_len; i++) {
        char *begin_module = 0, *begin_name = 0, *begin_offset = 0,
             *begin_addr = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = sym_list[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                begin_addr = p;
                break;
            }
        }

        if (begin_name && begin_offset && begin_addr &&
            begin_name < begin_offset) {
            begin_module = sym_list[i];
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *begin_addr++ = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, begin_addr). now apply
            // __cxa_demangle():

            int status = 0;
            char *ret = abi::__cxa_demangle(begin_name, func_name,
                                            &func_name_sz, &status);
            if (status == 0) {
                func_name = ret;  // use possibly realloc()-ed string
                stacktraceOut("  %s : %s+%s %s\n", sym_list[i], func_name,
                              begin_offset, begin_addr);
            } else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                stacktraceOut("  %s : %s()+%s %s\n", sym_list[i], begin_name,
                              begin_offset, begin_addr);
            }
        } else {
            // couldn't parse the line? print the whole line.
            Console::err("  %s\n", sym_list[i]);
        }
    }

    free(func_name);
    free(sym_list);
    free(addr_list);
    return stacktraceStr();
}

//
// http://eli.thegreenplace.net/2015/programmatic-access-to-the-call-stack-in-c/
//
static const char* stacktraceImpl2() {
    stacktraceClear();

    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        stacktraceOut("0x%lx:", pc);

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
            char *func_name = sym;
            int status = 0;
            char *demangled =
                abi::__cxa_demangle(sym, nullptr, nullptr, &status);
            if (status == 0) {
                func_name = demangled;
            }
            stacktraceOut(" (%s+0x%lx)\n", func_name, offset);
            free(demangled);
        } else {
            stacktraceOut(" -- error: unable to obtain symbol name for this frame\n");
        }
    }
    return stacktraceStr();
}

}
