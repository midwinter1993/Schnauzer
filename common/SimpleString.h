#ifndef __COMMON_SIMPLE_STRING_H__
#define __COMMON_SIMPLE_STRING_H__

#include <cstddef>

class SimpleString {
public:
    SimpleString(size_t sz);

    // void append(const char *s);
    void append(const char *fmt, ...);
    void clear();
    const char* str() const;

private:
    size_t len_;
    size_t capacity_;
    char *buf_;
};

#endif /* ifndef __COMMON_SIMPLE_STRING_H__ */
