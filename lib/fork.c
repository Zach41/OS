#include "headers.h"
#include "stdio.h"

PUBLIC int fork() {
    MESSAGE msg;

    msg.type = FORK;
    send_recv(BOTH, TASK_MM, &msg);
    assert(msg.type == SYSCALL_RET);
    assert(msg.RETVAL == 0);

    return msg.PID;
}

PUBLIC int do_fork() {
    PROCESS* p= proc_table;

    /* 找到一个空槽 */
    int i;
    for (i=0; i<NR_PROCS + NR_TASKS; i++, p++) {
	if (p -> p_flags == FREE_SLOT)
	    break;
    }
    assert(i >= NR_TASKS + NR_NATIVE_PROCS);
    int child_pid = i;
    if (i == NR_PROCS + NR_TASKS)
	return -1;

    /* 复制进程控制块 */
    int pid = mm_msg.source;
    u16 child_select = p -> ldt_sel;
    *p = proc_table[pid];
    p -> ldt_sel = child_select;
    p -> p_parent = pid;
    p -> pid = child_pid;
    sprintf(p -> p_name, "%s_%d", proc_table[pid].p_name, child_pid);

    /* dump_proc(p); */

    /* 复制代码 */
    DESCRIPTOR* ppd = &proc_table[pid].ldts[0];
    
    int caller_T_base = reassembly(ppd -> base_high, 24,
				   ppd -> base_mid, 16,
				   ppd -> base_low);
    int caller_T_limit = reassembly((ppd -> limit_high_attr2 & 0xF), 16,
				    0, 0,
				    ppd -> limit_low);
    int caller_T_size  = (caller_T_limit + 1) *
	((ppd -> limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1);
    /* 复制数据 */
    ppd = &proc_table[pid].ldts[1];
    int caller_D_S_base = reassembly(ppd -> base_high, 24,
				     ppd -> base_mid, 16,
				     ppd -> base_low);
    int caller_D_S_limit = reassembly((ppd -> limit_high_attr2 & 0xF), 16,
				      0, 0,
				      ppd -> limit_low);
    int caller_D_S_size = (caller_D_S_limit + 1) *
	((ppd -> limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1);

    assert((caller_T_base == caller_D_S_base) &&
	   (caller_T_limit == caller_D_S_limit) &&
	   (caller_T_size == caller_D_S_size));

    int child_base = alloc_mem(child_pid, caller_T_size);
    memcpy((void*)child_base, (void*)caller_T_base, caller_T_size);

    /* 初始化LDT表 */
    init_descriptor(&p -> ldts[0],
		    child_base,
		    (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		    DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
    init_descriptor(&p -> ldts[1],
		    child_base,
		    (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		    DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

    MESSAGE msg2fs;
    msg2fs.type = FORK;
    msg2fs.PID  = child_pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    mm_msg.PID = child_pid;

    /* 发送消息给子进程，以免子进程一直被阻塞 */
    MESSAGE child_msg;
    child_msg.type = SYSCALL_RET;
    child_msg.RETVAL = 0;
    child_msg.PID = 0;
    send_recv(SEND, child_pid, &child_msg);

    return 0;
}
