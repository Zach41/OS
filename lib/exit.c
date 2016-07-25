#include "headers.h"
#include "stdio.h"

PRIVATE void cleanup(PROCESS* p);

PUBLIC void do_exit(int status) {
    int pid  = mm_msg.source;
    int ppid = proc_table[pid].p_parent;

    PROCESS* p = &proc_table[pid];

    MESSAGE msg2fs;
    msg2fs.type = EXIT;
    msg2fs.PID  = pid;
    send_recv(BOTH, TASK_FS, &msg2fs); /* 告诉FS这个进程退出了 */

    free_mem(pid);		/* 释放内存, 之后p只是占用一个进程控制块项*/

    p -> exit_status = status;	/* 存储结束状态，等待父进程取走这一状态，一旦取走，子进程就结束了exit */

    if (proc_table[ppid].p_flags & WAITING) {
	proc_table[ppid].p_flags &= ~WAITING;
	cleanup(p);
    } else {
	p -> p_flags |= HANGING;
    }

    /* 如果p有任何子进程，将子进程过继给Init进程 */
    for (int i=0; i<NR_PROCS+NR_TASKS; i++) {
	if (proc_table[i].p_parent == pid) {
	    proc_table[i].p_parent = INIT;
	    if ((proc_table[INIT].p_flags & WAITING) &&
		(proc_table[i].p_flags & HANGING)) {
		proc_table[INIT].p_flags &= ~WAITING;
		cleanup(&proc_table[i]);
	    }
	}
    }
}

PUBLIC void do_wait() {
    int pid = mm_msg.source;

    /* 如果有任何子进程要退出 */
    int has_child = 0;
    for (int i=0; i<NR_PROCS + NR_TASKS; i++) {
	if (proc_table[i].p_parent == pid) {
	    has_child = 1;
	    if (proc_table[i].p_flags & HANGING) {
		cleanup(proc_table + i);
		return;
	    }
	}
    }

    if (has_child) {
	proc_table[pid].p_flags |= WAITING;
    } else {
	/* 没有任何子进程 */
	MESSAGE msg;
	msg.type = SYSCALL_RET;
	msg.PID  = NO_TASK;
	send_recv(SEND, pid, &msg);
    }
}

PRIVATE void cleanup(PROCESS* p) {
    MESSAGE msg;

    msg.type   = SYSCALL_RET;
    msg.PID    = p -> pid;
    msg.STATUS = p -> exit_status;

    send_recv(SEND, p -> p_parent, &msg);

    p -> p_flags = FREE_SLOT;
} 
