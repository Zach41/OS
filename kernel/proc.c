#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

/* 系统调用的函数 */
PUBLIC int sys_get_ticks() {
    /* 返回当前的时钟数 */
    return ticks;
}
