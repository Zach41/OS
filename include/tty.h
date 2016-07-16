#ifndef _ZACH_TTY_H_
#define _ZACH_TTY_H_

#define TTY_IN_BYTES    256	/* tty 输入缓冲区大小 */

/* struct s_console; */

typedef struct s_tty {
    u32    in_buf[TTY_IN_BYTES];
    u32*   p_inbuf_head;	/* 下一个空闲的位置 */
    u32*   p_inbuf_tail;	/* 指向要处理的位置 */
    int    inbuf_count;

    struct s_console    *p_console; /* 当前tty的console */
}TTY;

#endif
