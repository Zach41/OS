#include "headers.h"
#include "stdio.h"

PUBLIC int do_rdwt() {
    int fd    = fs_msg.FD;
    int len   = fs_msg.CNT;
    void* buf = fs_msg.BUF;

    /* printf("FD: %d, LEN: %d, STR: %s\n", fd, len, (char*)buf); */
    int src   = fs_msg.source;

    if ((pcaller -> filp[fd] -> fd_mode & O_RDWR) == 0)
	return -1;

    int pos = pcaller -> filp[fd] -> fd_pos;

    struct inode* pin = pcaller -> filp[fd] -> fd_inode;

    /* dump_inode(pin); */


    int imode = pin -> i_mode & I_TYPE_MASK;

    if (imode == I_CHAR_SPECIAL) {
	/* 字符串设备 */
	int type = fs_msg.type == READ ? DEV_READ : DEV_WRITE;
	fs_msg.type = type;

	int dev = pin -> i_start_sect;
	assert(MAJOR(dev) == 4); /* 字符设备的驱动程序号是4 */

	fs_msg.DEVICE  = MINOR(dev);
	fs_msg.BUF     = buf;
	fs_msg.CNT     = len;
	fs_msg.PROC_NR = src;

	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

	send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &fs_msg);
	assert(fs_msg.CNT == len);

	return fs_msg.CNT;
	
    } else {
	assert(pin -> i_mode == I_REGULAR || pin -> i_mode == I_DIRECTORY);
	assert(fs_msg.type == READ || fs_msg.type == WRITE);

	int pos_end;
	if (fs_msg.type == READ)
	    pos_end = min(pos + len, pin -> i_size);
	else
	    pos_end = min(pos + len, pin -> i_nr_sects * SECTOR_SIZE);
	
	int off = pos % SECTOR_SIZE;
	int rw_sect_min = pin -> i_start_sect + pos /  SECTOR_SIZE;
	int rw_sect_max = pin -> i_start_sect + pos_end / SECTOR_SIZE;

	int chunk = min(rw_sect_max - rw_sect_min + 1, FSBUF_SIZE / SECTOR_SIZE);

	/* assert(chunk == 1); */
	
	int bytes_rw   = 0;
	int bytes_left = len;
	for (int i=rw_sect_min; i<=rw_sect_max; i += chunk) {
	    int bytes = min(bytes_left, chunk*SECTOR_SIZE - off);

	    rw_sector(DEV_READ, pin -> i_dev, i*SECTOR_SIZE, chunk*SECTOR_SIZE, TASK_FS, fsbuf);
	    if(fs_msg.type == READ) {
		/* 读操作 */
		memcpy((void*)va2la(src, buf+bytes_rw),
		       (void*)va2la(TASK_FS, fsbuf+off),
		       bytes);
	    } else {
		/* 写操作 */
		memcpy((void*)va2la(TASK_FS, fsbuf+off),
		       (void*)va2la(src, buf+bytes_rw),
		       bytes);
		rw_sector(DEV_WRITE, pin -> i_dev, i*SECTOR_SIZE, chunk*SECTOR_SIZE, TASK_FS, fsbuf);
	    }

	    off = 0;
	    bytes_rw   += bytes;
	    bytes_left -= bytes;
	    pcaller -> filp[fd] -> fd_pos += bytes;
	}

	if (pcaller -> filp[fd] -> fd_pos > pin -> i_size) {
	    pin -> i_size = pcaller -> filp[fd] -> fd_pos;
	    sync_inode(pin);
	}

	return bytes_rw;
    }
}

