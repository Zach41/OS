#include "const.h"
#include "type.h"
#include "protect.h"
#include "console.h"
#include "tty.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

PRIVATE void init_hd();
PRIVATE void hd_identify(int drive);
PRIVATE void print_identify_info(u16* hdinfo);
PRIVATE void hd_cmd_out(HD_CMD* cmd);
PRIVATE void interrupt_wait();
PRIVATE int  waitfor(int mask, int val, int timeout);

PRIVATE u8   hd_status;
PRIVATE u8   hdbuf[SECTOR_SIZE * 2];

PUBLIC void hd_handler(int irq) {
    hd_status = in_byte(REG_STATUS);

    inform_int(TASK_HD);
}

PUBLIC void task_hd() {
    MESSAGE msg;

    init_hd();

    while (TRUE) {
	send_recv(RECEIVE, ANY, &msg);

	int src = msg.source;

	switch(msg.type) {
	case DEV_OPEN:
	    /* 获得硬盘的信息 */
	    hd_identify(0);
	    break;
	default:
	    panic("HD driver::unknown msg");
	}

	send_recv(SEND, src, &msg);
    }
}

PRIVATE void init_hd() {
    /* 初始化硬盘 */
    u8* pNrDrives = (u8*)(0x475); /* 从0x475处取得系统硬盘数，地址由BIOS指定 */
    printl("NR Drives: %d\n", *pNrDrives);
    assert(*pNrDrives);

    put_irq_handler(AT_WINI_IRQ, hd_handler);
    /* 开开启级联的中断口 */
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);
}

PRIVATE void hd_identify(int drive) {
    HD_CMD cmd;

    cmd.device  = MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;	/* 向硬盘发送IDENTIFY指令 */

    hd_cmd_out(&cmd);
    interrupt_wait();

    /* 中断结束 */
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);

    print_identify_info((u16*)hdbuf);
}

PRIVATE void print_identify_info(u16* hdinfo) {
    /* P334 table 9.2 */
    struct iden_info_ascii {
	/* 
	 idx:  开始偏移量(2个字节一个偏移)
	 len:  字符串长度
	 desc: 描述符
	 */
	int idx;
	int len;
	char *desc;
    } iinfo[] = {
	{10, 20, "HD SN"},	/* 序列号(20个ASCII字符) */
	{27, 40, "HD Model"}	/* 型号(40个ASCII字符) */
    };

    char s[64];
    for (int i=0; i<2; i++) {
	char* p = (char*)&hdinfo[iinfo[i].idx];
	int j;
	for (j=0; j<iinfo[i].len/2; j++) {
	    s[j*2 + 1] = *p++;
	    s[j*2]     = *p++;
	}
	s[j*2] = 0;
	printl("%s: %s\n", iinfo[i].desc, s);
    }
    int capabilities = hdinfo[49];
    printl("LBA supported: %s\n", (capabilities & 0x0200) ? "YES" : "NO");

    int cmd_set_supported = hdinfo[83];
    printl("LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "YES" : "NO");

    int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    printl("HD size: %dMB\n", sectors * SECTOR_SIZE / 1000000);
}

PRIVATE void hd_cmd_out(HD_CMD* cmd) {
    if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
	panic("hd error");

    out_byte(REG_DEV_CTRL, 0);	/* 打开硬盘中断 */
    /* 加载参数，依次为features, sector count, lba low, lba mid, lba high, device, command */
    out_byte(REG_FEATURES, cmd -> features);
    out_byte(REG_NSECTOR, cmd -> count);
    out_byte(REG_LBA_LOW, cmd -> lba_low);
    out_byte(REG_LBA_MID, cmd -> lba_mid);
    out_byte(REG_LBA_HIGH, cmd -> lba_high);
    out_byte(REG_DEVICE, cmd -> device);
    out_byte(REG_CMD, cmd -> command);
}

PRIVATE void interrupt_wait() {
    MESSAGE msg;

    send_recv(RECEIVE, INTERRUPT, &msg);
}

PRIVATE int waitfor(int mask, int val, int timeout) {
    int t = get_ticks();

    while (((get_ticks() -t) * 1000 / HZ) < timeout) {
	/* 注意优先级, `==` 高于 `&` */
	if ((in_byte(REG_STATUS) & mask) == val) {
	    return 1;
	}
    }
    return 0;
}
