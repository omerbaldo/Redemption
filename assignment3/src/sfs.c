/*
 Simple File System
 
 • .init : initialize file system
 • .destroy :  destroy the file system
 • .getattr :  get a file's information
 
 Kwabena
 • .create :   create and open a file
 • .unlink =   delete the file
 
 Terence
 • .open =    Open a file.
 • .release = Close a file.
 
 Omer
 • .read =    Read bytes from the given file into the buffer, beginning offset bytes into the file.
 • .write =    Write bytes from the given buffer into a file.
 
 
 Directory Functions (extra credit):
 • .readdir = Return one or more directory entries (struct dirent) to the caller.
 • .opendir = sfs opendir Open a directory for reading.
 • .releasedir = sfs releasedir
 • .rmdir = sfs rmdir Remove the given directory.
 • mkdir = makes a directory
 
 
 https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201001/homework/fuse/fuse_doc.html
 ---------------------------------------------------------------------------------------
 */

#include "params.h"
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

/**
 0 represents free
 1 represents not free
 */
char dataBitmap[amountOfDiskBlocks];
char inodeBitmap[amountOfINodes];
inode inodeTable[amountOfINodes];
super_block superBlock;
inode * root; //pointer to the root directory inode
directory rootDir;


/**
 This global variable deals with what current directory we are in.
 0 represents root directory.
 Without extra credit, current Directory will stay at 0
 */
static int currentDirectory = 0;


/**
 This uses functions from block.c to
 
 1) Open  a disk (testfsfile)
 2) Close a disk (testfsfile)
 3) Read  a block
 returns 1 when block size is returned
 returns 2 wh
 4)
 */

void printRootDirElements(){
    fprintf(stderr, "printRootDirElements() {\n");
    int i = 0;
    for(; i<31;i++){
        if(rootDir.table[i].inodeNumber == -1){continue;}
        fprintf(stderr, "%d  file : %s  Inode Num %d \n", i, rootDir.table[i].fileName, rootDir.table[i].inodeNumber );
        
    }
    fprintf(stderr, "}\n");
}


void printFirstTenINodes(){
    fprintf(stderr, "printTenInodes() {\n");
    int i = 0;
    for(; i<10;i++){
        if(inodeBitmap[i]=='1'){
            fprintf(stderr, "Inode Num %d \n", inodeTable[i].inode_number );
        }
    }
    fprintf(stderr, "}\n");
}

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
    
    fprintf(stderr, "bb-init(){\n");
    log_msg("\nsfs_init()\n");
    
    
    //Step 0) Create connection
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    disk_open((SFS_DATA)->diskfile);//SFS Data is a global
    int diskFileHandle = getDiskFile();
    
    //Step 1) Check if this file system has been initialized before
    
    void * ptr = (void *)&superBlock;
    int bytesRead = pread(diskFileHandle, ptr, sizeof(super_block), 0);
    //try to read in bytes into super block
    
    
    int hasBeenInitiled = 1;
    
    if(bytesRead==sizeof(super_block)){
        if(superBlock.init !=2017){
            hasBeenInitiled = -1;
        }
    }else{
        hasBeenInitiled = -1;
    }
    
    
    //Step 2.5) File system has not been initialized
    
    if (hasBeenInitiled == -1){
        fprintf(stderr, "\t this system has not been initalized \n ");
        
        
        //Step 2) SUPER BLOCK
        superBlock.s_magic = getpid();    									//magic number is process number
        superBlock.s_maxbytes = BLOCK_SIZE * maxDiskBlocks;                   //6kb is max file size
        superBlock.s_blocksize = BLOCK_SIZE;                                  //512 bytes
        superBlock.init = 2017;												//Block has been initiliazed
        
        
        //Step 3) ROOT DIR
        inodeTable[0].type = 'D';
        inodeTable[0].user_id = getuid();
        inodeTable[0].group_id = getegid();
        inodeTable[0].fileSize = 0;
        inodeTable[0].lastAccess = time(NULL);
        inodeTable[0].created = time(NULL);
        inodeTable[0].modified = time(NULL);
        inodeTable[0].block_amount = 0;
        inodeTable[0].group_id = getegid();
        inodeTable[0].mode = S_IFDIR | S_IRWXU | S_IRWXG;
        
        //ROOT DIR ELEMENTS
        int i = 0;
        for(;i<31;i++){
            rootDir.table[i].inodeNumber = -1; //no file exists
        }
        
        //First entry in a directory table is i
        rootDir.table[0].inodeNumber = 0;
        rootDir.table[0].fileName[0] = '.';
        rootDir.table[0].fileName[1] = '\0';
        
        //STEP 4) BIT MAP
        i = 0;
        for(;i<amountOfDiskBlocks;i++){
            dataBitmap[i] = '0';
        }
        i = 0;
        for(;i<amountOfINodes;i++){
            inodeBitmap[i] = '0';
        }
        inodeBitmap[0] = '1';//not free
        
        
        //Step 4) Null out all the pointers for each inode
        i = 0;
        for(;i<amountOfINodes;i++){
            int j = 0;
            for(;j<12;j++){
                inodeTable[i].pointers[j] = -1;
            }
        }
        //Step 5) Set root pointer, and make the pointer to its directory
        inodeTable[0].dir = &rootDir;     //directory pointer if file is directory
        root = &inodeTable[0];
        
        ftruncate(diskFileHandle, (722*BLOCK_SIZE));
        
        
    }else{
        
        fprintf(stderr, "\t this file system has been initalized. super block init value is %d \n",superBlock.init);
        
        //Step 1) Read the root directory struct at block 1
        
        void * ptr = (void *) &superBlock;
        
        pread(diskFileHandle, ptr, sizeof(superBlock), 0*BLOCK_SIZE);
        
        ptr = (void *) &rootDir;
        
        pread(diskFileHandle, ptr, sizeof(directory), 1*BLOCK_SIZE);
        
        ptr = (void *) &inodeBitmap;
        //Step 3) Read the inode bit map array in at block 2
        pread(diskFileHandle, ptr, sizeof(inodeBitmap), 2*BLOCK_SIZE);
        
        ptr = (void *) &dataBitmap;
        //Step 4) Read the disk bit map array at block 8
        pread(diskFileHandle, ptr, sizeof(dataBitmap), 8*BLOCK_SIZE);
        
        ptr = (void *) &inodeTable;
        
        root = &inodeTable[0];
        fprintf(stderr, "size of inode Table %d \n ",sizeof(inodeTable) );

        
        //Step 5) Read the inodes at block 722
        pread(diskFileHandle, ptr, sizeof(inodeTable), 70*BLOCK_SIZE);
        
    }
    fprintf(stderr, "}\n");
    
    return SFS_DATA;
}

/**
 * Clean up filesystem
 * Called on filesystem exit.
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    fprintf(stderr, "bb-destroy(){ \n");
    
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
    
    //write everything back in to the disk file
    
    //file handle for the disk file
    
    
    int diskFileHandle = getDiskFile();
    
    void * ptr = (void *) &superBlock;
    pwrite(diskFileHandle, ptr, sizeof(superBlock), 0*BLOCK_SIZE);
    
    ptr = (void *) &rootDir;
    
    pwrite(diskFileHandle, ptr, sizeof(directory), 1*BLOCK_SIZE);
    
    ptr = (void *) &inodeBitmap;
    //Step 3) Read the inode bit map array in at block 2
    pwrite(diskFileHandle, ptr, sizeof(inodeBitmap), 2*BLOCK_SIZE);
    
    ptr = (void *) &dataBitmap;
    //Step 4) Read the disk bit map array at block 8
    pwrite(diskFileHandle, ptr, sizeof(dataBitmap), 8*BLOCK_SIZE);
    
    ptr = (void *) &inodeTable;
    
    //Step 5) Read the inodes at block 722
    
    
    pwrite(diskFileHandle, ptr, sizeof(inodeTable), 70*BLOCK_SIZE);
    fprintf(stderr, "} \n");
    
}



/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 
 
 Only Required Things:
 st_mode
 st_uid
 st_gid
 st_nlink
 st_ino
 st_dev
 
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    fprintf(stderr, "getattr() searching for %s {\n", path);
    
    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
            path, statbuf);
    
    if(superBlock.init == 2017){
        fprintf(stderr, "\t We good in get attr. Values are read in correct \n");
    }
    int retstat = 0;
    char fpath[PATH_MAX];
    inode * node;
    
    if (strcmp("/",path) == 0) {//root
        
        fprintf(stderr, "\t We are looking for root directory\n");
        
        //Required
        statbuf->st_dev = 0;                //Device ID of device containing file.
        statbuf->st_rdev = 0;               //Device ID (if file is character or block special)
        statbuf->st_ino = 0;                //File serial number.
        statbuf->st_nlink = 1;              //Number of hard links to the file.
        
        statbuf->st_mode = root->mode;      //Mode of file. (file type and permissions)
        statbuf->st_uid = root->user_id;    //user id
        statbuf->st_gid = root->group_id;   //group id
        statbuf->st_atime = root->lastAccess ;   /* time of last access */
        statbuf->st_mtime = root->created;   /* time of last modification */
        statbuf->st_ctime = root->modified;   /* time of last status change */
        
        
    } else {
        fprintf(stderr, "\t We are looking for another file. Here are the things in the directory \n");
        printRootDirElements();
        
        int result = findINode(path, currentDirectory);
        
        if (result == -1) {
            fprintf(stderr, "\t Error file does not exsist\n");
            
            //log_msg("\tError: path does not exist\n");
            
            return -ENOENT;
            //ENOENT; // no file or directory
        }
        
        node = &inodeTable[result];
        
        statbuf->st_dev = 0;//Device ID of device containing file.
        statbuf->st_rdev = 0;//Device ID (if file is character or block special)
        statbuf->st_ino = 0;//File serial number.
        statbuf->st_nlink = 1;//Number of hard links to the file.
        statbuf->st_mode = node->mode;//Mode of file. (file type and permissions)
        statbuf->st_uid = node->user_id;//user id
        statbuf->st_gid = node->group_id;//group id
        statbuf->st_atime = node->lastAccess ;   /* time of last access */
        statbuf->st_mtime = node->created;   /* time of last modification */
        statbuf->st_ctime = node->modified;   /* time of last status change */

        

    }
    
    fprintf(stderr, "}\n");
    
    return retstat;//0
}

/*
  I rewrote the findInode method cause the other one keep bugging out. 
  Basically no matter what if we accept only sing directory support then it will be / filename
 
 example)
 
  touch ././././././mountdir/bbb.b , touch /mountdir/bbb.b, touch mountdir/bbb.b
 
 they all search for the string /bbb.b or bbb.b
 
*/
int findINode(const char *path, int currentLocation){
    
    
     const char * filename = path;
    
    if(*filename == '/'){
        filename = (filename+1);
    }
    
    
        fprintf(stderr, "findInode(){ %s \n", filename);
        
        int i = 0;
    

        for (; i < 31; i++) {
            if(rootDir.table[i].inodeNumber == -1){continue;}
            fprintf(stderr, "\t comparing %s with %s }\n",filename, rootDir.table[i].fileName);
        
        
            if ( strcmp(filename, rootDir.table[i].fileName) == 0){
                fprintf(stderr, "\t found inode %d }", rootDir.table[i].inodeNumber);

                return rootDir.table[i].inodeNumber;
            }
        }
    
    
    fprintf(stderr, "\t  inode not found returning -1 }");

        return -1; //not found
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
 
 
 If we are using root directories, it should work for the following test cases
 
 
 touch ./test.txt
 touch test.txt
 ls /                              works
 
 
 */
/*
int findINode(const char *path, int currentLocation){
    fprintf(stderr, "findInode(){ %s \n", path);

    int len = strlen(path);//length of current path
    int i = 0;
    
    char c;
    int result = -1;
    char temp_path[len];
    char current_file[len];
    
    for(i = 0; i < len; i++) { // for each character or until /
        c = *(path + i);
        
        if(c == '/') {
            
            //copy path name up until the slash and go search for that directory
            //in the current folder
            strncpy(current_file, path, i);
            int new_location = findchild(currentLocation, current_file);
            
            //if directory does not exist in the current folder
            if(new_location == -1) {
                printf("Error: path does not exist\n");
                return -1;
            }
            
            //else keep on recursing on the shortened path name
            strcpy(temp_path,path+(i+1));
            result = findINode(temp_path, new_location);
            break;
        }
        
        //end of the path name reached
        //search for the file in the current directory
        if(c == '\0') {
            result = findchild(currentLocation, path);
            break;
        }
        
    }
    
    
    
    // 1
    // for each character in path                             photos/water.jpg
    // if there is a slash
    // substring = get the substring up to that slash  photos
    // check if it exists in parent dir
    // findINode(less of a path, and inode)        findINode(water.jpg,1)
     
     
    // if it gets here there is no slash
    // check if it is in parentDirectory
    // return i node number
    // else
    // return -1
     
     
    fprintf(stderr, "} \n");

    
    return result;
}
*/

/*
int findchild (const int current_dir, char * child) {
 
    // get current directory's files and loop until
    // you find the file or directory name that matches the child name
 
    int i = 0;
    
    
    if (inodeTable[current_dir].type != 'D') {
        printf("Error: Folder does not exist\n");
        return -1;
    }
    
    directory * curr = inodeTable[current_dir].dir;
    
    for (; i < 31; i++) {
        if(curr->table[i].inodeNumber == -1){continue;}
        fprintf(stderr, "comparing %s with %s \n",child, curr->table[i].fileName);

        
        if ( strcmp(child, curr->table[i].fileName) == 0)
            return curr->table[i].inodeNumber;
    }
    
    return -1;
}
*/




void addFileToRoot (const char * fileName, int fileInodeNumber){
    
    if(*fileName == '/'){
        fileName = (fileName+1);
    }
    
    int i =0;
    for(;i<31;i++){
        if(rootDir.table[i].inodeNumber==-1){
            //its free
            rootDir.table[i].inodeNumber = fileInodeNumber;
            
            memcpy((void *)&rootDir.table[i].fileName, fileName, strlen(fileName)+1);
            rootDir.table[i].fileName;
            
            fprintf(stderr, "\t Adding file %s with inode # %d\n", fileName, fileInodeNumber);
            
            printRootDirElements();
            return;
        }
    }
    
    //here do memory indirection
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
    fprintf(stderr, "create() adding this file %s {\n", path);
    
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
            path, mode, fi);
    
    //Step 0) Check if inode for file trying to be created exists
    int result = findINode(path, currentDirectory);
    
    //Step 1) If it doesn't exist find empty inode in bitmap and give inode in table correct properties
    if (result == -1) {
        printf("path does not exist. Create that junk\n");
        
        int i = 0;
        for( ;i < amountOfINodes; i++){
            if (inodeBitmap[i] == '0'){
                inodeTable[i].type = 'F';                                  //File
                inodeTable[i].user_id = getuid();                          //User id
                inodeTable[i].group_id = getegid();                        //Group ID
                fprintf(stderr, "\t Permissions. File Group  %d File User %d\n",inodeTable[i].group_id,inodeTable[i].user_id);

                
                
                
                inodeTable[i].fileSize = 0;
                inodeTable[i].lastAccess = time(NULL);
                inodeTable[i].created = time(NULL);
                inodeTable[i].modified = time(NULL);
                inodeTable[i].block_amount = 0;
                inodeTable[i].group_id = getegid();
                
                /*
                   S_IFREG =  regular file
                   S_IRWXU =  read, write, execute/search by owner
                   S_IRWXG =  read, write, execute/search by group
 
                 */
                inodeTable[i].mode = S_IFREG | S_IRWXU | S_IRWXG;
                
                inodeTable[i].inode_number = i;
                fi->fh = i;
                
                int j = 0;
                for(;j<12;j++){
                    inodeTable[i].pointers[j] = -1;
                }
                
                
                
                inodeBitmap[i] = '1';//not free
                
                addFileToRoot(path,i); //add to the root directory.
                
                printFirstTenINodes();
                fprintf(stderr, "\t returning inode # %d \n}\n", i);
                return 0;
            }
        }
    }
    
    //Step 2) If it does exist just return -1
    return -1;
}




void removeFileFromRoot (const char * fileName, int fileInodeNumber){
    
    if(*fileName == '/'){
        fileName = (fileName+1);
    }
    
    fprintf(stderr, "removeFileFromRoot () {\n", fileName, fileInodeNumber);

    int i =0;
    for(;i<31;i++){
        if(rootDir.table[i].inodeNumber==fileInodeNumber){

            rootDir.table[i].inodeNumber = -1; //refree
            fprintf(stderr, "\t Deleting file %s with inode # %d\n", fileName, fileInodeNumber);
            printRootDirElements();
            fprintf(stderr, "}\n", fileName, fileInodeNumber);
            return;
        }
    }
    
    //here do memory indirection
    
    
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    fprintf(stderr, "unlink %s (){ \n", path);
    
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);
    
    
    //Step 0) Check if inode for file trying to be deleted exists
    int result = findINode(path, currentDirectory);
    
    //Step 1) If it doesn't exist just return -1
    if (result == -1) {
        printf("path does not exist. Cannot delete\n");
        return -ENOENT;
    }
    
    //Step 1.5) remove from root dir

    removeFileFromRoot(path,result);
    
    //Step 2) If it does exist set inode bitmap to 0
    inodeBitmap[result] = '0';// freed
    int j = 0;
    for(;j<12;j++){
        dataBitmap[inodeTable[result].pointers[j]] = '0';
        inodeTable[result].pointers[j] = -1;
    }
    
    fprintf(stderr, "} \n");
    
    return 0;
    
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
 
 Path indicates the name of the file.
 The flag argument indicates whether the file
 is to be read, written, or “updated” (read and written simultaneously)
 The returned value filep is called a file descriptor.
 It is a small integer used to identify the file in subsequent calls
 to read, write, or otherwise manipulate it. (return i-node # as fd)
 
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    fprintf(stderr, "open (){ \n");

    int retstat = 0;
    int fd;
    
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
            path, fi);
    fprintf(stderr, "open (){ \n");
    
    
    //set fd to inode # associated with the file
    
    
    fd = findINode(path, currentDirectory);
    if(fd <0){return fd;}
    
    int groupIdCurrrently = getegid();
    int userIdCurrrently = getuid();
    
    fprintf(stderr, "\t found inode # %d\n", fd);
    fprintf(stderr, "\t Permissions. Group Now %d User Now %d\n",groupIdCurrrently,userIdCurrrently);
    fprintf(stderr, "\t Permissions. File Group  %d File User %d\n",inodeTable[fd].group_id,inodeTable[fd].user_id);

    
    if(inodeTable[fd].group_id != groupIdCurrrently || inodeTable[fd].user_id != userIdCurrrently){
        //do not have permissions
        fprintf(stderr, "\t You do not have group or user permissions to open this file\n");

        return -EACCES; //error code for permission denied
    }
    
    if (fd == -1) {
        fprintf(stderr, "\t Error opening file\n");
        return -ENOENT; //error for no file exsisting
    }
    fi->fh = fd;    //set file descriptor to inode #
    fprintf(stderr, "} \n");
    
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
    fprintf(stderr, "sfs release (){ \n");
    
    //do not close the file descriptor. its not a real file descriptor but the inode # that lets us know
    //where to write in the disk file. that descriptor should always remain open and is closed by FUSE / Block.c
    //essentially this method should do absolutely nothing.
    
    
    //result = close(fi->fh);
    /*
    if (result == -1) {
        printf("Error closing file descriptor\n");
        return -1;
    }
     */
    fprintf(stderr, "} \n");
    
    return retstat;
}



int getBlockRead(int inode_num, int blockIndex){
    
    fprintf(stderr, "reading next block index %d  \n", blockIndex);

    inode * NODE = &inodeTable[inode_num];
    
    //mem indirection. >6kb
    if(blockIndex >=12){
        return -1;
    }
    
    if(NODE->pointers[blockIndex] == -1){
        return -1;
    }
    
    //has another block
    fprintf(stderr, "value %d  \n", NODE->pointers[blockIndex]);

    return NODE->pointers[blockIndex];
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
 
 Arguements)
 
 const char *path path of file
 char *buf is where we will read the bytes into
 size is size of bytes we are reading
 offset is where to start reading
 fi is a struct that gives us information about open files. in it we have the following fields
 (int)fh for file handle. this is filled out by open
 (int)flags  flags.
 
 
 Algorithm)
 open() recursively finds the file and allocates a file descriptor for something to read.
 get the current file open (from file descriptor)
 
 begin reading into the buff.
 start at a certain point. keep going. if you run out of space stop reading at curr block and go to new block
 
 
 error) try to read too much bytes
 
 int block_read(const int block_num, void *buf);
 
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    
    /*
    char * name1 = (char *)malloc(5);
    name1[0] = 'o';
    name1[1] = 'm';
    name1[2] = 'e';
    name1[3] = 'r';
    name1[4] = '\0';
    buf = name1;
    return 5;
    
     */

    
    
    //Step 0) Loging ------------------------------------------------------------------
        fprintf(stderr, "read (){ \n");
        fprintf(stderr, "\t path: %s  \n",path);
        fprintf(stderr, "\t inode #: %d  \n",fi->fh);
        fprintf(stderr, "\t offset: %d  \n", offset);
        fprintf(stderr, "\t size: %d  \n", size);
    
        log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
                path, buf, size, offset, fi);
    
    //Step 1) Define Your Variables ---------------------------------------------------
    
        int inodeNum = fi->fh;
        int diskfile = getDiskFile();
    
        inode * NODE = &inodeTable[inodeNum];
        // current inode for open file
        
        int blockIndex = ((int)offset/(int)BLOCK_SIZE);
        fprintf(stderr, "block Index  %d \n", blockIndex);
    
        //ex. trunc(1000/512) means start at index 1
        int byteOffset = offset % BLOCK_SIZE;
        fprintf(stderr, "byte offset   %d \n", byteOffset);
    
    
    //try to get the block that represents this
    int currDiskBlock = getBlockRead(inodeNum,blockIndex);
    if (currDiskBlock < 0){
        fprintf(stderr, "ran out of reading space 1 \n", currDiskBlock);
        
        return 0;//err
    }
    fprintf(stderr, "got block %d \n", currDiskBlock);
    
    
    int bytesRemaining = size;
    
    int bytesRead = 0;
    
    int sizeBetween = 0;
    
    int buffOffset = 0;
    
    
    void * ptr = (void *) &(buf);

    
    /*
    char * name = (char *)malloc(5);
    pread(diskfile,name,sizeof(name),(722+currDiskBlock)*BLOCK_SIZE);
    buf = name;
    return 5;
    */
    
    
    /*
    pread(diskfile, ptr, 4, (722+currDiskBlock)*BLOCK_SIZE);

    fprintf(stderr, "0) reading in %c  \n", *(buf));
    fprintf(stderr, "1) reading in %c  \n", *(buf+1));
    fprintf(stderr, "2) reading in %c  \n", *(buf+2));
    fprintf(stderr, "3) reading in %c  \n", *(buf+3));
    fprintf(stderr, "4) reading in %c  \n", *(buf+4));
    fprintf(stderr, "5) reading in %c  \n", *(buf+5));
    
    
    
    
    
    return 0;
    */
    
    
    
    do{
        sizeBetween = BLOCK_SIZE - byteOffset;
        fprintf(stderr, "size between %d and %d remaining \n", sizeBetween,bytesRemaining);

        
        if(sizeBetween >= bytesRemaining){
            fprintf(stderr, "reading at offset %d, about %d bytes at %d \n", (buffOffset),bytesRemaining, ((722+currDiskBlock)*BLOCK_SIZE));
            


            pread(diskfile, buf+buffOffset, bytesRemaining, (722+currDiskBlock)*BLOCK_SIZE);
            fprintf(stderr, "-----1----- \n");

            fprintf(stderr, "0) reading in %c  \n", *(buf));
            fprintf(stderr, "1) reading in %c  \n", *(buf+1));
            fprintf(stderr, "2) reading in %c  \n", *(buf+2));
            fprintf(stderr, "3) reading in %c  \n", *(buf+3));
            fprintf(stderr, "4) reading in %c  \n", *(buf+4));
            fprintf(stderr, "5) reading in %c  \n", *(buf+5));
            
            bytesRead += bytesRemaining;
            bytesRemaining -= bytesRemaining;
            return bytesRead;
        }else if (sizeBetween < bytesRemaining){
            fprintf(stderr, "reading at offset %d, about %d bytes at %d \n", (buffOffset),sizeBetween, ((722+currDiskBlock)*BLOCK_SIZE));
            

            pread(diskfile, buf+buffOffset, sizeBetween, (722+currDiskBlock)*BLOCK_SIZE);
            fprintf(stderr, "-----2----- \n");

            fprintf(stderr, "0) reading in %c  \n", *(buf));
            fprintf(stderr, "1) reading in %c  \n", *(buf+1));
            fprintf(stderr, "2) reading in %c  \n", *(buf+2));
            fprintf(stderr, "3) reading in %c  \n", *(buf+3));
            fprintf(stderr, "4) reading in %c  \n", *(buf+4));
            fprintf(stderr, "5) reading in %c  \n", *(buf+5));
            
            bytesRead += sizeBetween;
            bytesRemaining -= sizeBetween;
            buffOffset += sizeBetween;
            byteOffset = 0;
        }
        
        //get nxt blck. if theres nothing there add a new one
        blockIndex++;
        int response = getBlockRead(inodeNum,blockIndex);
        if (response >= 0){
            
            currDiskBlock = response;
        }else{
            fprintf(stderr, "No next block return %d bytes read  \n", bytesRead);

            return bytesRead;
        }
        
        
    }while(bytesRemaining > 0);
    
    fprintf(stderr, "returning %d bytes read } \n", size);
    return size;
}
/**
 This func essentially emulates
 
 size_t pread(int fd, void *buf, size_t count, off_t offset);
 
 fd is the file descriptor of the file
 buff is the buffer
 count is how much you want to read in
 offset is basically the offset from the beginning of the file.
 */





int getFreeBlock(){
    fprintf(stderr, "\t \t  getFreeBlock()  \n");

    int i = 0;
    for(;i<maxDiskBlocks;i++){
        if(dataBitmap[i] == '0'){
            
            
            fprintf(stderr, "\t \t  \t returning i %d \n", i);

            
            dataBitmap[i] = '1';
            return i;
        }
    }
    return -1;//full
}



/*

 attempt to increment the blockIndex to get the next owned block by the inode
 
 return the actual location or -1 on error
 
*/

int getBlock(int inode_num, int blockIndex){
   
    inode * NODE = &inodeTable[inode_num];
    
    //mem indirection. >6kb
    if(blockIndex >=12){
        return -ENOMEM;
    }
    
    //doesn't have another block
    if(NODE->pointers[blockIndex] == -1){
        
        int indx =getFreeBlock();
        if(indx == -1){ //no space
            return -ENOMEM; //no more memory
        }
        NODE->pointers[blockIndex]  = indx;
        return indx;
    }
    
    //has another block
    return NODE->pointers[blockIndex];
}









/** Write data to an open file
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 
 
 
 
 buf is your buffer
 size is how
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi)
{
    //Step 0) Loging ------------------------------------------------------------------
        fprintf(stderr, "write (){ \n");
        fprintf(stderr, "\t path: %s  \n",path);
        fprintf(stderr, "\t inode #: %d  \n",fi->fh);
        fprintf(stderr, "\t offset: %d  \n", offset);
        fprintf(stderr, "\t size: %d  \n", size);
    
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    
    //Step 1) Define Your Variables ---------------------------------------------------
    
        int inodeNum = fi->fh;
        int diskfile = getDiskFile();
    
        inode * NODE = &inodeTable[inodeNum];
        // current inode for open file
    
        int blockIndex = ((int)offset/(int)BLOCK_SIZE);
        fprintf(stderr, "block Index  %d \n", blockIndex);

    
        //ex. trunc(1000/512) means start at index 1
        int byteOffset = offset % BLOCK_SIZE;
        fprintf(stderr, "byte offset   %d \n", byteOffset);

    
        //try to get the block that represents this
        int currDiskBlock = getBlock(inodeNum,blockIndex);
        if (currDiskBlock < 0){
            fprintf(stderr, "returning error \n", currDiskBlock);

            return 0;//err
        }
            fprintf(stderr, "got block %d \n", currDiskBlock);

    
            int bytesRemaining = size;
    
            int bytesWritten = 0;                //    number of bytes read in
    
            int numberOfBytesRemaining = size;       //     number of bytes read in
    
            int sizeBetween = 0;
    
            int buffOffset = 0;
/*
    char name[5];
    name[0] = 'o';
    name[1] = 'm';
    name[2] = 'e';
    name[3] = 'r';
    name[4] = '\0';
    pwrite(diskfile,name,sizeof(name),(722+currDiskBlock)*BLOCK_SIZE);
    return 5;
*/
    
    
    
    /*
    fprintf(stderr, "0) writing in %c  \n", *(buf));
    fprintf(stderr, "1) writing in %c  \n", *(buf+1));
    fprintf(stderr, "2) writing in %c  \n", *(buf+2));
    fprintf(stderr, "3) writing in %c  \n", *(buf+3));
    fprintf(stderr, "4) writing in %c  \n", *(buf+4));
    fprintf(stderr, "5) writing in %c  \n", *(buf+5));
    
    void * ptr = (void *) &(buf);
    
    */
   // pwrite(diskfile, ptr, size, (722+currDiskBlock)*BLOCK_SIZE);
    
    
    do{
        sizeBetween = BLOCK_SIZE - byteOffset;
        
        if(sizeBetween >= bytesRemaining){
            fprintf(stderr, "0) writing in %c  \n", *(buf));
            fprintf(stderr, "1) writing in %c  \n", *(buf+1));
            fprintf(stderr, "2) writing in %c  \n", *(buf+2));
            fprintf(stderr, "3) writing in %c  \n", *(buf+3));
            fprintf(stderr, "4) writing in %c  \n", *(buf+4));
            fprintf(stderr, "5) writing in %c  \n", *(buf+5));

            fprintf(stderr, "reading at offset %d, about %d bytes at %d \n", (buffOffset),bytesRemaining, ((722+currDiskBlock)*BLOCK_SIZE));



            pwrite(diskfile, buf+buffOffset, bytesRemaining, (722+currDiskBlock)*BLOCK_SIZE);
            bytesWritten += bytesRemaining;
            bytesRemaining -= bytesRemaining;
            return bytesWritten;
        }else if (sizeBetween < bytesRemaining){
            fprintf(stderr, "2) writing in %c \n", buf);
            fprintf(stderr, "reading at offset %d, about %d bytes at %d \n", (buffOffset),sizeBetween, ((722+currDiskBlock)*BLOCK_SIZE));


            pwrite(diskfile, buf+buffOffset, sizeBetween, (722+currDiskBlock)*BLOCK_SIZE);

            bytesWritten += sizeBetween;
            bytesRemaining -= sizeBetween;
            buffOffset += sizeBetween;
            byteOffset = 0;
        }
        
       //get nxt blck. if theres nothing there add a new one
       blockIndex++;
       int response = getBlock(inodeNum,blockIndex);
       if (response >= 0){
            currDiskBlock = response;
       }else{
            return bytesWritten;//err
       }
        
        
    }while(bytesRemaining != 0);
    
    fprintf(stderr, "}\n");
    return size;
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
 
 Return one or more directory entries (struct dirent) to the call
 
 struct dirent {
 ino_t          d_ino;        inode number
 off_t          d_off;        offset to the next dirent
 unsigned short d_reclen;     length of this record
 unsigned char  d_type;       type of file; not supported
 by all file system types
 char           d_name[256];
 };
 
 
 Algo:
 get all the elements of the directory
 fill out information about them in the dirent struct (just the name)
 
 

 Args:
 path:    what folder to open
 buf:     holds dirent structs
 filler:  insert directory entries into the directory structure,
 which is also passed to your callback as buf.
 offset:  ignore this
 fi:      ignore this too
 
 
 
 https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/unclear.html
 
 
 
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                struct fuse_file_info *fi)
{
    fprintf(stderr, "readdir (){ \n");
    
    //Step 1. Count how much files exsist
    struct dirent temp;
    int i;
    
    int offset_mult = 0;
    for(i = 1;i<31;i++){//start at 1 so you dont include directory itself
        if(rootDir.table[i].inodeNumber != -1){//directory element
            
            
            memcpy(temp.d_name,rootDir.table[i].fileName, 10);
            temp.d_ino = rootDir.table[i].inodeNumber;           //inode #
            temp.d_off = sizeof(struct dirent);
            
            //buffer, dirent, pointer to struct stat or null, offset to next directory entry
            if(filler(buf, temp.d_name, NULL, 0)!= 0){
                return -ENOMEM;
            }
            offset_mult++;
        }
    }
    
    fprintf(stderr, "} \n");
    return 0;
}

/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
            path, mode);
    fprintf(stderr, "mkdir (){ \n");
    fprintf(stderr, "}\n");
    
    
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
            path);
    
    fprintf(stderr, "rmdir (){ \n");
    fprintf(stderr, "}\n");
    
    
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
    fprintf(stderr, "opendir () {\n");
    fprintf(stderr, "}\n");
    
    
    
    return retstat;
}




int sfs_truncate(const char *path , off_t offset) {
    fprintf(stderr, "sfs_truncate () {}\n");

    return 0;
}

int sfs_ftruncate(const char * path, off_t offset, struct fuse_file_info * fi){
    fprintf(stderr, "sfs_ftruncate () {}\n");
    
    return 0;
}




/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    fprintf(stderr, "releasedir () {\n");
    
    fprintf(stderr, "}\n");
    
    int retstat = 0;
    
    
    return retstat;
}

struct fuse_operations sfs_oper = {
    
    /*
        To do / Debugg:
     */
    
    .init = sfs_init,           // successful. next add more than 31 directories for root dir (at least 255)
    .destroy = sfs_destroy,     // successful.
    .getattr = sfs_getattr,     // successful/sketchy. says times function is not implemented. ask monday whats up with that
    
    //terence
    .open = sfs_open,           // ?
    .release = sfs_release,     // ?
    
    //kwabe
    .create = sfs_create,       // successful
    .unlink = sfs_unlink,       // successful
    
    //omer
    .read = sfs_read,           //
    .write = sfs_write,         //
    .readdir = sfs_readdir,     //works !
    .truncate = sfs_truncate,
    .ftruncate = sfs_ftruncate,
    
    .rmdir = sfs_rmdir,
    .mkdir = sfs_mkdir,
    
    .opendir = sfs_opendir,
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
