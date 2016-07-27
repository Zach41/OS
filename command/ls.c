#include "stdio.h"
#include "type.h"
#include "fs.h"

int main2(int argc, char* argv[]) {
    printf(" i-node    file name\n");
    printf("-------    -------------\n");
    
    struct stat s;
    char* root = "/";
    stat(root, &s);

    /* assert(s.st_ino == 1); */

    char buf[512];
    int chunk = sizeof(buf) > s.st_size ? s.st_size : sizeof(buf);
    int bytes_left = s.st_size;
    
    int fd = open(root, O_RDWR);
    int bytes;
    int flag  = 0;
    while (bytes_left) {
	bytes = read(fd, buf, chunk);
	assert(bytes > 0);
	struct dir_entry* pde = (struct dir_entry*)buf;
	for (int i=0; i<bytes / DIR_ENTRY_SIZE; i++, pde++) {
	    if (i == 0 && flag) {
		continue;
	    }
	    printf("%5d       %s\n", pde -> inode_nr, pde -> name);
	}
	bytes_left -= bytes;
	chunk = bytes_left > sizeof(buf) ? sizeof(buf) : bytes_left;
    }

    return 0;
    
}
