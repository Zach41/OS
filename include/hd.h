#ifndef _ZACH_HD_H_
#define _ZACH_HD_H_

/* Partition Entry Struct */
/* struct part_ent { */
/*     u8    boot_ind; */
/* } */

/* I/O ports used by hard disk controllers */
#define    REG_DATA        0x1F0
#define    REG_FEATURES    0x1F1
#define    REG_ERROR       REG_FEATURES
#define    REG_NSECTOR     0x1F2
#define    REG_LBA_LOW     0x1F3
#define    REG_LBA_MID     0x1F4
#define    REG_LBA_HIGH    0x1F5
#define    REG_DEVICE      0x1F6
#define    REG_STATUS      0x1F7
#define    REG_CMD         REG_STATUS

#define    REG_DEV_CTRL    0x3F6 /* Control Block Register */
#define    REG_ALT_STATUS  REG_DEV_CTRL

#define    REG_DRV_ADDR    0x3F7 /* Drive Address */

/* status of hard disk */
#define STATUS_BSY  0x80
#define STATUS_DRDY 0x40
#define STATUS_DFSE 0x20	/* Device Fault / Stream Error */
#define STATUS_DSC  0x10
#define STATUS_DRQ  0x08	/* Data request, ready to transfer data */
#define STATUS_CORR 0x04	/* obsolete */
#define STATUS_IDX  0x02	/* obsolete */
#define STATUS_ERR  0x01	/* an error occurred */

/* Variables about drive */
#define MAX_DRIVES            2
#define NR_PART_PER_DRIVE     4
#define NR_SUB_PER_PART       16
#define NR_SUB_PER_DRIVE      (NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define NR_PRIM_PER_DRIVE     (NR_PART_PER_DRIVE + 1)
#define MAX_PRIM              (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)
#define MAX_SUBPARTITIONS     (NR_SUB_PER_DRIVE * MAX_DRIVES)

/* hard disk command struct */
typedef struct hd_cmd {
    u8    features;
    u8    count;
    u8    lba_low;
    u8    lba_mid;
    u8    lba_high;
    u8    device;
    u8    command;
}HD_CMD;

/* partition information */
typedef struct part_info {
    u32    base;
    u32    size;
}PART_INFO;

/* one entry per drive */
typedef struct hd_info {
    int                 open_cnt;
    struct part_info    primary[NR_PRIM_PER_DRIVE];
    struct part_info    logical[NR_SUB_PER_DRIVE];
}HD_INFO;

typedef struct part_ent {
    /* P339 table 9.3 */
    u8 boot_ind;
    u8 start_head;
    u8 start_sector;
    u8 start_cyl;
    u8 sys_id;
    u8 end_head;
    u8 end_sector;
    u8 end_cyl;
    u32 start_sect;
    u32 nr_sects;
}PART_ENT;

/* Definitions */
#define HD_TIMEOUT             10000
#define ATA_IDENTIFY           0xEC
#define ATA_READ               0x20
#define ATA_WRITE              0x30
#define PARTITION_TABLE_OFFSET 0x1BE

/* Partition Type */
#define ORANGES_PART 0x99
#define NO_PART      0x00
#define EXT_PART     0x05


#define SECTOR_SIZE          512
#define SECTOR_BITS          (SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT    9

/* Macros */
#define MAKE_DEVICE_REG(lba, drv, lba_highest) (((lba) << 6) | ((drv) << 4) | \
						((lba_highest) | 0xA0))

#endif
