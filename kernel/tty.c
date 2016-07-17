#include "const.h"
#include "type.h"
#include "console.h"
#include "tty.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "global.h"
#include "keyboard.h"

#define TTY_FIRST    (tty_table)
#define TTY_LAST     (tty_table + NR_CONSOLE)

PRIVATE void init_tty(TTY*);
PRIVATE void tty_do_read(TTY*);
PRIVATE void tty_do_write(TTY*);
PRIVATE void tty_write(TTY*, char* buf, int len);

PRIVATE void put_key(TTY*, u32);

/* 终端任务，负责读键盘，写屏幕等 */
PUBLIC void task_tty() {
    TTY *p_tty;

    /* 初始化键盘 */
    init_keyboard();

    for (p_tty=TTY_FIRST; p_tty < TTY_LAST; p_tty++) {
	init_tty(p_tty);
    }

    /* 默认是0号控制台 */
    select_console(0);
    
    while (TRUE) {
	/* 不停地轮询tty */
	for (p_tty = TTY_FIRST; p_tty < TTY_LAST; p_tty++) {
	    tty_do_read(p_tty);
	    tty_do_write(p_tty);
	}
    }
}

PUBLIC void inprocess(TTY *p_tty, u32 key) {
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT)) {
	/* 调用顺序: tty_do_read -> keyboard_read -> inprocess */
	put_key(p_tty, key);
	/* if (p_tty -> inbuf_count < TTY_IN_BYTES) { */
	/*     *(p_tty -> p_inbuf_head) = key; */
	/*     p_tty -> p_inbuf_head++; */
	/*     if (p_tty -> p_inbuf_head == p_tty -> in_buf + TTY_IN_BYTES) { */
	/* 	p_tty -> p_inbuf_head = p_tty -> in_buf; */
	/*     } */
	/*     p_tty -> inbuf_count++; */
	/* } */
    } else {
	/* 控制键 */
	int raw_code = key & MASK_RAW; /* MASK_ROW = 0x01FF, 所有的控制键都加上了0x80*/
	switch(raw_code) {
	case ENTER:
	    put_key(p_tty, '\n');
	    break;
	case BACKSPACE:
	    put_key(p_tty, '\b');
	    break;
	case UP:
	    /* UP = 0x25 + FLAG_EXT(0x80) */
	    if (key & FLAG_SHIFT_L || key & FLAG_SHIFT_R) {
		/* 如果shift + 方向键上 */
		disable_int();
		out_byte(CRTC_ADDR_REG, START_ADDR_H);
		out_byte(CRTC_DATA_REG, ((80 * 15) >> 8) & 0xFF);
		out_byte(CRTC_ADDR_REG, START_ADDR_L);
		out_byte(CRTC_DATA_REG, ((80 * 15) & 0xFF));
		enable_int();
	    }
	    break;
	case DOWN:
	    if (key & FLAG_SHIFT_L || key & FLAG_SHIFT_R) {
		/* 向下卷动 */
		disable_int();
		out_byte(CRTC_ADDR_REG, START_ADDR_H);
		out_byte(CRTC_DATA_REG, ((80 * 0) >> 8) & 0xFF);
		out_byte(CRTC_ADDR_REG, START_ADDR_L);
		out_byte(CRTC_DATA_REG, (80 * 0) & 0xFF);
		enable_int();
	    }
	    break;
	case F1:
	case F2:
	case F3:
	    if (key & FLAG_CTRL_L || key & FLAG_CTRL_R) {
		/* F1 ~ F3 + Alt */
		select_console(raw_code - F1);
	    }
	default:
	    break;
	}
    }
    /* printf("ABC%x", 156); */
    /* char buf[256]; */
    /* disp_str(itoa(buf, 123)); */
}

PUBLIC int sys_write(char* buf, int len, PROCESS *p) {
    tty_write(&tty_table[p -> nr_tty], buf, len);
    return 0;
}
PRIVATE void tty_do_read(TTY *p_tty) {
    if (is_current_console(p_tty -> p_console)) {
	/* 如果是当前的控制台 */
	keyboard_read(p_tty);
    }
}

PRIVATE void tty_do_write(TTY *p_tty) {
    if (p_tty -> inbuf_count) {
	char ch = *(p_tty -> p_inbuf_tail);
	p_tty -> p_inbuf_tail++;

	if (p_tty -> p_inbuf_tail == p_tty -> in_buf + TTY_IN_BYTES) {
	    p_tty -> p_inbuf_tail = p_tty -> in_buf;
	}
	p_tty -> inbuf_count--;

	out_char(p_tty -> p_console, ch);
    }
}

PRIVATE void init_tty(TTY* p_tty) {
    p_tty -> inbuf_count = 0;
    p_tty -> p_inbuf_head = p_tty -> p_inbuf_tail = p_tty -> in_buf;

    init_screen(p_tty);
}

PRIVATE void put_key(TTY* p_tty, u32 key) {
    if (p_tty -> inbuf_count < TTY_IN_BYTES) {
	*(p_tty -> p_inbuf_head) = key;
	p_tty -> p_inbuf_head++;
	p_tty -> inbuf_count++;
	if (p_tty -> p_inbuf_head == p_tty -> in_buf + TTY_IN_BYTES) {
	    p_tty -> p_inbuf_head = p_tty -> in_buf;
	}
    }
}

PRIVATE void tty_write(TTY* p_tty, char* buf, int len) {
    char* p = buf;

    int i = len;

    while (i) {
	out_char(p_tty -> p_console, *p++);
	i--;
    }
}
