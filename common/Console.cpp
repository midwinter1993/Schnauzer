#include "Console.h"
#include <cstdarg>
#include <cstdio>

namespace Console {

enum Color {
    WHITE     = 0,
    BLACK     = 30,
    RED       = 31,
    GREEN     = 32,
    BROWN     = 33,
    BLUE      = 34,
    MAGENTA   = 35,
    CYAN      = 36,
    LIGHTGRAY = 37
};

enum IoLevel {
    IO_DBG = 0,
    IO_INFO = 1,
    IO_NOTHING = 2
};

#define OUT_FD  stderr

#define ENABLE_IO

#define IO_LEVEL IO_DBG


void out(IoLevel level, const char *fmt, va_list args) {
#ifdef ENABLE_IO
    if (level >= IO_LEVEL) {
        vfprintf(OUT_FD, fmt, args);
    }
#endif
}

void err(const char *fmt, ...) {
#ifdef ENABLE_IO
    va_list args;
    va_start(args, fmt);
    out(IO_DBG, fmt, args);
    va_end(args);
#endif
}

void info(const char *fmt, ...) {
#ifdef ENABLE_IO
    va_list args;
    va_start(args, fmt);
    out(IO_INFO, fmt, args);
    va_end(args);
#endif
}

void resetColor() {
    fprintf(OUT_FD, "\033[0m");
}

void startColor(Color c) {
    if (c == WHITE) {
        fprintf(OUT_FD, "\033[0m");
    } else {
        fprintf(OUT_FD, "\033[0;%dm", c);
    }
}

void colorOut(Color c, IoLevel level, const char* fmt, va_list args) {
#ifdef ENABLE_IO
    if (level >= IO_LEVEL) {
        startColor(c);
        vfprintf(OUT_FD, fmt, args);
        resetColor();
    }
#endif
}

void blueInfo(const char* fmt, ...) {
#ifdef ENABLE_IO
    va_list args;
    va_start(args, fmt);
    colorOut(Color::BLUE, IO_INFO, fmt, args);
    va_end(args);
#endif
}

void cyanInfo(const char* fmt, ...) {
#ifdef ENABLE_IO
    va_list args;
    va_start(args, fmt);
    colorOut(Color::CYAN, IO_INFO, fmt, args);
    va_end(args);
#endif
}

void redErr(const char* fmt, ...) {
#ifdef ENABLE_IO
    va_list args;
    va_start(args, fmt);
    colorOut(Color::RED, IO_DBG, fmt, args);
    va_end(args);
#endif
}

}
