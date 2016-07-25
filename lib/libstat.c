#include "headers.h"
#include "stdio.h"

PUBLIC int stat(const char* pathname, struct stat* s) {
    MESSAGE msg;
    
    msg.type = STAT;
    msg.BUF  = (void*)s;
    msg.PATHNAME = (void*)pathname;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL; 
}
