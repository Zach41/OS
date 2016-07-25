#include "headers.h"
#include "stdio.h"

PUBLIC int read(int fd, void* buf, int count) {
    MESSAGE msg;

    msg.type = READ;
    msg.CNT  = count;
    msg.FD   = fd;
    msg.BUF  = buf;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
