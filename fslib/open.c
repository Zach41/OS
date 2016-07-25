#include "headers.h"
#include "stdio.h"

PRIVATE struct inode* create_file(char* path, int flags);

PRIVATE int alloc_imap_bit(int dev);
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char *filename);

PUBLIC int do_open() {
    int fd = -1;
    char pathname[MAX_PATH];

    int flags = fs_msg.FLAGS;
    int name_len = fs_msg.NAME_LEN;
    int src = fs_msg.source;
    assert(name_len < MAX_PATH);

    memcpy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
    pathname[name_len] = 0;

    int i;
    for(i=0; i<NR_FILES; i++) {
	/* a empty filp item */
	if (pcaller -> filp[i] == 0)
	    break;
    }
    fd = i;
    /* printl("PATHNAME: %s, NAME_LEN: %d, FD: %d\n", pathname, name_len, fd); */
    if (fd <0 || fd >= NR_FILES)
	panic("file[] is full. PID: %d", proc2pid(pcaller));

    for(i=0; i<NR_FILE_DESC; i++) {
	/* a empty file descriptor */
	if (f_desc_table[i].fd_inode == 0)
	    break;
    }
    if (i >= NR_FILE_DESC)
	panic("f_desc_table[] is full. PID: %d", proc2pid(pcaller));

    // FIXME: search_file is wrong
    int inode_nr = search_file(pathname);
    
    struct inode* pin = 0;
    if (flags & O_CREAT) {
	if (inode_nr) {
	    /* file exists */
	    printl("file exists\n");
	    return -1;
	} else {
	    pin = create_file(pathname, flags);
	    /* dump_inode(pin); */
	}
    } else {
	assert(flags & O_RDWR);

	char filename[MAX_PATH];
	struct inode* dir_inode;
	if (strip_path(filename, pathname, &dir_inode) != 0)
	    return -1;
	pin = get_inode(dir_inode -> i_dev, inode_nr);
    }

    if (pin) {
	pcaller -> filp[fd] = &f_desc_table[i];

	f_desc_table[i].fd_inode = pin;

	f_desc_table[i].fd_mode  = flags;

	f_desc_table[i].fd_pos   = 0;
	f_desc_table[i].fd_cnt   = 1;

	int imode = pin -> i_mode & I_TYPE_MASK;

	if (imode == I_CHAR_SPECIAL) {
	    /* tty */
	    MESSAGE msg;
	    msg.type = DEV_OPEN;

	    int dev = pin -> i_start_sect;
	    assert(MAJOR(dev) == 4);
	    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

	    msg.DEVICE = MINOR(dev);
	    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &msg);
	} else if (imode == I_DIRECTORY) {
	    assert(pin -> i_num == ROOT_INODE);
	} else {
	    assert(pin -> i_mode == I_REGULAR);
	}
    } else {
	return -1;
    }

    return fd;
}

PUBLIC int do_close() {
    int fd = fs_msg.FD;
    put_inode(pcaller -> filp[fd] -> fd_inode);
    pcaller -> filp[fd] -> fd_inode = 0;
    pcaller -> filp[fd] = 0;

    return 0;
}

PRIVATE int alloc_imap_bit(int dev) {
    int inode_nr = 0;

    int i, j, k;
    int imap_blk0_nr = 2;
    assert(dev == ROOT_DEV);
    struct super_block *sb = get_super_block(dev);

    
    for (i=0; i<sb -> nr_imap_sects; i++) {
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
    assert(dev == ROOT_DEV);
    assert(nr_sects_to_alloc == NR_DEFAULT_FILE_SECTS);
    struct super_block* sb = get_super_block(dev);
    assert(sb -> nr_smap_sects);
    int smap_blk0_nr = 2 + sb -> nr_imap_sects;
    int free_sect_nr = 0;
    
    for (i=0; i<sb -> nr_smap_sects; i++) {
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
		free_sect_nr = (i*SECTOR_SIZE + j) * 8 + k - 1 + sb -> n_1st_sect;
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
	if (nr_sects_to_alloc == 0)
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

	for (j = 0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, p_dir_entry++) {
	    if (++m > nr_dir_entries)
		break;

	    if (p_dir_entry -> inode_nr == 0) {
		/* 之前被删除的文件留下来的entry */
		/* 如果文件删除了，dir_inode -> i_size不会改变 */
		new_entry = p_dir_entry;
		break;
	    }
	}
	if (m > nr_dir_entries || new_entry)
	    break;
    }
    if (!new_entry) {
	new_entry = p_dir_entry;
	dir_inode -> i_size += DIR_ENTRY_SIZE;
    }

    new_entry -> inode_nr = inode_nr;
    strcpy(new_entry -> name, filename);

    WR_SECT(dir_inode -> i_dev, dir_blk0_nr+i);

    sync_inode(dir_inode);
}

PRIVATE struct inode* create_file(char* path, int flags) {
    char filename[MAX_PATH];
    struct inode* dir_inode;

    if (strip_path(filename, path, &dir_inode) != 0)
	return 0;
    int inode_nr = alloc_imap_bit(dir_inode -> i_dev);

    int free_sect_nr = alloc_smap_bit(dir_inode -> i_dev, NR_DEFAULT_FILE_SECTS);

    struct inode* nd = new_inode(dir_inode -> i_dev, inode_nr, free_sect_nr);

    new_dir_entry(dir_inode, nd -> i_num, filename);

    return nd;
}
