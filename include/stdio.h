#ifndef _ZACH_STDIO_H_
#define _ZACH_STDIO_H_

#define MAX_PATH    128

#define STR_DEFAULT_LEN 1024
#define O_CREAT         1
#define O_RDWR          2

#define SEEK_SET        1
#define SEEK_CUR        2
#define SEEK_END        3

PUBLIC int open(const char* pathname, int flags);

PUBLIC int close(int fd);

PUBLIC int readf(int fd, void* buf, int count);

PUBLIC int writef(int fd, const void* buf, int count);

#endif
