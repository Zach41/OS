#ifndef _ZACH_STDIO_H_
#define _ZACH_STDIO_H_

#define MAX_PATH    128

#define STR_DEFAULT_LEN 1024
#define O_CREAT         1
#define O_RDWR          2

#define SEEK_SET        1
#define SEEK_CUR        2
#define SEEK_END        3

#define FD_STDOUT       1
#define FD_STDIN        0

typedef int off_t;

PUBLIC int open(const char* pathname, int flags);

PUBLIC int close(int fd);

PUBLIC int read(int fd, void* buf, int count);

PUBLIC int write(int fd, const void* buf, int count);

PUBLIC int unlink(const char* pathname);

PUBLIC int lseek(int fd, off_t offset, int whence);

PUBLIC int printf(const char* fmt, ...);

#endif
