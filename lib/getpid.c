#include "headers.h"
#include "stdio.h"

PUBLIC int getpid() {
    MESSAGE msg;

    msg.type = GET_PID;
    send_recv(BOTH, TASK_SYS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.PID;
}

PUBLIC int getppid() {
    MESSAGE msg;

    msg.type = GET_PPID;
    printf("TYPE: %d\n", msg.type);
    send_recv(BOTH, TASK_SYS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.PID;
}
