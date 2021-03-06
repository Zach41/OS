#include "headers.h"
#include "keymap.h"

#define TTY_FIRST    (tty_table)
#define TTY_LAST     (tty_table + NR_CONSOLE)

PRIVATE void init_tty(TTY*);
PRIVATE void tty_do_read(TTY*, MESSAGE*);
PRIVATE void tty_do_write(TTY*, MESSAGE*);
PRIVATE void tty_write(TTY*, char* buf, int len);
PRIVATE void tty_dev_write(TTY*);
PRIVATE void tty_dev_read(TTY*);


PRIVATE void put_key(TTY*, u32);

/* 终端任务，负责读键盘，写屏幕等 */
PUBLIC void task_tty() {
    TTY *p_tty;
    MESSAGE msg;
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
	    do {
		tty_dev_read(p_tty);
		tty_dev_write(p_tty);
	    } while(p_tty -> inbuf_count);
	}

	send_recv(RECEIVE, ANY, &msg);

	int src = msg.source;
	assert(src != TASK_TTY);

	TTY* p_tty = &tty_table[msg.DEVICE];

	switch(msg.type) {
	case DEV_OPEN:
	    reset_msg(&msg);
	    msg.type = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	case DEV_READ:
	    tty_do_read(p_tty, &msg);
	    break;
	case DEV_WRITE:
	    tty_do_write(p_tty, &msg);
	    break;
	case HARD_INT:
	    key_pressed = 0;
	    break;
	default:
	    panic("Unknown TTY Messsage.\n");
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

/* PUBLIC int sys_write(int _unused, char* buf, int len, PROCESS *p) { */
/*     tty_write(&tty_table[p -> nr_tty], buf, len); */
/*     return 0; */
/* } */
PRIVATE void tty_do_read(TTY *tty, MESSAGE* msg) {
    tty -> tty_caller = msg -> source; /* 通常是FS发送的消息 */
    tty -> tty_procnr = msg -> PROC_NR;
    tty -> tty_left_cnt = msg -> CNT;
    tty -> tty_trans_cnt = 0;
    tty -> tty_req_buf = va2la(tty -> tty_procnr, msg -> BUF);

    msg -> type = SUSPEND_PROC;

    /* 立即返回 */
    send_recv(SEND, tty -> tty_caller, msg);
    
}

PRIVATE void tty_do_write(TTY *tty, MESSAGE* msg) {
    char buf[1];
    char* p = (char*)va2la(msg -> PROC_NR, msg -> BUF);
    int left = msg -> CNT;

    while (left) {
	memcpy(va2la(TASK_TTY, buf), (void*)p, 1);
	out_char(tty -> p_console, *buf);

	left -= 1;
	p    += 1;
    }
    msg -> type = SYSCALL_RET;
    send_recv(SEND, msg -> source, msg);
}

PRIVATE void init_tty(TTY* p_tty) {
    p_tty -> inbuf_count   = 0;
    p_tty -> p_inbuf_head  = p_tty -> p_inbuf_tail = p_tty -> in_buf;
    p_tty -> tty_caller    = NO_TASK;
    p_tty -> tty_procnr    = NO_TASK;
    p_tty -> tty_left_cnt  = 0;
    p_tty -> tty_trans_cnt = 0;
    p_tty -> tty_req_buf   = 0;
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

PUBLIC int sys_printx(int _unused1, int _unused2, char *s, PROCESS* p_proc) {
    const char* p;
    char ch;

    char reenter_err[] = "? k_reenter is incorrect for unknown reason";
    reenter_err[0]     = MAG_CH_PANIC;

    if (k_reenter == 0) {
	/* ring0~3调用的printx */
	p = va2la(proc2pid(p_proc), s);
    }
    else if (k_reenter > 0) {
	p = s;			/* ring0 调用printx */
    } else {
	p = reenter_err;
    }

    if (*p == MAG_CH_PANIC || (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
	/* 如果是在任务中assert失败 */
	disable_int();

	char *v = (char*)V_MEM_BASE;

	const char *q = p + 1;	/* 跳过magic char */
	while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
	    *v++ = *q++;
	    *v++ = MAKE_COLOR(BLACK, RED);

	    if (!*q) {
		while (((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16)) {
		    v++;
		    *v++ = MAKE_COLOR(BLACK, WHITE);
		}
		q = p+1;
	    }
	}
	__asm__ __volatile__("hlt");
    }

    while ((ch = *p++) != 0) {
	if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
	    continue;

	out_char(tty_table[0].p_console, ch);
    }
    return 0;
}


PRIVATE void tty_dev_write(TTY* tty) {
    while (tty -> inbuf_count) {
	char ch = *(tty -> p_inbuf_tail);
	tty -> p_inbuf_tail++;
	if (tty -> p_inbuf_tail == tty -> in_buf + TTY_IN_BYTES) {
	    tty -> p_inbuf_tail = tty -> in_buf;
	}
	tty -> inbuf_count--;

	if (tty -> tty_left_cnt) {
	    if (ch >= ' ' && ch <= '~') {
		/* 可以打印的字符传给进程 */
		out_char(tty -> p_console, ch);
		void* p = tty -> tty_req_buf + tty -> tty_trans_cnt;
		memcpy(p, (void*)va2la(TASK_TTY, &ch), 1);
		tty -> tty_left_cnt--;
		tty -> tty_trans_cnt++;
	    } else if (ch == '\b' && tty -> tty_trans_cnt) {
		/* 如果是backspace，而且已经有输入 */
		out_char(tty -> p_console, ch);
		tty -> tty_trans_cnt--;
		tty -> tty_left_cnt++;
	    }

	    if (ch == '\n' || tty -> tty_left_cnt == 0) {
		/* 如果字符输入完毕或者遇到换行符 */
		out_char(tty -> p_console, '\n');
		MESSAGE msg;
		msg.type = RESUME_PROC;
		msg.PROC_NR = tty -> tty_procnr;
		msg.CNT = tty -> tty_trans_cnt;
		send_recv(SEND, tty -> tty_caller, &msg);
		tty -> tty_left_cnt = 0;
	    }
	}
    }
}


PRIVATE void tty_dev_read(TTY* tty) {
    if (is_current_console(tty -> p_console))
	keyboard_read(tty);
}
