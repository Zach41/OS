#include "headers.h"

PRIVATE void init_fs();
PRIVATE void mkfs();
PRIVATE void read_super_block(int dev);

/* 文件系统进程 */
PUBLIC void task_fs() {
    printl("Task FS begins\n");
    
    init_fs();

    while (TRUE) {
	send_recv(RECEIVE, ANY, &fs_msg);

	int src = fs_msg.source;
	pcaller = &proc_table[src];

	switch(fs_msg.type) {
	case OPEN:
	    fs_msg.FD = do_open();
	    break;
	case CLOSE:
	    fs_msg.RETVAL = do_close();
	    break;
	case READ:
	case WRITE:
	    fs_msg.CNT = do_rdwt();
	    break;
	case UNLINK:
	    fs_msg.RETVAL = do_unlink();
	    break;
	default:
	    panic("FS:: unknown message.\n");
	}

	fs_msg.type = SYSCALL_RET;
	send_recv(SEND, src, &fs_msg);
    }
    spin("FS");
}

PUBLIC struct super_block* get_super_block(int dev) {
    struct super_block* sb = super_block;
    for(; sb<super_block+NR_SUPER_BLOCK; sb++) {
	if (sb -> sb_dev == dev)
	    return sb;
    }
    panic("super block of device %d not found.\n", dev);
    return 0;
}

PUBLIC struct inode* get_inode(int dev, int num) {
    if (num == 0){
	return 0;
    }

    struct inode* p;
    struct inode* q = 0;

    for (p = inode_table; p<inode_table + NR_INODE; p++) {
	if (p -> i_cnt) {
	    /* not a free inode */
	    if (p -> i_num == num && p -> i_dev == dev) {
		p -> i_cnt++;
		return p;
	    }
	} else {
	    /* found a free inode */
	    if (!q)
		q = p;
	    /* 注意这里不能直接break，因为需要的inode可能在数组后面 */
	}
    }

    if (!q)
	panic("the inode table is full.\n");

    q -> i_dev = dev;
    q -> i_num = num;
    q -> i_cnt = 1;

    struct super_block *sb = get_super_block(dev);
    /* 读取硬盘上的inode */
    int blk_nr = 2 + sb -> nr_imap_sects + sb -> nr_smap_sects +
	(num - 1) / ((SECTOR_SIZE / INODE_SIZE));
    RD_SECT(dev, blk_nr);
    struct inode* pinode = (struct inode*)((u8*)fsbuf + ((num-1)%(SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
    q -> i_mode = pinode -> i_mode;
    q -> i_size = pinode -> i_size;
    q -> i_start_sect = pinode -> i_start_sect;
    q -> i_nr_sects = pinode -> i_nr_sects;

    return q;
}

/* decrease the reference nr of a slot in inode_table[]. */
PUBLIC void put_inode(struct inode* pinode) {
    assert(pinode -> i_cnt > 0);
    pinode -> i_cnt--;
}

/* write inode back to the disk */
PUBLIC void sync_inode(struct inode* p) {
    struct inode* pinode;
    struct super_block *sb = get_super_block(p -> i_dev);
    int blk_nr = 2 + sb -> nr_imap_sects + sb -> nr_smap_sects + ((p->i_num -1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p-> i_dev, blk_nr);
    pinode = (struct inode*)((u8*)fsbuf +
			     ((p->i_num -1)%(SECTOR_SIZE / INODE_SIZE))*INODE_SIZE);
    pinode -> i_mode = p -> i_mode;
    pinode -> i_size = p -> i_size;
    pinode -> i_start_sect = p -> i_start_sect;
    pinode -> i_nr_sects = p -> i_nr_sects;
    WR_SECT(p -> i_dev, blk_nr);
    
}

PRIVATE void init_fs() {
    /* 初始化全局变量 */
    for (int i=0; i<NR_FILE_DESC; i++) {
	memset(&f_desc_table[i], 0, sizeof(FILE));
    }

    for (int i=0; i<NR_INODE; i++) {
	memset(&inode_table[i], 0, sizeof(struct inode));
    }

    for (int i=0; i<NR_SUPER_BLOCK; i++) {
	super_block[i].sb_dev = NO_DEV;
    }
    
    MESSAGE driver_msg;

    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    /* 打开硬盘 */
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    mkfs();

    read_super_block(ROOT_DEV);
    struct super_block* sb = get_super_block(ROOT_DEV);
    /* printl("MAGIC: 0x%x", sb -> magic); */
    assert(sb -> magic == MAGIC_V1);

    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

PRIVATE void mkfs() {
    PART_INFO geo;
    MESSAGE driver_msg;

    driver_msg.type    = DEV_IOCTL;
    driver_msg.DEVICE  = MINOR(ROOT_DEV); /* 启动分区 */
    driver_msg.REQUEST = DIOCTL_GET_GEO;
    driver_msg.BUF     = &geo;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    printl("dev size: 0x%x sectors\n", geo.size);

    /* super block */
    struct super_block sb;
    sb.magic          = MAGIC_V1;
    sb.nr_inodes      = SECTOR_BITS;
    sb.nr_sects       = geo.size;
    sb.nr_imap_sects  = 1;	/* 最对有4096个inode */
    sb.nr_smap_sects  = sb.nr_sects / SECTOR_BITS + 1;
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
    sb.n_1st_sect     = 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
    sb.root_inode     = ROOT_INODE; /* root_inode = 1 */

    sb.inode_size     = INODE_SIZE;
    struct inode i_node;
    sb.inode_isize_off = (int)&i_node.i_size - (int)&i_node;
    sb.inode_start_off = (int)&i_node.i_start_sect - (int)&i_node;
    
    sb.dir_ent_size      = DIR_ENTRY_SIZE;
    struct dir_entry d_ent;
    sb.dir_ent_inode_off = (int)&d_ent.inode_nr - (int)&d_ent;
    sb.dir_ent_fname_off = (int)&d_ent.name - (int)&d_ent;

    memset(fsbuf, 0x90, SECTOR_SIZE);
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

    WR_SECT(ROOT_DEV, 1);

    /* inode map */
    memset(fsbuf, 0, SECTOR_SIZE);
    for (int i=0; i<NR_CONSOLE + 2; i++) {
	fsbuf[0] |= 1<<i;	/* 0: reserved, 1: ROOT_NODE, 2: dev_tty0, 3: dev_tty1, 4: dev_tty2 */
    }
    assert(fsbuf[0] == 0x1F);

    WR_SECT(ROOT_DEV, 2);

    /* sector map */
    memset(fsbuf, 0, SECTOR_SIZE);
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1; /* 2048 + 1,  1位保留 */

    int i=0;
    /* 根目录占用一个文件大小的扇区，而且第一个扇区保留 */
    /* 注意，sector_map的第一位对应的是sb.n_1st_sect */
    for (i=0; i<nr_sects/8; i++) {
	fsbuf[i] = 0xFF;
    }

    for (int j=0; j<nr_sects % 8; j++) {
	fsbuf[i] |= 1 << j;
    }

    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

    /* 其余的扇区均未使用 */
    memset(fsbuf, 0, SECTOR_SIZE);
    for (int i=1; i<sb.nr_smap_sects; i++) {
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects+i);
    }

    /* inode array */
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode *p_node = (struct inode*)fsbuf;
    p_node -> i_mode = I_DIRECTORY; /* 根目录 */
    p_node -> i_size = DIR_ENTRY_SIZE * 4; /* `.`, `dev_tty0~2` */
    p_node -> i_start_sect = sb.n_1st_sect;
    p_node -> i_nr_sects = NR_DEFAULT_FILE_SECTS;

    /* inode of dev+tty0~2 */
    /* 注意tty0~2不占用磁盘空间，它只有inode信息 */
    for (int i=0; i<NR_CONSOLE; i++) {
	p_node = (struct inode*)(fsbuf + (INODE_SIZE * (i+1)));
	p_node -> i_mode = I_CHAR_SPECIAL;
	p_node -> i_size = 0;
	p_node -> i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
	p_node -> i_nr_sects = 0;
    }

    WR_SECT(ROOT_DEV, 2 + sb.nr_smap_sects + sb.nr_imap_sects);

    /* `/` */
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry* p_dir = (struct dir_entry*)fsbuf;
    p_dir -> inode_nr = ROOT_INODE;
    strcpy(p_dir -> name, ".");

    for (int i=0; i<NR_CONSOLE; i++) {
	p_dir++;
	p_dir -> inode_nr = i+2;
	sprintf(p_dir -> name, "dev_tty%d", i);
    }

    WR_SECT(ROOT_DEV, sb.n_1st_sect);

    printl("devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n        inodes:0x%x00, 1st_sector:0x%x00\n", geo.base*2,
	   (geo.base + 1)*2,
	   (geo.base + 2)*2,
	   (geo.base + 2 + sb.nr_imap_sects) * 2,
	   (geo.base + 2 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
	   (geo.base + sb.n_1st_sect) * 2);
}

PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf) {
    MESSAGE driver_msg;

    driver_msg.type     = io_type;	/* DEV_READ or DEV_WRITE */
    driver_msg.DEVICE   = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF      = buf;
    driver_msg.CNT      = bytes;
    driver_msg.PROC_NR  = proc_nr;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    return 0;
}


PRIVATE void read_super_block(int dev) {
    MESSAGE driver_msg;

    driver_msg.type     = DEV_READ;
    driver_msg.DEVICE   = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE;
    driver_msg.BUF      = fsbuf;
    driver_msg.CNT      = SECTOR_SIZE;
    driver_msg.PROC_NR  = TASK_FS;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
    int i;
    for (i=0; i<NR_SUPER_BLOCK; i++) {
	if (super_block[i].sb_dev == NO_DEV)
	    break;
    }

    if (i == NR_SUPER_BLOCK)
	panic("super block table is full.\n");

    assert(i == 0);		/* one 1 hard drive in our system */
    struct super_block *sb = (struct super_block*)fsbuf;
    
    super_block[i] = *sb;
    super_block[i].sb_dev = dev;

    /* printl("NR_SECTS: %d, NR_IMAP_SECTS: %d, ROOT_INODE: %d\n", sb -> nr_sects, */
    /* 	    sb -> nr_imap_sects, sb -> root_inode); */
      
}
