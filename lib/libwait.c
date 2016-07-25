#include "headers.h"
#include "stdio.h"

PUBLIC int wait(int* status) {
    MESSAGE msg;
    msg.type = WAIT;

    send_recv(BOTH, TASK_MM, &msg);

    *status = msg.STATUS;

    if (msg.PID == NO_TASK)
	return -1;
    else
	return msg.PID;
}
