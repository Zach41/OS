#include "headers.h"

/* 由于文件系统是扁平的，最后的结果是filename是文件名，ppinode是&root_inode */
/* filename, ppinode are both input */
PUBLIC int strip_path(char* filename, const char* pathname, struct inode** ppinode) {
    const char* s = pathname;
    char* t       = filename;

    if (s == 0)
	return -1;
    if (*s == '/')
	s++;
    while(*s) {
	if (*s == '/')
	    return -1;
	*t++ = *s++;
	if (t - filename >= MAX_FILENAME_LEN) /* truncate */
	    break;
    }
    *t = 0;
    *ppinode = root_inode;

    return 0;
}

/* @return inode number if found, otherwise 0 */
PUBLIC int search_file(char* path) {
    char filename[MAX_PATH];
    memset(filename, 0, MAX_FILENAME_LEN);

    struct inode* dir_inode;

    if (strip_path(filename, path, &dir_inode) != 0)
	return 0;

    if (filename == 0) {
	/* root dir */
	return dir_inode -> i_num;
    }
    /* printl("FILENAME: %s\n", filename); */
    /* dump_inode(dir_inode); */
    int dir_blk0_nr = dir_inode -> i_start_sect;
    int nr_dir_blks = (dir_inode -> i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode -> i_size / DIR_ENTRY_SIZE; /* including unused slots */

    int m = 0;
    struct dir_entry *pde;

    for (int i=0; i<nr_dir_blks; i++) {
	RD_SECT(dir_inode -> i_dev, dir_blk0_nr+i);
	pde = (struct dir_entry*)fsbuf;
	for(int j=0; j<SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
	    if (memcmp(filename, pde -> name, MAX_FILENAME_LEN) == 0)
		return pde -> inode_nr;
	    if (++m > nr_dir_entries)
		break;
	}
	if (m > nr_dir_entries)
	    break;
    }
    return 0;
}

