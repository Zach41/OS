#ifndef _ZACH_FS_H_
#define _ZACH_FS_H_

struct dev_drv_map {
    int driver_nr;
};

#define MAGIC_V1    0x111	/* magic number to indicate file system */

struct super_block {
    u32    magic;		/* magic number */
    u32    nr_inodes;		/* number of inodes */
    u32    nr_sects;		/* number of sectors */
    u32    nr_imap_sects;	/* how many sectors inode-map costs */
    u32    nr_smap_sects;	/* how many sectors sector-map costs */
    u32    n_1st_sect;		/* number of first data secotr */
    u32    nr_inode_sects;	/* how many sectors inode costs */
    u32    root_inode;		/* inode number of root dir*/

    /* 参考inode和dir_entry结构 */
    u32    inode_size;
    u32    inode_isize_off;
    u32    inode_start_off;
    u32    dir_ent_size;	/* dir entry size */
    u32    dir_ent_inode_off;
    u32    dir_ent_fname_off;

    int    sb_dev;		/* super block's device */
};

#define SUPER_BLOCK_SIZE    56

struct inode {
    u32    i_mode;
    u32    i_size;
    u32    i_start_sect;
    u32    i_nr_sects;		/* how many sectors this file costs */
    u8     _unused[16];		/* just for alignment */

    /* in memory items */
    int    i_dev;
    int    i_cnt;		/* how many procs share this file */
    int    i_num;		/* inode number */
};

#define INODE_SIZE          32	/* in device size */

#define MAX_FILENAME_LEN    12

struct dir_entry {
    int  inode_nr;
    char name[MAX_FILENAME_LEN];
};

#define DIR_ENTRY_SIZE    sizeof(struct dir_entry)

#define WR_SECT(dev, sect_nr) rw_sector(DEV_WRITE, dev, (sect_nr) * SECTOR_SIZE, \
					SECTOR_SIZE, TASK_FS, fsbuf);

#define RD_SECT(dev, sect_nr) rw_sector(DEV_READ, dev, (sect_nr) * SECTOR_SIZE, \
					SECTOR_SIZE, TASK_FS, fsbuf);

#define ROOT_INODE    1
#define INVALID_INODE 0

/* file mode */
#define I_TYPE_MASK        0170000
#define I_DIRECTORY        0040000
#define I_REGULAR          0100000
#define I_BLOCK_SPECIAL    0060000
#define I_CHAR_SPECIAL     0020000
#define I_NAMED_PIPE       0010000

#define NR_DEFAULT_FILE_SECTS    2048 /* file's max size is 1MB */

typedef struct file_desc {
    int           fd_mode;		/* R or W */
    int           fd_pos;		/* current position */
    struct inode* fd_inode;	        /* pointer to inode */
}FILE;

#endif
