#include "headers.h"
#include "stdio.h"

PUBLIC int do_stat() {
    char pathname[MAX_PATH];
    char filename[MAX_PATH];

    int name_len = fs_msg.NAME_LEN;
    int src      = fs_msg.source;
    
    memcpy((void*)va2la(TASK_FS, pathname),
	   (void*)va2la(src, fs_msg.PATHNAME),
	   name_len);
    pathname[name_len] = 0;

    int inode_nr = search_file(pathname);

    if (inode_nr == INVALID_INODE) {
	printl("FS::do_stat() failed, search file %s return INVALID INODE.\n", pathname);
	return -1;
    }

    struct inode* pin = 0;
    struct inode* dir_inode = 0;
    if(strip_path(filename, pathname, &dir_inode) != 0) {
	assert(0);
    }

    pin = get_inode(dir_inode -> i_dev, inode_nr);

    struct stat s;

    s.st_dev  = pin -> i_dev;
    s.st_ino  = pin -> i_num;
    s.st_mode = pin -> i_mode;
    s.st_rdev = pin -> i_mode == I_CHAR_SPECIAL ? pin -> i_start_sect : NO_DEV;
    s.st_size = pin -> i_size;

    put_inode(pin);

    memcpy((void*)va2la(src, fs_msg.BUF),
	   (void*)va2la(TASK_FS, &s),
	   sizeof(struct stat));

    return 0;
}
