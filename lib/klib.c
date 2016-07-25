#include "headers.h"
#include "elf.h"

PUBLIC char *itoa(char* str, int num) {
    char *p = str;
    char ch;
    int flag = 0;

    *p++ = '0';
    *p++ = 'x';
    if (num == 0) {
	*p++ = '0';
    } else {
	for (int i=28; i>=0; i-=4) {
	    ch = (num >> i) & 0xF;
	    if (flag || ch > 0) {
		flag = 1;	/* 找到第一个不为0的数 */
		ch += '0';
		if (ch > '9') {
		    ch += 7;	/* ord('9') = 57, ord('A') = 65 */
		}
		*p++ = ch;
	    }
	}
    }
    *p = 0;
    return str;
}

PUBLIC char* itod(char* str, int num) {
    char *p = str;
    int  cnt = 0;

    while (num) {
	*p++ = num % 10 + '0';
	num /= 10;
	cnt += 1;
    }
    *p++ = 0;
    for (int i=1; i <= cnt/2; i++) {
	char ch      = str[i-1];
	str[i-1]     = str[cnt - i];
	str[cnt - i] = ch;
    }

    return str;
     
}

PUBLIC void disp_int(int num) {
    char input[16];
    itoa(input, num);
    disp_str(input);
}

PUBLIC void delay(int time) {
    for (int k=0; k<time; k++) {
	for (int i=0; i<1000; i++) {
	    for (int j=0; j<100; j++) {
		;
	    }
	}
    }
}

PUBLIC void dump_inode(struct inode* p_inode) {
    printl("Mode: %x\nSize: %d\nStart_Sect:%x\nNR_Sects:%d\n",
	   p_inode -> i_mode,
	   p_inode -> i_size,
	   p_inode -> i_start_sect,
	   p_inode -> i_nr_sects);
}

PUBLIC void get_boot_params(BOOT_PARAMS* p_bp) {
    int* p = (int*)BOOT_PARAM_ADDR;

    assert(p[BI_MAG] == BOOT_PARAM_MAGIC);

    p_bp -> memsize = p[BI_MEM_SIZE];
    p_bp -> kernel_file = (unsigned char*)(p[BI_KERNEL_FILE]);
    assert(memcmp(p_bp -> kernel_file, ELFMAG, SELFMAG) == 0);
}

PUBLIC int get_kernel_map(unsigned int *base, unsigned int* limit) {
    BOOT_PARAMS bp;
    get_boot_params(&bp);

    Elf32_Ehdr* e_header = (Elf32_Ehdr*)(bp.kernel_file);

    if (memcmp(e_header -> e_ident, ELFMAG, SELFMAG) != 0)
	return -1;

    *base = ~0;
    unsigned int l = 0;

    for (int i=0; i<e_header -> e_shnum; i++) {
	Elf32_Shdr* section_header = (Elf32_Shdr*)(bp.kernel_file
						   + e_header -> e_shoff
						   + i * e_header -> e_shentsize);
	if (section_header -> sh_flags & SHF_ALLOC) {
	    int bottom = section_header -> sh_addr;
	    int top    = section_header -> sh_size + section_header -> sh_addr;

	    if (*base > bottom)
		*base = bottom;
	    if (top > l)
		l = top;
	}
	
    }
    assert(*base < l);

    *limit = l - *base - 1;

    return 0;
}

struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

PUBLIC void untar(const char* filename) {
    printf("extract `%s`\n", filename);
    int fd = open(filename, O_RDWR);
    assert(fd != -1);

    char buf[SECTOR_SIZE * 16];
    int  chunk = sizeof(buf);

    while (TRUE) {
	/* 读tar文件头 */
	read(fd, buf, SECTOR_SIZE);

	if (buf[0] == 0)
	    break;

	struct posix_tar_header *header = (struct posix_tar_header*)buf;

	char* p = header -> size;
	int file_len = 0;
	while (*p)
	    file_len = file_len * 8 + (*p++ - '0');
	int bytes_left = file_len;
	int fdout = open(header -> name, O_CREAT | O_RDWR);
	if (fdout == -1) {
	    printf("failed to extract file: %s. Aborted.\n", header -> name);
	    return;
	}
	printf("    %s (%d bytes)\n", header -> name, file_len);
	while (bytes_left) {
	    int iobytes = min(bytes_left, chunk);
	    read(fd, buf, ((iobytes - 1) / SECTOR_SIZE + 1)*SECTOR_SIZE);
	    write(fdout, buf, iobytes);
	    bytes_left -= iobytes;
	}
	close(fdout);
    }
    close(fd);

    printf("Done!\n");
}

