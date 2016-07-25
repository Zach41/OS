#include "headers.h"
#include "stdio.h"

PUBLIC int unlink(const char* pathname) {
    MESSAGE msg;
    msg.PATHNAME = (void*)pathname;
    msg.NAME_LEN = strlen(pathname);
    msg.type     = UNLINK;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
