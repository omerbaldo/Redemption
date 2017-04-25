#include <stdlib.h>
#include <stdio.h>
#include "params.h"

int main(int argc, char **argv){
	printf("i node size is %d\n", sizeof(inode));
	printf("super block size is %d\n", sizeof(super_block));
	printf("directory size is %d\n", sizeof(directory));
}

