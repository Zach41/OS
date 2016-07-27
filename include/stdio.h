#ifndef _ZACH_STDIO_H_
#define _ZACH_STDIO_H_

#define PUBLIC
#define PRIVATE    static

#define EXTERN     extern

/* ASSERT */
#define ASSERT
#ifdef  ASSERT
void assert_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp) if (exp) ; \
    else assert_failure(#exp, __FILE__, __BASE_FILE__, __LINE__);
#else
#define assert(exp)
#endif

#define MAX_PATH    128

#define STR_DEFAULT_LEN 1024
#define O_CREAT         1
#define O_RDWR          2
#define O_TRUNC         4

#define SEEK_SET        1
#define SEEK_CUR        2
#define SEEK_END        3

#define FD_STDOUT       1
#define FD_STDIN        0

typedef int off_t;
typedef char* va_list;

struct stat {
    int st_dev;
    int st_ino;
    int st_mode;
    int st_rdev;
    int st_size;
};

PUBLIC int open(const char* pathname, int flags);

PUBLIC int close(int fd);

PUBLIC int read(int fd, void* buf, int count);

PUBLIC int write(int fd, const void* buf, int count);

PUBLIC int unlink(const char* pathname);

PUBLIC int lseek(int fd, off_t offset, int whence);

PUBLIC int sprintf(char*buf, const char* fmt, ...);

PUBLIC int vsprintf(char* buf, const char* fmt, va_list args);

PUBLIC int printf(const char* fmt, ...);

PUBLIC int fork();

PUBLIC int getpid();

PUBLIC int getppid();

PUBLIC int wait(int* status);

PUBLIC void exit(int status);

PUBLIC int  stat(const char* pathname, struct stat* s);

PUBLIC int execl(char* path, char* arg, ...);
PUBLIC int execv(char* path, char* argv[]);

#endif
