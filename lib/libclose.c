#include "headers.h"
#include "stdio.h"

PUBLIC int close(int fd) {
    MESSAGE msg;

    msg.type = CLOSE;
    msg.FD   = fd;
    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
