#ifndef __COMMON_CONSOLE_H__
#define __COMMON_CONSOLE_H__

namespace Console {

//
// Macro can't be protected in namespace
// And macro is tricky, so use it as less as possible :-P
//
void info(const char *fmt, ...);
void err(const char *fmt, ...);

void blueInfo(const char *fmt, ...);
void cyanInfo(const char *fmt, ...);

void redErr(const char *fmt, ...);

}

#endif /* ifndef __COMMON_CONSOLE_H__

 */
