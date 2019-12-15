#ifndef _PAGING_H
#define _PAGING_H

#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "system_calls.h"

//magic numbers
#define ONE_KB 1024
#define FOUR_KB 4096
#define FOUR_MB 0x400000
#define EIGHT_MB 0x800000
#define KERNEL_ADDRESS 0x00400000

//more magic numbers
#define PRESENT 0x00000001
#define NOT_PRESENT 0x00000002
#define SUP_ATTRIBUTES 0x00000003
#define USER_ATTRIBUTES 0x00000007
#define PAGE_ATTRIBUTES 0x00000005
#define VIDEO_MEMORY 0xB8
#define ZERO 0x0
#define PAGE_SIZE 0x00000080
#define SCALE 0x00001000
#define PAGE_OFFSET 12

//declare page directory and page table and align them properly
uint32_t page_directory[ONE_KB] __attribute__((aligned (FOUR_KB)));
uint32_t page_table[ONE_KB] __attribute__((aligned (FOUR_KB)));
uint32_t video_page_table[ONE_KB] __attribute__((aligned (FOUR_KB)));

//initialize paging
extern void init_paging(void);
//create page for new program
extern void new_task_page(uint32_t pid);
//switch page for context switch in scheduling
extern void switch_task_page(uint32_t pid);
// function updates vidmap for current process during context switch
extern void update_vidmap();
//reload cr3
extern void flush_tlb();
//create page
extern uint8_t* vidmap_init();
//destroy page
extern void vidmap_close();
#endif //_PAGING_H
