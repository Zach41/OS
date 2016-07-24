#include "headers.h"

PRIVATE void init_mm();

PUBLIC void task_mm() {
    init_mm();
    
    printl("TASK MM begins.\n");

    while (TRUE) {
	send_recv(RECEIVE, ANY, &mm_msg);
	int src = mm_msg.source;
	int reply = 1;

	switch(mm_msg.type) {
	case FORK:
	    mm_msg.RETVAL = do_fork();
	    break;
	default:
	    panic("Unknown message in MM.");
	    break;
	}

	if (reply) {
	    mm_msg.type = SYSCALL_RET;
	    send_recv(SEND, src, &mm_msg);
	}
    }
}

PUBLIC int alloc_mem(int pid, int memsize) {
    assert(pid >= NR_TASKS + NR_NATIVE_PROCS);

    if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
	panic("Can not alloc memory that has size bigger than 1MB");
    }
    int base = PROCS_BASE + (pid - NR_TASKS - NR_NATIVE_PROCS) * PROC_IMAGE_SIZE_DEFAULT;

    if (base + memsize >= memory_size) {
	panic("memory is full");
    }

    return base;
}

PRIVATE void init_mm() {
    BOOT_PARAMS bp;
    get_boot_params(&bp);

    memory_size = bp.memsize;

    printl("MM memsize: %dMB\n", memory_size / (1024*1024));
}


