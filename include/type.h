#ifndef _ZACH_TYPE_H
#define _ZACH_TYPE_H

typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

typedef void (*int_handler) ();

/* 进程任务函数指针 */
typedef void (*task_f) ();

/* 中断处理函数指针 */
typedef void (*irq_handler) (int irq);

#endif
