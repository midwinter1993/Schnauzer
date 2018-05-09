#include "SimpleString.h"
#include "auxiliary.h"

#include <cassert>
#include <cstring>
#include <stdio.h>
#include <stdarg.h>

SimpleString::SimpleString(size_t sz)
    : len_(0)
    , capacity_(sz) {
    buf_ = new char[capacity_];
    std::memset(buf_, 0, capacity_);
}

void SimpleString::append(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    auto l = vsnprintf(buf_ + len_, capacity_ - len_, fmt, args);
    va_end(args);

    len_ += l;
    assert(0 <= l && len_ < capacity_);
    buf_[len_] = '\0';
}


void SimpleString::clear() {
    len_ = 0;
    std::memset(buf_, 0, capacity_);
}

const char* SimpleString::str() const {
    return buf_;
}
