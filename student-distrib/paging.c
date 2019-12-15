#include "paging.h"
#include "paging_asm.h"

/*
* init_paging
* Description: initializes paging by creating and initializing
  page directory and page table
* Inputs: None
* Return Value: None
* Side effects:
*/
void init_paging(void) {
  // initialize page directory to 2 (not present) and page table values to 0
  memset(page_directory, NOT_PRESENT, ONE_KB*sizeof(uint32_t));

  // initialize video page table for user programs to access screen as not present
  memset(video_page_table, NOT_PRESENT, ONE_KB*sizeof(uint32_t));

  // Note that KB4 is defined with respect to a 4B pointer so need to multiply by 4 to get 4096
  video_page_table[0] = (uint32_t)(VID_ADDR) | USER_ATTRIBUTES;
  video_page_table[1] = (uint32_t)(TERM1_ADDR) | USER_ATTRIBUTES;
  video_page_table[2] = (uint32_t)(TERM2_ADDR) | USER_ATTRIBUTES;
  video_page_table[3] = (uint32_t)(TERM3_ADDR) | USER_ATTRIBUTES;



  //set page_table at 0 to 0
  page_table[0] = ZERO;

  int i; //loop counter
  for(i = 1; i < ONE_KB; i++)
    page_table[i] = NOT_PRESENT | (i * SCALE); //4 B per 4 KB

  //accounting for kernel
  page_table[VIDEO_MEMORY] = page_table[VIDEO_MEMORY] | PRESENT;
  //page attributes are "present" and "user mode"
  page_table[VIDEO_MEMORY + 1] = page_table[VIDEO_MEMORY + 1] | PAGE_ATTRIBUTES;
  page_table[VIDEO_MEMORY + 2] = page_table[VIDEO_MEMORY + 2] | PAGE_ATTRIBUTES;
  page_table[VIDEO_MEMORY + 3] = page_table[VIDEO_MEMORY + 3] | PAGE_ATTRIBUTES;


  //add page table to page directory
  //supervisor attributes indicate page is present, writeable, and in supervisor mode (011)
  page_directory[0] = SUP_ATTRIBUTES | ((uint32_t)page_table);

  //add kernel to page directory
  //supervisor attributes indicate page is present, writeable, and in supervisor mode (011)
  page_directory[1] = SUP_ATTRIBUTES | KERNEL_ADDRESS | PAGE_SIZE;

  //clear();

  //call x86 methods to load directory and enable paging
  loadPageDirectory(page_directory);
  enablePaging();
}

/*
* new_task_page
* Description: allocates memory for and creates place for new task page
* Inputs: pid
* Outputs: none
* Side effects: adds entry into page directory
*/
void new_task_page(uint32_t pid) {
  //allocate space for new task
  
  uint32_t task_address;

  task_address = ((FOUR_MB*pid) + EIGHT_MB);

  // 32 in page directory is at 128 MB
  // User attributes indicate user mode, writeable page, and present page (111)
  page_directory[32] = task_address | PAGE_SIZE | USER_ATTRIBUTES;
  flush_tlb();
  return;
}
/*
* switch_task_page
* Description: switches the page
* Inputs: pid
* Outputs: none
* Side effects: switches the page
*/
void switch_task_page(uint32_t pid) {
  //allocate space for new task
  uint32_t task_address;
  task_address = ((FOUR_MB*pid) + EIGHT_MB);

  // 32 in page directory is at 128 MB
  // User attributes indicate user mode, writeable page, and present page (111)
  page_directory[32] = task_address | PAGE_SIZE | USER_ATTRIBUTES;
  flush_tlb();
  return;
}

/*
* flush_tlb
* Description: flushes tlb
* Inputs: none
* Outputs: none
* Side effects: reloads cr3
*/
void flush_tlb() {
  asm volatile(
    "movl %%cr3, %%eax \n"
    "movl %%eax, %%cr3 \n"
    ::: "eax"
  );
}

/*
* vidmap_init
* Description: allocates page for new process
* Inputs: None
* Outputs: address of page
* Side effects : creates page and allows user to write to its video memory
*/
uint8_t* vidmap_init() {
  //only one process at a time for now so allocate it this way
  uint32_t virtual_addr = MB132;
  uint32_t running_term = pcb_ptr_array[cur_pid]->term_id;

  page_directory[33] = USER_ATTRIBUTES | ((uint32_t)(video_page_table));

  if(cur_term != running_term){
    video_page_table[0] = video_page_table[running_term+1];
  }
  else{
    video_page_table[0] = USER_ATTRIBUTES | (uint32_t)(VID_ADDR);
  }

  //always flush
  flush_tlb();

  return (uint8_t*)virtual_addr;
}

/*  
 *  Function updates vidmap pages when a context switch occurs
 *  NOTE: That this happens during IRQ0 handler, should not be interrupted by other interrupts
 *  Do not place in cli()/sti(). If neccesary, place critical section in sched where this is called.
 *  MAKE SURE that this function is called after cur_term is updated for new process
 *  INPUT: NONE
 *  OUTPUT: NONE
 *  SIDEEFFECT: remaps page at 132MB depending on the new process. 
 */
void update_vidmap(){
  int32_t running_term = pcb_ptr_array[cur_pid]->term_id;

  page_directory[33] = (uint32_t)(USER_ATTRIBUTES | ((uint32_t)(video_page_table)));

  if(running_term != cur_term){
    video_page_table[0] = video_page_table[running_term+1];
  }
  else{
    video_page_table[0] = (uint32_t)(USER_ATTRIBUTES | (uint32_t)(VID_ADDR));
  }

  flush_tlb();
  return;
}

/*
* vidmap_close
* Description: deallocates page previously allocated in vidmap_init
* Inputs: None
* Outputs: none
* Side effects : sets specified pages to not present
*/
void vidmap_close(){
  //set all positions in vidmap_init to not PRESENT
  //as a proper way to deallocate the page

  page_directory[33] = NOT_PRESENT;
  flush_tlb();
  return;
}
