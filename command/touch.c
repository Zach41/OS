#include "stdio.h"

int main2(int argc, char* argv[]) {
    assert(argc >= 2);

    int fd = open(argv[1], O_CREAT);
    if (fd == -1) {
	printf("Failed to create file %s\n", argv[1]);
	return -1;
    }

    close(fd);

    return 0;	
}
