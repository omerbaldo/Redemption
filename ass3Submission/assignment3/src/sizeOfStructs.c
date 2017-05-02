#include <stdlib.h>
#include <stdio.h>
#include "params.h"
#include "block.h"

char dataBitmap[amountOfDiskBlocks];
char inodeBitmap[amountOfINodes];
inode inodeTable[amountOfINodes];
int main(int argc, char **argv){
    printf("i node size is %d\n", sizeof(inode));
    printf("super block size is %d\n", sizeof(super_block));
    printf("directory size is %d\n", sizeof(directory));
    printf("Inode Table size is %d\n", sizeof(inodeTable));
    printf("Inode BitMap size is %d\n", sizeof(inodeBitmap));
    printf("Disk BitMap size is %d\n", sizeof(dataBitmap));
    
}

