#ifndef _ZACH_TTY_H_
#define _ZACH_TTY_H_

#define TTY_IN_BYTES    256	/* tty 输入缓冲区大小 */

/* struct s_console; */

typedef struct s_tty {
    u32    in_buf[TTY_IN_BYTES];
    u32*   p_inbuf_head;	/* 下一个空闲的位置 */
    u32*   p_inbuf_tail;	/* 指向要处理的位置 */
    int    inbuf_count;

    int    tty_caller;		/* the process that sends messages to tty task (generally it's task FS)*/
    int    tty_procnr;		/* the process that requests data */
    int    tty_left_cnt;	/* number of bytes that process wants to read */
    int    tty_trans_cnt;	/* number of bytes that has been transferred */
    void*  tty_req_buf;		/* buffer that process reads */

    struct s_console    *p_console; /* 当前tty的console */
}TTY;

#endif
