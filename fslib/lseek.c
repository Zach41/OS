#include "headers.h"
#include "stdio.h"

PUBLIC int do_lseek() {
    off_t offset = fs_msg.OFFSET;
    int   whence = fs_msg.WHENCE;
    int   fd     = fs_msg.FD;

    if (pcaller -> filp[fd] == 0) {
	/* 进程没有打开文件 */
	printl("FS::do_lseek() failed, process didn't open the file.\n");
	return -1;
    }
    if ((pcaller -> filp[fd] -> fd_mode & O_RDWR) == 0) {
	/* 没有读写权限 */
	printl("FS::do_lseek() failed, process didn't have read/write rights to file.\n");
	return -1;
    }

    struct inode* pin = pcaller -> filp[fd] -> fd_inode;

    if (pin) {
	switch(whence) {
	case SEEK_SET:
	    if (offset < 0) {
		return -1;
	    }
	    offset = min(offset, pin -> i_size);
	    pcaller -> filp[fd] -> fd_pos = offset;
	    break;
	case SEEK_END:
	    if (offset + pin -> i_size - 1 < 0) {
		return -1;
	    }
	    offset = min(offset+ pin -> i_size - 1, pin -> i_nr_sects * SECTOR_SIZE - 1);
	    pcaller -> filp[fd] -> fd_pos = offset;
	    break;
	case SEEK_CUR:
	    if (offset + pcaller -> filp[fd] -> fd_pos < 0) {
		return -1;
	    }

	    offset = min(offset + pcaller -> filp[fd] -> fd_pos, pin -> i_nr_sects * SECTOR_SIZE);
	    pcaller -> filp[fd] -> fd_pos = offset;
	    break;
	default:
	    printl("FS::do_lseek(), unknown seeking command.\n");
	    return -1;
	}

	return offset;
    }
    return -1;
}

PUBLIC int lseek(int fd, off_t offset, int whence) {
    MESSAGE msg;

    msg.type   = LSEEK;
    msg.OFFSET = offset;
    msg.WHENCE = whence;
    msg.FD     = fd;
    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}
