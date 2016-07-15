#include "const.h"
#include "type.h"
#include "proto.h"
#include "protect.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"

/* 终端任务，负责读键盘，写屏幕等 */
PUBLIC void task_tty() {
    while (TRUE) {
	/* disp_str("*"); */
	keyboard_read();
    }
}

PUBLIC void inprocess(u32 key) {
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT)) {
	/* 这个简单的函数没有用到shift等状态位 */
	output[0] =  key & 0xFF;
	disp_str(output);
    }
}
