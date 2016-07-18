#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/* 系统进程 */
PUBLIC void task_sys() {
    MESSAGE msg;

    while(1) {
	send_recv(RECEIVE, ANY, &msg);
	/* printl("msg source: %d type: %d", msg.source, msg.type); */
	
	int src = msg.source;

	switch(msg.type) {
	case GET_TICKS:
	    msg.RETVAL = ticks;
	    send_recv(SEND, src, &msg);
	    break;
	default:
	    panic("unknow msg type");
	    break;
	}
    }
}
