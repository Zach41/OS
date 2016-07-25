#include "headers.h"
#include "stdio.h"

PUBLIC int write(int fd, const void* buf, int count) {
    MESSAGE msg;

    msg.type = WRITE;
    msg.FD   = fd;
    msg.CNT  = count;
    msg.BUF  = (void*)buf;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
