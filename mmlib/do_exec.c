#include "headers.h"
#include "stdio.h"
#include "elf.h"

PUBLIC int do_exec() {
    char pathname[MAX_PATH];
    
    int  name_len = mm_msg.NAME_LEN;
    int src = mm_msg.source;

    memcpy((void*)va2la(TASK_MM, pathname),
	   (void*)va2la(src, mm_msg.PATHNAME),
	   name_len);

    pathname[name_len] = 0;

    struct stat s;
    int ret = stat(pathname, &s);
    if (ret != 0) {
	printl("MM::do_exec::stat() pathname: %s failed.\n", pathname);
	return -1;
    }

    int fd = open(pathname, O_RDWR);
    if (fd == -1)
	return -1;

    assert(s.st_size < MMBUF_SIZE);
    read(fd, mmbuf, s.st_size);
    close(fd);

    /* 拷贝程序至对应的内存位置 */
    Elf32_Ehdr *header = (Elf32_Ehdr*)mmbuf;

    for (int i=0; i<header -> e_phnum; i++) {
	Elf32_Phdr *p_header = (Elf32_Phdr*)(mmbuf + header -> e_phoff +
					     i * (header -> e_phentsize));

	if (p_header -> p_type == PT_LOAD) {
	    assert(p_header -> p_vaddr + p_header -> p_memsz < PROC_IMAGE_SIZE_DEFAULT);

	    memcpy((void*)va2la(src, (void*)p_header -> p_vaddr),
		   (void*)va2la(TASK_MM, mmbuf + p_header -> p_offset),
		   p_header -> p_filesz);
	}
    }

    /* 中心调整堆栈 */
    int orig_stack_len = mm_msg.BUF_LEN;
    assert(mm_msg.BUF_LEN == 33);
    char stackcopy[PROC_ORIGIN_STACK];
    

    memcpy((void*)va2la(TASK_MM, stackcopy),
	   (void*)va2la(src, mm_msg.BUF),
	   orig_stack_len);

    /* printl("PARAM1: %s\n", stackcopy+12); */
    /* printl("PARAM2: %s\n", stackcopy+18); */
    u8* orig_stack = (u8*)(PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK);
    int delta = (int)orig_stack - (int)mm_msg.BUF;

    int argc = 0;
    if (orig_stack_len) {
	char** q = (char**)stackcopy;
	for(; *q!=0 ; q++, argc++) {
	    *q += delta;
	}
    }

    /* printl("ARGC: %d\n", argc); */

    memcpy((void*)va2la(src, orig_stack),
	   (void*)va2la(TASK_MM, stackcopy),
	   orig_stack_len);
    proc_table[src].regs.ecx = argc;
    proc_table[src].regs.eax = (u32)orig_stack; /* argv */

    proc_table[src].regs.eip = header -> e_entry; /* 程序入口 */
    proc_table[src].regs.esp = PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK;

    strcpy(proc_table[src].p_name, pathname);

    return 0;
    
    
    
}
