/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
 
 
 We are required to fill out certain functions in this file.
 
     • .init : initialize file system
     • .destroy :  destroy the file system
     • .getattr :  get a file's information
 
     • .create :   create and open a file
     • .unlink =   delete the file
    
     • .open =    Open a file.
     • .release = Close a file.
 
     • .read =    Read bytes from the given file into the buffer, beginning offset bytes into the file.
     • .write =    Write bytes from the given buffer into a file.
     
     • .readdir = Return one or more directory entries (struct dirent) to the caller.
     
     Directory Functions (extra credit):
         • .opendir = sfs opendir Open a directory for reading.
         • .releasedir = sfs releasedir
         • .rmdir = sfs rmdir Remove the given directory.
         • mkdir = makes a directory
---------------------------------------------------------------------------------------
*/

/**

*/
#include "params.h"


/**
 0 represents free
 1 represents not free
 */
int dataBitmap[amountOfDiskBlocks];
int inodeBitmap[amountOfINodes];
inode inodeTable[amountOfINodes];
super_block superBlock;

inode * root;

/**
    This uses functions from block.c to
 
        1) Open  a disk (testfsfile)
        2) Close a disk (testfsfile)
        3) Read  a block 
            returns 1 when block size is returned
            returns 2 wh
        4)
*/

#include "block.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"


//--------------------------------------------------------------------------------

//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *sfs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "in bb-init\n");
    //log_msg("HELLO WORLD\n");
    log_msg("\nsfs_init()\n");
    //printf("in init");
    log_conn(conn);
    log_fuse_context(fuse_get_context());

    //Step 1) Open the disk file (file which we save everything to)
     disk_open((SFS_DATA)->diskfile);//SFS Data is a global
    
    //Step 2) Set the super block
     superBlock->s_magic = getpid();     //magic number is process number
     superBlock->s_maxbytes =     BLOCK_SIZE * maxDiskBlock;  //6kb is max file size
     superBlock->s_blocksize =    BLOCK_SIZE;   //512 bytes
     superBlock->s_blocksize_bits = BLOCK_SIZE*8;
    
    //Step 3) Set the root directory
     inodeTable[0].type = 'D';                                  //Directory
     inodeTable[0].user_id = getuid();                          //User id
     inodeTable[0].group_id = getegid();                        //Group ID
     inodeTable[0].fileSize = 0;
     inodeTable[0].lastAccess = time(NULL);
     inodeTable[0].created = time(NULL);
     inodeTable[0].modified = time(NULL);
     inodeTable[0].block_amount = 0; //Directory
     inodeTable[0].group_id = getegid(); //Directory
     inodeTable[0].mode = S_IFDIR | S_IRWXU | S_IRWXG;
     i = 0;
     for(;i<12;i++){
        inodeTable[0].pointers = -1;
     }
     inodeTable[0].directoryRow = NULL;     //directory pointer if file is directory
     root = inodeTable[0];
     //Name of the file
    
    
    return SFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    
    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
   
    
    if (strcmp("/",path) == 0) {//root

        stat->st_dev = 0;//Device ID of device containing file.
        stat->st_rdev = 0;//Device ID (if file is character or block special)
        stat->st_ino = 0;//File serial number.
        stat->st_nlink = 1;//Number of hard links to the file.
        stat->mode = root->mode;//Mode of file. (file type and permissions)
        stat->st_uid = root->user_id;//user id
        stat->st_gid = root->group_id;//group id
        
        //time
        stat->st_atime = root->lastAccess;
        stat->st_mtime = root->modified;
        stat->st_ctime = root->created;
    } else {
        
    }

    
    return retstat;
}
/*
 Given a path get the inode that represents it. 
    return -1 on err
 
 
 Before calling check if the path begins with a /
    if so begin search at the root directory or inode 0
 
    findINode( path without /, 0)
 
 
 Else
    begin search at current directory
    findINode( path \, curr_dir)
 
 

    example)
        cat /doc/file.txt      starts search at root directory and get doc
        cat file.txt           starts search at your current directory
 
 
 
 
*/
int findINode(const char *path, int currentLocation){
    int i, len = 0;
    char * c;
    
    len = strlen(path);
    
    for(i = 0; i < len; i++) {
        c = *path[i];
        
        if(c == '/'){
            char[] subPath;
            
            
        }
    }
    
    
    /**
                                                            1
     for each character in path                             photos/water.jpg
        if there is a slash
            substring = get the substring up to that slash  photos
            check if it exists in parent dir
                findINode(less of a path, and inode)        findINode(water.jpg,1)
     
    
     if it gets here there is no slash
        check if it is in parentDirectory
            return i node number
        else
            return -1
     
     */
    
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);
    
    
    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    
    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);

    
    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    

    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);

   
    return retstat;
}

/** Write data to an open file
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
    
    
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
   
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    
    
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    
    
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    
    
    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    
    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,
  .getattr = sfs_getattr,

    //terence
   .open = sfs_open,
   .release = sfs_release,

    //kwabe
  .create = sfs_create,
  .unlink = sfs_unlink,
    
    //omer
  .read = sfs_read,
  .write = sfs_write,
    
    
    
    
    
    
    
    
    
    
    
    

    
    
  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}



/**
    Hand over control to FUSE library using arguements
*/
int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    sfs_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
