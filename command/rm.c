#include "stdio.h"

int main2(int argc, char* argv[]) {
    assert(argc >= 2);

    for (int i=1; i<argc; i++) {
	int ret = unlink(argv[i]);
	if (ret != 0) {
	    printf("Delete file %s failed.\n", argv[i]);
	} else {
	    printf("Delete file %s succeed.\n", argv[i]);
	}
    }
    return 0;
}
