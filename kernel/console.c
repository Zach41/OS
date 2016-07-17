#include "const.h"
#include "type.h"
#include "console.h"
#include "tty.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PRIVATE void set_cursor(u32 position);
PRIVATE void flush(CONSOLE*);
PRIVATE void set_video_start_addr(u32);
PRIVATE void scroll_screen(CONSOLE*, int);

PUBLIC int is_current_console(CONSOLE *p_con) {
    return (p_con == &console_table[nr_current_console]);
}

PUBLIC void out_char(CONSOLE *p_con, char ch) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con -> cursor * 2);

    switch(ch) {
    case '\n':
	/* 回车键 */
	if (p_con -> cursor < p_con -> original_addr + p_con -> v_mem_limit - SCREEN_WIDTH) {
	    int line = (p_con -> cursor - p_con -> original_addr) / SCREEN_WIDTH + 1;
	    p_con -> cursor = p_con -> original_addr + SCREEN_WIDTH * line;
	}
	break;

    case '\b':
	/* Backspace */
	if (p_con -> cursor > p_con -> original_addr) {
	    p_con -> cursor--;
	    *(p_vmem-2) = ' ' ;
	    *(p_vmem-1) = DEFAULT_COLOR;
		
	}
	break;
    default:
	if (p_con -> cursor < p_con -> original_addr + p_con -> v_mem_limit - 1) {
	    *p_vmem++ = ch;
	    *p_vmem++ = DEFAULT_COLOR;
	    p_con -> cursor++;
	}
    }

    while (p_con -> cursor >= p_con -> current_start_addr + SCREEN_SIZE) {
	scroll_screen(p_con, SCR_DN);
    }

    while (p_con -> cursor < p_con -> current_start_addr) {
	/* 向上卷 */
	scroll_screen(p_con, SCR_UP);
    }

    flush(p_con);
}

PUBLIC void init_screen(TTY* p_tty) {
    int nr_tty = p_tty - tty_table;
    p_tty -> p_console = &console_table[nr_tty];

    int v_mem_size = V_MEM_SIZE >> 1; /* 以双字节计的显存大小 */

    /* 一个字符占两个字节 */
    int con_v_mem_size = v_mem_size / NR_CONSOLE;
    p_tty -> p_console -> original_addr      = nr_tty * con_v_mem_size;
    p_tty -> p_console -> current_start_addr = p_tty -> p_console -> original_addr;
    p_tty -> p_console -> v_mem_limit        = con_v_mem_size;
    p_tty -> p_console -> cursor             = p_tty -> p_console -> original_addr;

    if (nr_tty == 0) {
	/* 保留原来的光标 */
	p_tty -> p_console -> cursor = disp_pos / 2;
	disp_pos = 0;
    } else {
	out_char(p_tty -> p_console, nr_tty + '0');
	out_char(p_tty -> p_console, '#');
    }

    set_cursor(p_tty -> p_console -> cursor);
}

PUBLIC void select_console(int nr_console) {
    if (nr_console < 0 || nr_console >= NR_CONSOLE)
	return;

    nr_current_console = nr_console;

    /* set_cursor(console_table[nr_current_console].cursor); */
    /* set_video_start_addr(console_table[nr_current_console].current_start_addr); */
    flush(&console_table[nr_current_console]);
}

PRIVATE void set_cursor(u32 position) {
    disable_int();

    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, (position) & 0xFF);
    enable_int();
}

PRIVATE void set_video_start_addr(u32 addr) {
    disable_int();

    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);

    enable_int();
}

PRIVATE void flush(CONSOLE* p_con) {
    if (is_current_console(p_con)) {
	set_cursor(p_con -> cursor);
	set_video_start_addr(p_con -> current_start_addr);
    }
}

PRIVATE void scroll_screen(CONSOLE *p_con, int direction) {
    switch(direction) {
    case SCR_DN:
	if (p_con -> current_start_addr + SCREEN_SIZE <
	    p_con -> original_addr + p_con -> v_mem_limit) {
	    p_con -> current_start_addr += SCREEN_SIZE;
	}
	break;
    case SCR_UP:
	if (p_con -> current_start_addr > p_con -> original_addr + SCREEN_SIZE)
	    p_con -> current_start_addr -=  SCREEN_SIZE;
	break;
    }
}
