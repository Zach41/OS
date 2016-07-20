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

#define DRV_OF_DEV(dev) (dev <= MAX_PRIM ? \
			 dev / NR_PRIM_PER_DRIVE : \
			 (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)
PRIVATE void init_hd();
PRIVATE void hd_open(int device);
PRIVATE void get_part_table(int drive, int sect_nr, PART_ENT* entry);
PRIVATE void partition(int device, int style);
PRIVATE void print_hdinfo(HD_INFO* hdi);
PRIVATE void hd_identify(int drive);
PRIVATE void print_identify_info(u16* hdinfo);
PRIVATE void hd_cmd_out(HD_CMD* cmd);
PRIVATE void interrupt_wait();
PRIVATE int  waitfor(int mask, int val, int timeout);

PRIVATE u8   hd_status;
PRIVATE u8   hdbuf[SECTOR_SIZE * 2];

PRIVATE HD_INFO hd_info[1];

PUBLIC void hd_handler(int irq) {
    hd_status = in_byte(REG_STATUS);
    printl("HD interrupt\n");
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
	    /* hd_identify(0); */
	    hd_open(msg.DEVICE);
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
    /* 开启级联的中断口 */
    enable_irq(CASCADE_IRQ);
    enable_irq(AT_WINI_IRQ);

    for (int i=0; i< (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
	memset(&hd_info[i], 0, sizeof(hd_info[0]));
    }
    hd_info[0].open_cnt = 0;
    
}

/* identify hard drive and print partition table */
PRIVATE void hd_open(int device) {
    int drive = DRV_OF_DEV(device);
    assert(drive == 0);	/* 系统上只有一个硬盘 */

    hd_identify(drive);

    if (hd_info[drive].open_cnt++ == 0) {
	/* 第一次打开硬盘 */
	partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY);
	print_hdinfo(&hd_info[drive]);
    }
}

PRIVATE void get_part_table(int drive, int sect_nr, PART_ENT* entry) {
    HD_CMD cmd;
    /* 设置命令 */
    cmd.features = 0;
    cmd.count    = 1;
    cmd.lba_low  = sect_nr & 0xFF;
    cmd.lba_mid  = (sect_nr >> 8) & 0xFF;
    cmd.lba_high = (sect_nr >> 16) & 0xFF;
    cmd.device   = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF); /* LBA Mode */

    cmd.command  = ATA_READ;

    hd_cmd_out(&cmd);
    /* 等待中断发生 */
    printl("Interrupt Wait\n");
    interrupt_wait();
    printl("Interrupt Done\n");
    /* 数据已经就绪 */
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    /* 分区表的偏移为PARTITION_TABLE_OFFSET = 0x1BE */
    memcpy(entry, hdbuf + PARTITION_TABLE_OFFSET, sizeof(PART_ENT) * NR_PART_PER_DRIVE);
}

/* 
 * @param device Device number
 * @param style  Primary or extended
 */
PRIVATE void partition(int device, int style) {
    int drive = DRV_OF_DEV(device);
    HD_INFO *hdi = &hd_info[drive];

    PART_ENT part_tbl[NR_SUB_PER_DRIVE];
    if (style == P_PRIMARY) {
	/* 如果是主分区 */
	get_part_table(drive, drive, part_tbl);
	printl("Table Read.\n");
	int nr_prim_parts = 0;
	for (int i=0; i<NR_PART_PER_DRIVE; i++) {
	    if (part_tbl[i].sys_id == NO_PART) {
		continue;
	    }

	    nr_prim_parts++;

	    int dev_nr = i+1;
	    hdi -> primary[dev_nr].base = part_tbl[i].start_sect;
	    hdi -> primary[dev_nr].size = part_tbl[i].nr_sects;

	    if (part_tbl[i].sys_id == EXT_PART) {
		partition(device + dev_nr, P_EXTENDED);
	    }
	}
	assert(nr_prim_parts);
    } else if (style == P_EXTENDED) {
	/* 如果是扩展分区 */
	int j = device % NR_PRIM_PER_DRIVE; /* drive number */
	int ext_start_sect = hdi -> primary[j].base;
	int start = ext_start_sect;

	int nr_1st_sub = (j-1)*NR_SUB_PER_PART; /* 第一个逻辑分区号 */

	for(int i=0; i<NR_SUB_PER_PART; i++) {
	    int dev_nr = nr_1st_sub + i; /* 逻辑分区次设备号 */
	    get_part_table(drive, start, part_tbl);

	    hdi -> logical[dev_nr].base = start + part_tbl[0].start_sect;
	    hdi -> logical[dev_nr].size = part_tbl[0].nr_sects;

	    start = ext_start_sect + part_tbl[1].start_sect;

	    if (part_tbl[1].sys_id == NO_PART)
		break;
	}
    } else {
	assert(0);
    }
}

PRIVATE void print_hdinfo(HD_INFO *hdi) {
    for (int i=0; i<NR_PART_PER_DRIVE+1; i++) {
	printl("%sPART_%d: base %d(0x%x), size: %d(0x%x) (in sector)\n",
	       i == 0 ? " " : "     ",
	       i,
	       hdi -> primary[i].base,
	       hdi -> primary[i].base,
	       hdi -> primary[i].size,
	       hdi -> primary[i].size);
    }

    for (int i=0; i<NR_SUB_PER_DRIVE; i++) {
	if (hdi -> logical[i].size == 0)
	    continue;
	printl("         %d: base %d(0x%x), size: %d(0x%x) (in sector\n)",
	       i,
	       hdi -> logical[i].base,
	       hdi -> logical[i].base,
	       hdi -> logical[i].size,
	       hdi -> logical[i].size);
    }
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

    u16* hdinfo = (u16*)hdbuf;

    hd_info[drive].primary[0].base = 0;
    hd_info[drive].primary[0].size = ((int)hdinfo[61] << 16) + hdinfo[60];
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
