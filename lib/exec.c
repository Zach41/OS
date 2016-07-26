#include "headers.h"
#include "stdio.h"

PUBLIC int execl(char *path, char *arg, ...) {
    va_list parg = (va_list)&arg;
    char **p = (char**)parg;

    return execv(path, p);
}

PUBLIC int execv(char* path, char* argv[]) {
    char **p = argv;
    char arg_stack[PROC_ORIGIN_STACK];
    int stack_len = 0;

    /* printf("PROC PATH: %s\n", path); */


    /* 准备参数栈 */
    while (*p++) {
	/* 一个地址和一个字符串指针 */
	assert(stack_len + 2*sizeof(char*) < PROC_ORIGIN_STACK);
	stack_len += sizeof(char*);
    }

    /* printf("STACK_LEN:%d\n", stack_len); */

    *((int*)(&arg_stack[stack_len])) = 0;
    stack_len += sizeof(char*);

    char **q = (char**)arg_stack;
    for(p = argv; *p; p++) {
	*q++ = &arg_stack[stack_len]; /* 第一个字符串的指针 */

	assert(stack_len + strlen(*p) + 1 < PROC_ORIGIN_STACK); /* 1为一个结束符 */
	strcpy(&arg_stack[stack_len], *p);
	stack_len += strlen(*p);
	arg_stack[stack_len] = 0;
	stack_len++;
    }

    /* printf("PARAM1: %s\n", arg_stack+16); */
    /* printf("PARAM2: %s\n", arg_stack+21); */
    /* printf("PARAM3: %s\n", arg_stack+27); */
    
    MESSAGE msg;
    msg.type    = EXEC;
    msg.BUF     = (void*)arg_stack;
    msg.BUF_LEN = stack_len;
    msg.PATHNAME = (void*)path;
    msg.NAME_LEN = strlen(path);

    send_recv(BOTH, TASK_MM, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.RETVAL;
    
}

