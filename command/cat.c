#include "stdio.h"

int main2(int argc, char* argv[]) {
    assert(argc >= 2);
    int fd = open(argv[1], O_RDWR);

    if (fd == -1) {
	printf("Read file %s failed.\n", argv[1]);
	return -1;
    }
    char buf[33];
    int bytes;
    struct stat s;
    stat(argv[1], &s);
    int bytes_left = s.st_size;
    int chunk = bytes_left > 32 ? 32 : bytes_left;

    while(bytes_left) {
       int bytes = read(fd, buf, chunk);
       buf[bytes] = 0;
       printf("%s", buf);
       bytes_left -= bytes;
       chunk = bytes_left > 32 ? 32 : bytes_left;
	
    }
    printf("\n");

    return 0;
}
