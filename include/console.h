#ifndef _ZACH_CONSOLE_H_
#define _ZACH_CONSOLE_H_

typedef struct s_console {
    u32    current_start_addr;	/* 当前现实到了什么地方 */
    u32    original_addr;	/* 当前控制台对应的显存位置 */
    u32    v_mem_limit;		/* 当前控制台的显存大小 */
    u32    cursor;		/* 当前光标位置 */
}CONSOLE;

#endif
