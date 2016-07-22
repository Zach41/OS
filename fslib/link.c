#include "headers.h"
#include "stdio.h"

PUBLIC int do_unlink() {
    char pathname[MAX_PATH];

    int name_len = fs_msg.NAME_LEN;
    int src      = fs_msg.source;

    memcpy((void*)va2la(TASK_FS, pathname),
	   (void*)va2la(src, fs_msg.PATHNAME),
	   name_len);

    pathname[name_len] = 0;

    if (strcmp(pathname, "/")) {
	printl("FS: do_unlink():: cannot unlink root directory\n");
	return -1;
    }

    int inode_nr = search_file(pathname);
    if (inode_nr == INVALID_INODE) {
	/* 文件不存在　*/
	printl("FS:: do_unlink():: search file failed, invalid inode: %s\n", pathname);
	return -1;
    }
    char filename[MAX_PATH];
    struct inode* dir_inode;

    if (strip_path(filename, pathname, &dir_inode) != 0)
	return -1;

    struct inode* pin = get_inode(dir_inode -> i_dev, inode_nr);

    if (pin -> i_mode != I_REGULAR) {
	/* 不能删除字符设备 */
	printl("FS::do_unlink():: cannot remove file %s, it's not a regular file.\n", pathname);
	return -1;
    }

    if (pin -> i_cnt > 1) {
	/* 不能删除正在被使用的文件 */
	printl("FS::do_unlink():: cannot remove file %s, it's currently used by another process.\n", pathname);
	return -1;
    }

    struct super_block *sb = get_super_block(dir_inode -> i_dev);

    /* free imap bit */
    int byte_idx = inode_nr / 8;
    int bit_idx  = inode_nr % 8;

    RD_SECT(pin -> i_dev, 2);
    assert((fsbuf[byte_idx % SECTOR_SIZE] & (1<<bit_idx)));
    fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << bit_idx);
    WR_SECT(pin -> i_dev, 2);

    /* free bits in sector map */
    bit_idx  = pin -> i_start_sect - sb -> n_1st_sect + 1;
    byte_idx = bit_idx / 8;

    int bits_left = pin -> i_nr_sects;
    int byte_cnt  = (bits_left - (8 - (bit_idx % 8))) / 8; /* 第二个字节到最后第二个字节 */

    int smap_blk0_nr = 2 + sb -> nr_imap_sects + byte_idx / SECTOR_SIZE;
    
    RD_SECT(pin -> i_dev, smap_blk0_nr);
    for(int i=bit_idx%8; i<8 && bits_left > 0; i++, bits_left--) {
	/* 置位第一个字节 */
	assert(fsbuf[byte_idx % SECTOR_SIZE] & (1<<i));
	fsbuf[byte_idx % SECTOR_SIZE] &= ~(1<<i);
    }

    /* 置位第二个到最后第二个字节 */
    int i=(byte_idx % SECTOR_SIZE + 1);
    
    for (int k=0; k<byte_cnt; k++, i++, bits_left-= 8) {
	if (i == SECTOR_SIZE) {
	    /* 遇到边界 */
	    WR_SECT(pin -> i_dev, smap_blk0_nr);
	    RD_SECT(pin -> i_dev, ++smap_blk0_nr);
	}
	assert(fsbuf[i] == 0xFF);
	fsbuf[i] = 0;
    }

    if (i == SECTOR_SIZE) {
	WR_SECT(pin -> i_dev, smap_blk0_nr);
	RD_SECT(pin -> i_dev, ++smap_blk0_nr);
	i = 0;
    }
    /* 置位最后一个字节 */
    fsbuf[i] &= (~0) << bits_left;
    WR_SECT(pin -> i_dev, smap_blk0_nr);

    /* free inode */
    pin -> i_mode = 0;
    pin -> i_size = 0;
    pin -> i_start_sect = 0;
    pin -> i_nr_sects = 0;
    sync_inode(pin);
    /* 将引用减一 */
    put_inode(pin);

    /* free dir entry in directory */
    int dir_blk0_nr = dir_inode -> i_start_sect;
    int nr_dir_blks = (dir_inode -> i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode -> i_size / DIR_ENTRY_SIZE;

    int m = 0;
    struct dir_entry* pde = 0;
    int flag = 0;
    int dir_size = 0;

    for (int i=0; i<nr_dir_blks; i++) {
	RD_SECT(dir_inode -> i_dev, dir_blk0_nr+i);

	pde = (struct dir_entry*)fsbuf;

	for (int j=0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
	    if (++m > nr_dir_entries)
		break;

	    if (pde -> inode_nr == inode_nr) {
		memset(pde, 0, DIR_ENTRY_SIZE);
		WR_SECT(dir_inode -> i_dev, dir_blk0_nr + i);
		flag = 1;
		break;
	    }

	    if (pde -> inode_nr != inode_nr)
		dir_size += DIR_ENTRY_SIZE;
	}

	if (m > nr_dir_entries || flag) {
	    break;
	}
    }

    if (m == nr_dir_entries) {
	/* 恰好是最后一个文件 */
	dir_inode -> i_size = dir_size;
	sync_inode(dir_inode);
    }

    return 0;
}


PUBLIC int unlink(const char* pathname) {
    MESSAGE msg;
    msg.PATHNAME = (void*)pathname;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
