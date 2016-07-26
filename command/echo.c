#include "stdio.h"

int main2(int argc, char* argv[]) {
    for (int i=1; i< argc; i++) {
	printf("%s%s", i==1 ? "" : " ", argv[i]);
    }
    printf("\n");
    /* exit(111); */

    return 0;
}
