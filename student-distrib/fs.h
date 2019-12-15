#ifndef _FS_H
#define _FS_H

//#include "lib.h"
//#include "rtc.h"
#include "system_calls.h"
//#include "types.h"


#define B4 1                 // pointer is uint32_t, so adding 1 is adding 4B
#define B8 2
#define B32 8
#define B64 16
#define KB4 1024
#define MAX_FILE_NUM 62      // since 64 64B blocks in 4KB, remove 2 64B blocks for boot block and dir info 
#define MAX_FILE_NAME_LEN 31 // since null terminated 32B - 1B
#define MAX_UINT32 0xFFFFFFFF 
#define FILE_TYPE 2
#define DIR_TYPE 1
#define RTC_TYPE 0
#define INVALID_ENTRY -1

// struct to hold directory entry information once opened
typedef struct dentry{
    int8_t file_name[32];
    uint32_t file_type;
    uint32_t inode_index;
    uint32_t file_size;
} dentry_t;

// struct to make quick access to boot block info 
typedef struct fs{
    uint32_t num_dir_entries; 
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint32_t* dir_entries_ptr; 
} fs_t;

// global file system struct, initialized when calling fs_init in kernel.c
fs_t fs;
uint32_t* fs_ptr;

// function prototypes for fs.c
extern void fs_init();
//extern int32_t file_open(const uint8_t* file_name);
//extern int32_t dir_open(uint8_t* dir_name);
//extern int32_t file_close(int32_t fd);
//extern int32_t dir_close(int32_t fd);
extern int32_t file_read(int32_t fd, void* buf, int32_t num_of_bytes);
extern int32_t dir_read(int32_t fd, void* buf, int32_t num_of_bytes); 

extern int32_t read_dentry_by_name(const uint8_t* file_name, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t dir_index, dentry_t* dentry);
extern int32_t read_data(uint32_t inode_index, uint32_t offset, uint8_t* buf, uint32_t len);

#endif
