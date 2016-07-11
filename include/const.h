#ifndef _ZACH_CONST_H
#define _ZACH_CONST_H

#define PUBLIC
#define PRIVATE    static

#define EXTERN     extern

#define GDT_SIZE    128
#define IDT_SIZE    256

/* 中断的IO端口 */
#define INT_M_CTL        0x20	/* 主片 */
#define INT_M_CTLMASK    0x21
#define INT_S_CTL        0xA0	/* 从片 */
#define INT_S_CTLMASK    0xA1

/* 权限 */
#define PRIVILEGE_KRNL   0
#define PRIVILEGE_TASK   1
#define PRIVILEGE_USER   3


#endif
