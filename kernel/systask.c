#include "headers.h"

/* 系统进程 */
PUBLIC void task_sys() {
    MESSAGE msg;

    while(1) {
	send_recv(RECEIVE, ANY, &msg);
	
	int src = msg.source;

	switch(msg.type) {
	case GET_TICKS:
	    msg.RETVAL = ticks;
	    msg.type   = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	case GET_PID:
	    msg.PID    = src;
	    msg.type   = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	case GET_PPID:
	    msg.PID    = proc_table[src].p_parent;
	    msg.type   = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	default:
	    panic("TASK_SYS::unknow msg type %d\n", msg.type);
	    break;
	}
    }
}
