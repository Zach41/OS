/* #include "type.h" */
/* #include "const.h" */
/* #include "hd.h" */
/* #include "fs.h" */
/* #include "protect.h" */
/* #include "console.h" */
/* #include "tty.h" */
/* #include "proc.h" */
/* #include "proto.h" */
/* #include "global.h" */
#include "headers.h"

PRIVATE struct inode* create_file(char* path, int flags);

PRIVATE int alloc_imap_bit(int dev);
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char *filename);

PUBLIC int open(const char* pathname, int flags) {
    MESSAGE msg;

    msg.type = OPEN;

    msg.PATHNAME = (void*)pathname;
    msg.FLAGS    = flags;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);

    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}

PUBLIC int do_open() {
    return 0;
}

PRIVATE int alloc_imap_bit(int dev) {
    int inode_nr = 0;

    int i, j, k;
    int imap_blk0_nr = 2;
    struct super_block *sb = get_super_block(dev);

    for (i=0; i<sb.nr_imap_sects; i++) {
	RD_SECT(dev, imap_blk0_nr+i);

	for (j=0; j<SECTOR_SIZE; j++) {
	    if (fsbuf[j] == 0xFF)
		continue;
	    for (k=0; (fsbuf[j] & (1<<k))!=0; k++)
		;
	    inode_nr = (i*SECTOR_SIZE + j)*8 + k;
	    fsbuf[j] |= (1<<k);

	    WR_SECT(dev, imap_blk0_nr+i);

	    return inode_nr;
	}
    }

    panic("Inode map is full.\n");

    return 0;
}

PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc) {
    int i, j, k;
    struct super_block* sb = get_super_block(dev);

    int smap_blk0_nr = 2 + sb.nr_imap_sects;
    int free_sect_nr = 0;

    for (i=0; i<sb.nr_smap_sects; i++) {
	RD_SECT(dev, smap_blk0_nr + i);

	for (j=0; j<SECTOR_SIZE && nr_sects_to_alloc>0; j++) {
	    k = 0;
	    if(!free_sect_nr) {
		/* 找到第一个空闲的扇区 */
		/* 由于文件最大长度固定，并不需要考虑空洞，找到就可以了 */
		if (fsbuf[j] == 0xFF)
		    continue;
		for(; (fsbuf[j] & (1<<k)) != 0; k++)
		    ;
		free_sect_nr = (i*SECTOR_SIZE + j) * 8 + k - 1 + sb.n_1st_sect;
	    }

	    for(; k<8; k++) {
		assert(((fsbuf[j] >> k) & 1) == 0);
		fsbuf[j] |= 1<<k;
		if (--nr_sects_to_alloc == 0)
		    break;
	    }
	}
	if (free_sect_nr)
	    WR_SECT(dev, smap_blk0_nr+i);
	if (nr_sects_to_alloc)
	    break;
    }

    if (nr_sects_to_alloc != 0) {
	panic("Don't have enough space.\n");
    }

    return free_sect_nr;
}

PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect) {
    struct inode *p_node = get_inode(dev, inode_nr);

    p_node -> i_mode       = I_REGULAR;
    p_node -> i_size       = 0;
    p_node -> i_start_sect = start_sect;
    p_node -> i_nr_sects   = NR_DEFAULT_FILE_SECTS;

    /* in memory items */
    p_node -> i_dev = dev;
    p_node -> i_cnt = 1;
    p_node -> i_num = inode_nr;

    /* write to the inode array */
    sync_inode(p_node);

    return p_node;
}

/* 向目录写一个新的dir_entry */
/* dir_inode: root_node */
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename) {
    int dir_blk0_nr = dir_inode -> i_start_sect;
    int nr_dir_blks = (dir_inode -> i_size + SECTOR_SIZE) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode -> i_size / DIR_ENTRY_SIZE;

    int i, j;
    int m = 0;
    struct dir_entry* p_dir_entry;
    struct dir_entry* new_entry = 0;

    for(i=0; i<nr_dir_blks; i++) {
	RD_SECT(dir_inode -> i_dev, dir_blk0_nr + i);
	p_dir_entry = (struct dir_entry*)fsbuf;

	for (j = 0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
	    if (++m > nr_dir_entries)
		break;

	    if (pde -> inode_nr == 0) {
		/* 之前被删除的文件留下来的entry */
		/* 如果文件删除了，dir_inode -> i_size不会改变 */
		new_entry = pde;
		break;
	    }
	}
	if (m > nr_dir_entries || new_entry)
	    break;
    }
    if (!new_entry) {
	new_entry = pde;
	dir_inode -> i_size += DIR_ENTRY_SIZE;
    }

    new_entry -> inode_nr = inode_nr;
    strcpy(new_entry -> name, filename);

    WR_SECT(dir_inode -> i_dev, dir_blk0_nr+i);

    sync_inode(dir_inode);
}
