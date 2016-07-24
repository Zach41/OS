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

/* 更精确的延迟函数 */
/* 延迟milli_sec毫秒 */
PUBLIC void milli_delay(int milli_sec) {
    int ticks = get_ticks();
    while (((get_ticks() - ticks) * 1000 / HZ) < milli_sec) {
	;
    }
}

PUBLIC int strlen(const char* str) {
    int cnt = 0;
    const char* p = str;
    
    while (*p) {
	cnt++;
	p++;
    }

    return cnt;
}

PUBLIC int strcmp(const char* s1, const char* s2) {
    const char* p1 = s1;
    const char* p2 = s2;
    if (p1 == 0 || p2 == 0) {
	return (p1 - p2);
    }

    while (*p1 && *p2) {
	if (*p1 != *p2)
	    return -1;
	p1++;
	p2++;
    }

    return (*p1 - *p2);
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
