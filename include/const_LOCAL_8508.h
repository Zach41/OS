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

/* 8253计数器端口 */
#define TIMER_MODE       0x43
#define TIMER0           0x40
#define TIMER1           0x41
#define TIMER2           0x42

#define TIMER_FREQ       1193182L
#define HZ               100	/* 每10ms产生一次中断 */
#define RATE_GENERATOR   0x34	/* 模式控制寄存器的值，00110100 */

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

/* 系统调用相关的变量 */
#define NR_SYS_CALL    3

/* 一些有用的定义 */
/* Boolean */
#define TRUE    1
#define FALSE   0

/* Color */
#define BLACK    0x0
#define WHITE    0x7
#define RED      0x4
#define GREEN    0x2
#define BLUE     0x1
#define FLASH    0x80
#define BRIGHT   0x08

#define MAKE_COLOR(x, y) (x | y) /* (BG, FG) */

#define DEFAULT_COLOR MAKE_COLOR(BLACK, WHITE)

/* VGA寄存器 */
#define CRTC_ADDR_REG    0x3D4	  /* CRT Controller Address Register */
#define CRTC_DATA_REG    0x3D5	  /* CRT Controller Data Register */
#define START_ADDR_H     0xC	  /* reg index of video mem start addr (HIGH) */
#define START_ADDR_L     0xD	  /* reg index of video mem start addr (LOW) */
#define CURSOR_H         0xE	  /* reg index of cursor location (HIGH) */
#define CURSOR_L         0xF	  /* reg index of cursor location (LOW) */
#define V_MEM_BASE       0xB8000  /* videmo mem base addr */
#define V_MEM_SIZE       0x8000	  /* we have 32KB in total */

/* 控制台个数 */
#define NR_CONSOLE    3

#define SCREEN_WIDTH  80
#define SCREEN_SIZE   (80*25)

#define SCR_DN        TRUE	/* 卷行的方向, down*/
#define SCR_UP        FALSE	/* up */

/* ASSERT */
#define ASSERT
#ifdef  ASSERT
void assert_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp) if (exp) ; \
    else assert_failure(#exp, __FILE__, __BASE_FILE__, __LINE__);
#else
#define assert(exp)
#endif

/* magic chars used by `printx' */
#define MAG_CH_PANIC	'\002'
#define MAG_CH_ASSERT	'\003'

enum msgtype {
    HARD_INT = 1,
    GET_TICKS
};

/* IPC */
#define SEND    1
#define RECEIVE 2
#define BOTH    3

#define SENDING    0x02
#define RECEIVING  0x04

#define ANY        (NR_TASKS + NR_PROCS + 255)
#define NO_TASK    (NR_TASKS + NR_PROCS + 511)
#define INTERRUPT  -10

#define RETVAL  u.m3.m3i1

#define TASK_SYS   1
   

#endif