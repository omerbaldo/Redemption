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
    int inodeNumber;
    struct directoryRow * next;
    char fileName[400];
    int lengthOfFileName;
    
}directoryRow;

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
}inode;

typedef struct super_block {
    unsigned long  s_magic;             /* filesystem's magic number*/
    unsigned long long  s_maxbytes;     /* max file size 6kb        */
    unsigned long  s_blocksize;         /* block size in bytes 512  */
    unsigned char s_blocksize_bits;     /* block size in bits 512*8 */
}super_block;

#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#endif
