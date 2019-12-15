#include "sched.h"

/* void PIT_init();
 * Inputs: void
 * Return Value: none
 * Function: Init PIT */
void PIT_init(){
    //set channel 0 to mode 3, sending lowbyte/highbyte
    outb(COMMAND, CMD_REG);
    //send lowbyte of fequency to channel 0
    outb(FREQ_LOW_BYTE, CH0_PORT);
    //send highbyte of frequency to channel 0
    outb(FREQ_HIGH_BYTE, CH0_PORT);
    enable_irq(IRQ_PIT);
    return;
}

/* void PIT_interrupt_handler(void);
 * Inputs: void
 * Return Value: none
 * Function: Runs interrupt for PIT */
void PIT_interrupt_handler(){
    send_eoi(IRQ_PIT);
    sched();
    return;
}


/* void sched(void);
 * Inputs: void
 * Return Value: none
 * Function: Allows each process to run periodically. Triggered by PIT interrupt */
void sched(){
    int next_pid;
    cli();

    // sanity check in case interrupt occurs before first execute    
    if(cur_pid < 0 || cur_pid >= MAX_PCBS){
        return;
    }

    next_pid = cur_pid;

    do{
        next_pid = (next_pid + 1) % MAX_PCBS; 
    }while(pcb_ptr_array[next_pid] == NULL || pcb_ptr_array[next_pid]->isParent);

    // sanity check
    // should not happen since 3 shells should be running at any given time
    if(cur_pid == next_pid){
        //printf("ERROR: Only one PCB initialized.");
        return;
    }

    // update current process with the current ebp and esp
    pcb_t* cur_pcb = pcb_ptr_array[cur_pid];
    asm volatile(   "movl %%esp, %0 	\n"
                    "movl %%ebp, %1 	\n"
                    : "=r"(cur_pcb->user_esp), "=g"(cur_pcb->user_ebp)
                    :: "memory"
                );    

    
    // switch the current page at address 128MB
    switch_task_page(next_pid);

    // update current process id
    cur_pid = next_pid;

    if(pcb_ptr_array[next_pid]->vidmap_ptr != NULL){
        update_vidmap();
    }
    

    // update kernel stack
    tss.ss0 = KERNEL_DS;
    tss.esp0 = pcb_ptr_array[cur_pid]->kernel_esp;
    
    // get pointer to the pcb that we are switching to
    pcb_t* next_pcb = pcb_ptr_array[cur_pid];

    // must reenable before switch since return doesnt execute till next switch
    sti();

    // load the next pcb's ebp/esp
    asm volatile(   "movl %0, %%esp     \n"
                    "movl %1, %%ebp     \n"
                    :
                    : "a"(next_pcb->user_esp), "b"(next_pcb->user_ebp)
                    : "esp", "ebp"
                );

    // return should breakdown the sched on the switched process stack
    return;
}



