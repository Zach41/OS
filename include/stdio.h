#ifndef _ZACH_STDIO_H_
#define _ZACH_STDIO_H_

#define MAX_PATH    128

PUBLIC int open(const char* pathname, int flags);

PUBLIC int close(int fd);

#endif
