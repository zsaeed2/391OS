#ifndef _SYSTEM_CALLS_H
#define _SYSTEM_CALLS_H

#include "lib.h"
#include "types.h"
#include "terminal.h"
#include "rtc.h"
#include "fs.h"
#include "paging.h"
#include "x86_desc.h"
#include "interrupts.h"

#define NUM_TERMS 3
#define aligned_1 4
#define MAX_PCBS  6
#define MAX_OPEN_FILES 8
#define BUF_LEN 128
#define KB8 8192
#define MB8 0x800000
#define MB4 0x400000
#define MB128 0x8048000
#define MB132 0x8400000
#define NUM_OF_MAGIC_NUMBERS  4
#define MAGIC_NUM_0         0x7f
#define MAGIC_NUM_1         0x45
#define MAGIC_NUM_2         0x4c
#define MAGIC_NUM_3         0x46


/*  Struct for file operations 
    Used in table.              */
typedef struct fops{
    int32_t (*open)(const uint8_t* file_name);
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} fops_t;


/*
    struct for file descriptor array. Contains pointers to file ops table and inode.
    file position used to tell offset for reas_data in fs.c
    flags used to tell if entry is empty. Should be set to 0 for all
    file descriptors upon initialization
*/
typedef struct file_desc{
    fops_t ops_table;
    int32_t inode_index;
    uint32_t file_position;
    uint32_t flags;
} file_desc_t;

/*
    PCB struct used for every process.
    Currently holds proccess id, parent process id, esp for process in kernel memory,
    buffer to hold args, and file descriptor array
*/
typedef struct pcb{
    int32_t pid;
    int32_t parent_pid;
    int32_t term_id;
    uint32_t kernel_esp;
    uint32_t user_esp;
    uint32_t user_ebp;
    uint8_t error_flag;
    uint8_t isParent;
    uint8_t*  vidmap_ptr;
    uint8_t cmd[BUF_LEN];
    uint8_t args[BUF_LEN];
    file_desc_t file_desc_array[MAX_OPEN_FILES];
} pcb_t;


// global variable for the current process id. This id is an entry in the proccess pointer array
extern int32_t cur_pid;
extern int32_t cur_term;
extern int32_t running_terms[NUM_TERMS];

// global array of processes, contains ptr to pcb on its index.
// if process not intialized, the entry is null
// when process halts, it must reset its entry to NULL
extern pcb_t* pcb_ptr_array[MAX_PCBS];

// Function to initialize a new PCB takes the buffer to retrieve args as input
// returns the process id which is the index of the pcb ptr in the global pcb_ptr_array
extern int32_t init_pcb(uint8_t* buf, int32_t len);

extern int32_t halt_c (uint8_t status);

extern int32_t execute_c (const uint8_t* command);

extern int32_t read_c (int32_t fd, void* buf, int32_t nbytes);

extern int32_t write_c (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t open_c (const uint8_t* filename);

extern int32_t close_c (int32_t fd);

extern int32_t vidmap (uint8_t** screen_start);

extern int32_t getargs_c (uint8_t* buf, int32_t nbytes);

extern int32_t bad_call_open (const uint8_t* str);
extern int32_t bad_call_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t bad_call_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t bad_call_close (int32_t fd);

int32_t file_open (const uint8_t* file_name);
int32_t file_close (int32_t fd);

#endif
