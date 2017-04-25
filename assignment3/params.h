/*
 Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>
 
 This program can be distributed under the terms of the GNU GPLv3.
 See the file COPYING.
 
 There are a couple of symbols that need to be #defined before
 #including all the headers.
 */
#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

// maintain bbfs state in here
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
struct sfs_state {
    FILE *logfile;
    char *diskfile;
};

/*
 mode
 physical address
 pow of 2
 offset
 */



typedef struct directoryRow{
    int inodeNumber;//-1 if it is free. otherwise inode# of directory
    char fileName[10];
}directoryRow;

/*
Description:
 Directory that can allow up to 31 files to be saved to it.
 If more files are needed we can use the indirection
Length:
 504 bytes in length
**/
typedef struct directory{
    directoryRow table[31];
    struct directory* indirect;
}directory;

/*
 Description:
    I node. about 4 of these fit in each 512 block. To support one file of 
    16Mb or (16*10^6 bytes) we need about 31250 disk blocks for this. 
 
 
    31250/12 means we need 2605 inodes to represent this 
 
    So we need a max of 2606 Inodes.
 
 
 -------------------------------------------------------------------------
 Length:
    128 bytes
 
 **/
typedef struct inode{
    char type;              //type. directory or file               1
    uid_t user_id;          //User ID  Number                       4
    gid_t group_id;         //Group ID Number                       4
    long fileSize;          //File Size In Bytes                    8
    time_t lastAccess;      //Time of last access.                  4
    time_t created;         //When was it created                   4
    time_t modified;        //When was it last modified             4
    int block_amount;       //amount of blocks it owns              4
    int pointers[12];       //pointers to blocks the                32
    directoryRow * dir;     //directory pointer if file is directory
    mode_t mode;
    //file owns
    char buffer[8];
}inode;

/*
 50 bytes
 */
typedef struct super_block {
    int init;                           //has the file system been initialized before ?
    unsigned long  s_magic;             /* filesystem's magic number*/
    unsigned long long  s_maxbytes;     /* max file size 6kb        */
    unsigned long  s_blocksize;         /* block size in bytes 512  */
    unsigned char s_blocksize_bits;     /* block size in bits 512*8 */
}super_block;

#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#endif
