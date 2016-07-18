#include "type.h"
#include "const.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PRIVATE void block(PROCESS* p);
PRIVATE void unblock(PROCESS* p);
PRIVATE int  deadlock(int src, int dest);
PRIVATE int  msg_send(PROCESS*, int, MESSAGE*);
PRIVATE int  msg_receive(PROCESS*, int src, MESSAGE*);

/* 系统调用的函数 */
PUBLIC int sys_get_ticks() {
    /* 返回当前的时钟数 */
    return ticks;
}


/* 进程调度函数 */
PUBLIC void schedule() {
    /* 优先级最大的优先执行 */
    PROCESS* p;
    int largest_ticks = 0;

    while (largest_ticks == 0) {
	for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS ; p++) {
	    if (p -> p_flags == 0) {
		if (p -> ticks > largest_ticks) {
		    largest_ticks = p -> ticks;
		    p_proc_ready  = p;
		}
	    }
	}
        if (largest_ticks == 0) {
       	/* 全部为0, 那么重新赋值 */
    	    for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
		if (p -> p_flags == 0)
		    p -> ticks = p -> priority;
    	    }
        }
    }
}

/* 返回一个段的线性基址 */
PUBLIC int ldt_seg_linear(PROCESS* p, int idx) {
    DESCRIPTOR* d = &p -> ldts[idx];

    return d -> base_high << 24 | d -> base_mid << 16 | d -> base_low;
    
}

/* virtual address to linear address */
PUBLIC void* va2la(int pid, void* va) {
    PROCESS* p = &proc_table[pid];

    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if (pid < NR_TASKS + NR_PROCS) {
	assert(la == (u32)va);
    }

    return (void*)la;
}


/* IPC 核心代码 */
PUBLIC void reset_msg(MESSAGE* p) {
    memset(p, 0, sizeof(MESSAGE));
}

PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, PROCESS *p) {
    assert(k_reenter == 0);	/* ring0 */
    assert(src_dest >= 0 && src_dest <= NR_TASKS + NR_PROCS || src_dest == ANY || src_dest == INTERRUPT);

    int ret = 0;
    
    int caller = proc2pid(p);
    MESSAGE *mla = (MESSAGE *)va2la(caller, m);

    mla -> source = caller;

    assert(mla -> source != src_dest);

    switch(function) {
    case SEND:
	ret = msg_send(p, src_dest, m);
	if (ret != 0)
	    return ret;
	break;
    case RECEIVE:
	ret = msg_receive(p, src_dest, m);
	if (ret != 0)
	    return ret;
	break;
    default:
	panic("invalid function &d (SEND: %d, RECEIVE: %d) in sys_sendrec.", function, SEND, RECEIVE);
    }

    return 0;
}

/* 阻塞一个进程，必须在p_flags被置１后调用 */
PRIVATE void block(PROCESS* p) {
    assert(p -> p_flags);

    schedule();
}

/* dummy routine, does nothing but checks if the p_flags was set to 0 */
PRIVATE void unblock(PROCESS* p) {
    assert(p -> p_flags == 0);
}

/* 检查死锁，若有死锁，返回1，否则返回0 */
PRIVATE int deadlock(int src, int dest) {
    PROCESS* p = proc_table + dest;

    while (TRUE) {
	if (p -> p_flags & SENDING) {
	    if (p -> p_sendto == src) {
		p = proc_table + dest;
		printl("=_=%s", p -> p_name);

		do {
		    assert(p -> p_msg);
		    p = proc_table + p -> p_sendto;
		    printl("->%s", p -> p_name);
		} while (p != proc_table + src);

		printl("=_=\n");

		return 1;
	    }

	    p = proc_table + p -> p_sendto;
	} else {
	    return 0;
	}
    }
}

/* assert 做调试用，其实可以不用加的 */

/* if the receiver is blocked, unblocking and copying the message to it, otherwise the caller will be blocked and appended to the receiver's sending queue.
 */
PRIVATE int msg_send(PROCESS* current, int dest, MESSAGE* m) {
    PROCESS *p_dest   = proc_table + dest;
    PROCESS *p_sender = current;

    /* 不能发送给自己 */
    assert(proc2pid(p_sender) != dest);

    if (deadlock(proc2pid(p_sender), dest)) {
	panic(">>DEADLOCK<< %s -> %s", p_sender -> p_name, p_dest -> p_name);
    }

    if((p_dest -> p_flags & RECEIVING) && (p_dest -> p_recvfrom == ANY || p_dest -> p_recvfrom == proc2pid(p_sender))) {
	/* 如果dest正在接收消息 */
	
	/* 保存必须有消息，而且接收者的p_msg不能为空指针，即必须有地方存放消息 */
	assert(m);
	assert(p_dest -> p_msg);

	memcpy(va2la(dest, p_dest -> p_msg),
	       va2la(proc2pid(p_sender), m),
	       sizeof(MESSAGE));
	p_dest -> p_msg = 0;
	p_dest -> p_recvfrom = NO_TASK;
	p_dest -> p_flags &= ~RECEIVING; /* dest已经收到消息 */

	/* 之前设置过了p_flags */
	unblock(p_dest);

	/* assert(p_dest -> p_flags == 0); */
	/* assert(p_dest -> p_recvfrom == NO_TASK); */
	/* assert(p_dest -> p_msg == 0); */
	/* assert(p_dest -> p_sendto == NO_TASK); */
	/* assert(p_sender -> p_flags == 0); */
	/* assert(p_sender -> p_msg == 0); */
	/* assert(p_sender -> p_recvfrom == NO_TASK); */
	/* assert(p_sender -> p_sendto == NO_TASK); */
    } else {
	p_sender -> p_flags |= SENDING;
	p_sender -> p_sendto = dest;
	p_sender -> p_msg    = m;
	assert(p_sender -> p_flags == SENDING);

	PROCESS *p;

	if (p_dest -> q_sending) {
	    p = p_dest -> next_sending;
	    while (p -> next_sending)
		p = p -> next_sending;
	    p -> next_sending = p_sender;
	} else {
	    p_dest -> q_sending = p_sender;
	}
	p_sender -> next_sending = 0;

	block(p_sender);

	/* assert(p_sender -> p_flags == SENDING); */
	/* assert(p_sender -> p_msg != 0); */
	/* assert(p_sender -> p_recvfrom = NO_TASK); */
	/* assert(p_sender -> p_sendto == dest); */
    }

    return 0;
}

PRIVATE int msg_receive(PROCESS *current, int src, MESSAGE* m) {
    PROCESS *p_recv   = current;
    PROCESS *p_from   = 0;
    PROCESS *prev     = 0;
    
    assert(proc2pid(p_recv) != src);

    int copyok = 0;

    if (p_recv -> has_int_msg && (src == ANY || src == INTERRUPT)) {
	/* 如果有一个中断需要处理 */
	MESSAGE msg;

	reset_msg(&msg);
	msg.source = INTERRUPT;
	msg.type   = HARD_INT;
	assert(m);
	memcpy(va2la(proc2pid(p_recv), m), &msg, sizeof(MESSAGE));

	p_recv -> has_int_msg = 0;

	/* assert(p_recv -> p_flags == 0); */
	/* assert(p_recv -> p_msg == 0); */
	/* assert(p_recv -> p_sendto == NO_TASK); */
	/* assert(p_recv -> has_int_msg == 0); */

	return 0;
    }

    if (src == ANY) {
	/* 如果接收任意源 */
	if (p_recv -> q_sending) {
	    p_from = p_recv -> q_sending;
	    copyok = 1;

	    /* assert(p_recv -> p_flags == 0); */
	    /* assert(p_recv -> p_msg == 0); */
	    /* assert(p_recv -> p_recvfrom == NO_TASK); */
	    /* assert(p_recv -> p_sendto == NO_TASK); */
	    /* assert(p_recv -> q_sending != 0); */
	    /* assert(p_from -> p_msg != 0); */
	    /* assert(p_from -> p_recvfrom = NO_TASK); */
	    /* assert(p_from -> p_sendto == proc2pid(p_recv)); */
	}
    } else  if (src >= 0 && src < NR_TASKS + NR_PROCS) {
	/* 接受特定源 */
	p_from = &proc_table[src];

	if ((p_from -> p_flags & SENDING) && (p_from -> p_sendto == proc2pid(p_recv))) {
	    copyok = 1;
	    PROCESS* p = p_recv -> q_sending;

	    while(p) {
		if (proc2pid(p_from) == src)
		    break;
		prev = p;
		p = p -> next_sending;
	    }
	}
    }

    if (copyok) {
	if (p_from == p_recv -> q_sending) {
	    p_recv -> q_sending = p_from -> next_sending;
	    p_from -> next_sending = 0;
	} else {
	    prev -> next_sending = p_from -> next_sending;
	    p_from -> next_sending = 0;
	}

	memcpy(va2la(proc2pid(p_recv), m),
	       va2la(proc2pid(p_from), p_from -> p_msg),
	       sizeof(MESSAGE));

	p_from -> p_flags &= ~SENDING;
	p_from -> p_sendto = NO_TASK;
	p_from -> p_msg    = 0;

	unblock(p_from);
    } else {
	/* 没有发送者 */
	p_recv -> p_flags   |= RECEIVING;
	p_recv -> p_recvfrom = src;
	p_recv -> p_msg      = m;

	block(p_recv);
    }

    return 0;
}


PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg) {
    int ret = 0;
    
    if (function == RECEIVE) {
	reset_msg(msg);
    }

    switch(function) {
    case BOTH:
	ret = sendrec(SEND, src_dest, msg);
	if (ret == 0)
	    ret = sendrec(RECEIVE, src_dest, msg);
	break;
    case SEND:
    case RECEIVE:
	ret = sendrec(function, src_dest, msg);
    default:
	assert(function == BOTH || function == SEND || function == RECEIVE);
	break;
    }
    return ret;
}
