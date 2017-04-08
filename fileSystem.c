/* Redemption */


//Libraries-------------------------------------------------------------------------------------------------------------------------------------------------------------





//Structs-------------------------------------------------------------------------------------------------------------------------------------------------------------
	/**
	Super block structure. (with each block 512 bytes or 4096 kb)
		I Nodes

		Data Region

	*/


//Helper Methods-------------------------------------------------------------------------------------------------------------------------------------------------------

	void memory_init(){}
	int find_free_inode(){}
	int find_free_datanode(){}
	int create_inode(int index){} //args are 

//Main Methods---------------------------------------------------------------------------------------------------------------------------------------------------------

	/**
		1) Find Free INode
		2) Write To INode BitMap To Mark Allocated
		3) Initialize The Inode
		4) Link Directory To Inode. (name of file to inode)
			4.5) Read & Write the directory inode to update it
				4.55) If there is no space, add a new inode for directory.
	*/
	int create(char* path){}

	/**
		Not Sure
	*/
	int delete(char* path){}

	/**
		Not Sure
	*/
	int stat(char* path, struct stat* buf){}

	/**
		Algorithm:

		if path starts with /
			start at root directory
		else
			check current user directory. start search here 
		read inode of current directory
			recursively search path until inode is found
			check permissions of file. 
				if permissions are good
					allocate file descriptor in file table
					return file descriptor
		return error num
	*/
	int open(char* path){}

	/**
		update file table?
	*/
	int close(int fileID){}


	/**
		check file table to see if file is open
		if it is not open
			 open the file.
			 start to read at byte 0
		else
			start read from position in file table
	*/
	int read(int fileID, void* buffer, int bytes){}


	/**
		1) Check if file is open. must be open to write
		2) Write, if you run out of space
			2.5) find free data block to allocate. reflect bit mask to not free
			2.6) allocate a new inode if needed to point to this new data
				if you dont need another inode update the last datablock pointer in inode to point to this. 		
	*/
	int write(int fileID, void* buffer, int bytes){}

	/**

	*/
	struct dirent* readdir(int directoryID){}