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

/* RPL */
#define RPL_KRNL    SA_RPL0
#define RPL_TASK    SA_RPL1
#define RPL_USER    SA_RPL3

/* 中断的个数 */
#define NR_IRQ    16

/* 硬件中断号 */
#define CLOCK_IRQ     0
#define KEYBOARD_IRQ  1
#define CASCADE_IRQ   2
#define ETHER_IRQ     3		/* default ethernet interrupt vector */
#define SECONDARY_IRQ 3
#define RS232_IRQ     4
#define XT_WINI_IRQ   5
#define FLOPPY_IRQ    6
#define PRINTER_IRQ   7
#define AT_WINI_IRQ   14

#endif
