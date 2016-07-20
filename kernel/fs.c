#include "const.h"
#include "type.h"
#include "hd.h"
#include "fs.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/* 文件系统进程 */
PUBLIC void task_fs() {
    printl("Task FS begins\n");

    MESSAGE driver_msg;

    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);

    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    spin("FS");
}
