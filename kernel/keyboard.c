#include "headers.h"
#include "keymap.h"


PRIVATE KB_INPUT    kb_in;

PRIVATE int code_with_E0;
PRIVATE int shift_l;		/* l shift state */
PRIVATE int shift_r;
PRIVATE int alt_l;
PRIVATE int alt_r;
PRIVATE int ctrl_l;
PRIVATE int ctrl_r;
PRIVATE int caps_lock;
PRIVATE int num_lock;
PRIVATE int scroll_lock;
PRIVATE int column;

PRIVATE u8   get_byte_from_buf();
PRIVATE void kb_wait();
PRIVATE void kb_ack();
PRIVATE void set_leds();

/* 键盘中断处理程序 */
PUBLIC void keyboard_handler(int irq) {
    u8 scan_code = in_byte(KB_DATA);
    /* 键盘被按下 */
    key_pressed = 1;
    
    if (kb_in.count < KB_IN_BYTES) {
	*(kb_in.p_head) = scan_code;
	kb_in.p_head++;
	if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
	    kb_in.p_head = kb_in.buf;
	}
	kb_in.count += 1;
	
	/* disp_int(kb_in.count); */
    }
}

PUBLIC void init_keyboard(TTY *p_tty) {
    kb_in.count  = 0;
    kb_in.p_head = kb_in.p_tail = kb_in.buf;

    code_with_E0 = 0;
    shift_l = shift_r = 0;
    ctrl_l  = ctrl_r  = 0;
    alt_l   = alt_r   = 0;
    num_lock    = 1;
    caps_lock   = 0;
    scroll_lock = 0;

    set_leds();
	
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}

PUBLIC void keyboard_read(TTY* p_tty) {
    u8   scan_code;
    char output[2];
    int  make;			/* Boolean to indicate Make Code or Break Code */

    u32  key = 0;
    u32  *keyrow;

    memset(output, 0, 2);
    if (kb_in.count > 0) {
	code_with_E0 = 0;

	scan_code = get_byte_from_buf();
	
     	/* 解析扫描码 */
	if (scan_code == 0xE1) {
	    u8 pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
	    int is_pausebreak = 1;

	    for (int i=0; i<6; i++) {
		if (get_byte_from_buf() != pausebrk_scode[i]) {
		    is_pausebreak = 0;
		    break;
		}
	    }

	    if (is_pausebreak) {
		key = PAUSEBREAK;
	    }
	} else if (scan_code == 0xE0) {
	    scan_code = get_byte_from_buf();

	    if (scan_code == 0x2A) {
		/* Print Screen按下 */
		if (get_byte_from_buf() == 0xE0) {
		    if (get_byte_from_buf() == 37) {
			key = PRINTSCREEN;
			make = 1;
		    }
		}
	    }
	    if (scan_code == 0xB7) {
		/* Print Screen被释放 */
		if (get_byte_from_buf() == 0xE0) {
		    if (get_byte_from_buf() == 0xAA) {
			key = PRINTSCREEN;
			make = 0;
		    }
		}
	    }
	    if (key == 0) {
		/* 不是Print Screen */
		code_with_E0 = 1;
	    }
	}
	if (key != PAUSEBREAK && key != PRINTSCREEN ){
	    make = (scan_code & FLAG_BREAK) ? FALSE : TRUE; /* Make code <= 0x7F */

	    keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
	    
	    column = 0;

	    int caps = shift_l || shift_r;

	    if (caps_lock) {
		if (keyrow[0] >= 'a' && keyrow[0] <= 'z') {
		    caps = !caps;
		}
	    }

	    if (caps) {
		column = 1;
	    }
	    
	    /* if (shift_l || shift_r) { */
	    /* 	column = 1; */
	    /* } */
	    if (code_with_E0) {
		code_with_E0 = 0;
		column = 2;
	    }

	    key = keyrow[column];

	    switch(key) {
	    case SHIFT_L:
		shift_l = make;
		key = 0;
		break;
	    case SHIFT_R:
		shift_r = make;
		key = 0;
		break;
	    case CTRL_L:
		ctrl_l = make;
		key = 0;
		break;
	    case CTRL_R:
		ctrl_r = make;
		key = 0;
		break;
	    case ALT_L:
		alt_l = make;
		key = 0;
		break;
	    case ALT_R:
		alt_r = make;
		key = 0;
		break;
	    case CAPS_LOCK:
		if (make) {
		    caps_lock = !caps_lock;
		    set_leds();
		}
		break;
	    case NUM_LOCK:
		if (make) {
		    num_lock = !num_lock;
		    set_leds();
		}
		break;
	    case SCROLL_LOCK:
		if (make) {
		    scroll_lock = !scroll_lock;
		    set_leds();
		}
		break;
	    default:
		break;
	    }

	    if (make) {
		int pad = 0;
		/* 处理小键盘 */
		if (key >= PAD_SLASH && key <= PAD_9) {
		    pad = 1;
		    switch(key) {
		    case PAD_SLASH:
			key = '/';
			break;
		    case PAD_STAR:
			key = '*';
			break;
		    case PAD_MINUS:
			key = '-';
			break;
		    case PAD_PLUS:
			key = '+';
			break;
		    case PAD_ENTER:
			key = '\n';
			break;
		    default:
			if (num_lock && (key <= PAD_9 && key >= PAD_0)) {
			    key = key - PAD_0 + '0';
			} else if (num_lock && key == PAD_DOT) {
			    key = '.';
			} else {
			    switch(key) {
			    case PAD_HOME:
				key = HOME;
				break;
			    case PAD_END:
				key = END;
				break;
			    case PAD_PAGEUP:
				key = PAGEUP;
				break;
			    case PAD_PAGEDOWN:
				key = PAGEDOWN;
				break;
			    case PAD_INS:
				key = INSERT;
				break;
			    case PAD_DEL:
				key = DELETE;
				break;
			    case PAD_UP:
				key = UP;
				break;
			    case PAD_DOWN:
				key = DOWN;
				break;
			    case PAD_LEFT:
				key = LEFT;
				break;
			    case PAD_RIGHT:
				key = RIGHT;
				break;
			    /* case PAD_DOT: */
			    /* 	key = DELETE; */
			    /* 	break; */
			    default:
				break;
			    }
			}
			break;
		    }
		}
		key |= shift_l ? FLAG_SHIFT_L : 0;
		key |= shift_r ? FLAG_SHIFT_R : 0;
		key |= ctrl_l  ? FLAG_CTRL_L  : 0;
		key |= ctrl_r  ? FLAG_CTRL_R  : 0;
		key |= alt_l   ? FLAG_ALT_L   : 0;
		key |= alt_r   ? FLAG_ALT_R   : 0;
		key |= pad     ? FLAG_PAD     : 0;
		inprocess(p_tty, key);
	    }
	}
    }
}


PRIVATE u8 get_byte_from_buf() {
    while (kb_in.count <= 0) {}	/* 等待下一个字符 */

    disable_int();

    u8 scan_code = *(kb_in.p_tail);
    kb_in.p_tail++;
    if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
	kb_in.p_tail = kb_in.buf;
    }
    kb_in.count -= 1;

    enable_int();

    return scan_code;
}

PRIVATE void kb_wait() {
    u8 kb_stat;

    do {
	kb_stat = in_byte(KB_CMD);       
    } while (kb_stat & 0x02);
}

PRIVATE void kb_ack() {
    u8 kb_read;

    do {
	kb_read = in_byte(KB_DATA);
    } while (kb_read != KB_ACK);
}

PRIVATE void set_leds() {
    u8 leds = (caps_lock << 2) | (num_lock << 1) | (scroll_lock);

    kb_wait();
    out_byte(KB_DATA, LED_CODE);
    kb_ack();
    kb_wait();
    out_byte(KB_DATA, leds);
    kb_ack();
}
