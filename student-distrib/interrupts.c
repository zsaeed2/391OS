#include "interrupts.h"

#define KEYBOARD_INT 0x21
#define RTC_INT 0x28
#define MOUSE_INT 0x3C
#define SYS_CALL 0x80


/* void initialize_interrupts(void);
 * Inputs: void
 * Return Value: none
 * Function: Initializes the IDT */
void intialize_interrupts(){
	/* Used for indexing */
	int i;

	//idt is exported from x86_desc.h

	/* Init first 20 interrupts */
	/* TRAPS are software-generated interrupts
	while INTs are hardware-generated and unpredictable */
	/* Privelege level comes from linux standardization */
	init_trap_gate(0, &divide_error_exception, PRIVILEGED);
	init_trap_gate(1, &debug_exception, PRIVILEGED);
	init_int_gate (2, &NMI_interrupt, PRIVILEGED);
	init_int_gate (3, &breakpoint_exception, UNPRIVILEGED);
	init_trap_gate(4, &overflow_exception, UNPRIVILEGED);
	init_trap_gate(5, &BOUND_range_exceeded_exception, UNPRIVILEGED);
	init_trap_gate(6, &invalid_opcode_exception, PRIVILEGED);
	init_trap_gate(7, &dev_not_available_exception, PRIVILEGED);
	init_task_gate(8, &double_fault_exception, PRIVILEGED);
	init_trap_gate(9, &coprocessor_segment_overrun_exception, PRIVILEGED);
	init_trap_gate(10, &invalid_TSS_exception, PRIVILEGED);
	init_trap_gate(11, &segment_not_present_exception, PRIVILEGED);
	init_trap_gate(12, &stack_segment_fault_exception, PRIVILEGED);
	init_trap_gate(13, &general_protection_exception, PRIVILEGED);
	init_trap_gate(14, &page_fault_exception, PRIVILEGED);
	init_trap_gate(16, &FPU_floating_point_error_exception, PRIVILEGED);
	init_trap_gate(17, &alignment_check_exception, PRIVILEGED);
	init_trap_gate(18, &machine_check_exception, PRIVILEGED);
	init_trap_gate(19, &SIMD_floating_point_exception, PRIVILEGED);

	/* Interrupts 20 - 31 - Intel reserved */

	//Interrupts 32 to 255 - User defined:
	for(i = 32; i<NUM_VEC; i++) {
		init_int_gate_unused(i, &unused_exception, PRIVILEGED);
	} 

	/* Misc Interrupts */

	/* PIT CLOCK */
	init_int_gate(PIT_INT, &PIT_interrupt, PRIVILEGED);
	/* Keyboard */ 
	init_int_gate(KEYBOARD_INT, &keyboard_interrupt, PRIVILEGED);
	/* Real Time Clock */
	init_int_gate(RTC_INT, &RTC_interrupt, PRIVILEGED);
	/* Mouse */
	init_int_gate(MOUSE_INT, &mouse_handler, PRIVILEGED);
	/* System Call */
	init_trap_gate(SYS_CALL, &system_call_handler, UNPRIVILEGED);
}


/* void init_task_gate(void);
 * Inputs: vector - the interrupt vector number
 *           func - a pointer to the interrupt subroutine
 * 			 priv - the privilege level of the interrupt
 * Return Value: none
 * Function: Initializes a task gate interrupt */
void init_task_gate(int vector, void* func, int priv) {
	int addr = (int) func;
	idt[vector].offset_15_00 = addr & 0xFFFF;
	idt[vector].seg_selector = KERNEL_CS;
	idt[vector].reserved4 = 0; //ALWAYS 0
	//R0 S R1 R2 R3 = TYPE_ATTR
	//FOR TASK_GATE, TYPE_ATTR = 00101
	idt[vector].reserved3 = 1;
	idt[vector].reserved2 = 0;
	idt[vector].reserved1 = 1;
	idt[vector].size = 0;
	idt[vector].reserved0 = 0;
	idt[vector].dpl = priv;
	idt[vector].present = 1;
	idt[vector].offset_31_16 = (addr >> 16) & 0xFFFF;
}

/* void init_trap_gate(void);
 * Inputs: vector - the interrupt vector number
 *           func - a pointer to the interrupt subroutine
 * 			 priv - the privilege level of the interrupt
 * Return Value: none
 * Function: Initializes a trap interrupt */
void init_trap_gate(int vector, void* func, int priv) {
	int addr = (int) func;
	idt[vector].offset_15_00 = addr & 0xFFFF;
	idt[vector].seg_selector = KERNEL_CS;
	idt[vector].reserved4 = 0; //ALWAYS 0
	//R0 S R1 R2 R3 = TYPE_ATTR
	//FOR TRAP_GATE, TYPE_ATTR = 01111
	idt[vector].reserved3 = 1;
	idt[vector].reserved2 = 1;
	idt[vector].reserved1 = 1;
	idt[vector].size = 1;
	idt[vector].reserved0 = 0;
	idt[vector].dpl = priv;
	idt[vector].present = 1;
	idt[vector].offset_31_16 = (addr >> 16) & 0xFFFF;
}

/* void init_int_gate_unused(void);
 * Inputs: vector - the interrupt vector number
 *           func - a pointer to the interrupt subroutine
 * 			 priv - the privilege level of the interrupt
 * Return Value: none
 * Function: Initializes an unused interrupt */
void init_int_gate_unused(int vector, void* func, int priv) {
	int addr = (int) func;
	idt[vector].offset_15_00 = addr & 0xFFFF;
	idt[vector].seg_selector = KERNEL_CS;
	idt[vector].reserved4 = 0; //ALWAYS 0
	//R0 S R1 R2 R3 = TYPE_ATTR
	//FOR TRAP_GATE, TYPE_ATTR = 01110
	idt[vector].reserved3 = 0;
	idt[vector].reserved2 = 1;
	idt[vector].reserved1 = 1;
	idt[vector].size = 1;
	idt[vector].reserved0 = 0;
	idt[vector].dpl = priv;
	idt[vector].present = 0; //because unused
	idt[vector].offset_31_16 = (addr >> 16) & 0xFFFF;
}

/* void init_int_gate(void);
 * Inputs: vector - the interrupt vector number
 *           func - a pointer to the interrupt subroutine
 * 			 priv - the privilege level of the interrupt
 * Return Value: none
 * Function: Initializes a interrupt gate interrupt */
void init_int_gate(int vector, void* func, int priv) {
	int addr = (int) func;
	idt[vector].offset_15_00 = addr & 0xFFFF;
	idt[vector].seg_selector = KERNEL_CS;
	idt[vector].reserved4 = 0; //ALWAYS 0
	//R0 S R1 R2 R3 = TYPE_ATTR
	//FOR TRAP_GATE, TYPE_ATTR = 01110
	idt[vector].reserved3 = 0;
	idt[vector].reserved2 = 1;
	idt[vector].reserved1 = 1;
	idt[vector].size = 1;
	idt[vector].reserved0 = 0;
	idt[vector].dpl = priv;
	idt[vector].present = 1;
	idt[vector].offset_31_16 = (addr >> 16) & 0xFFFF;
}


/* void divide_error_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 0 */
void divide_error_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 1;
	}
	halt_c(USER_PROGRAM_CRASH);
	BSOD("Divide by 0 exception");
}

/* void debug_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 1 */
void debug_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 2;
	}
	halt_c(USER_PROGRAM_CRASH - 1);
	BSOD("Debug exception");
}


/* void NMI_interrupt(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 2 */
void NMI_interrupt() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 3;
	}
	halt_c(USER_PROGRAM_CRASH - 2);
	BSOD("NMI interrupt");
}

/* void breakpoint_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 3 */
void breakpoint_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 4;
	}
	halt_c(USER_PROGRAM_CRASH - 3);
	BSOD("Breakpoint exception");
}

/* void overflow_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 4 */
void overflow_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 5;
	}
	halt_c(USER_PROGRAM_CRASH - 4);
	BSOD("Overflow exception");
}

/* void BOUND_range_exceeded_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 5 */
void BOUND_range_exceeded_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 6;
	}
	halt_c(USER_PROGRAM_CRASH - 5);
	BSOD("BOUND range exceeded exception");
}

/* void invalid_opcode_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 6 */
void invalid_opcode_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 7;
	}
	halt_c(USER_PROGRAM_CRASH - 6);
	BSOD("Invalid opcode exception");
}

/* void dev_not_available_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 7 */
void dev_not_available_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 8;
	}
	halt_c(USER_PROGRAM_CRASH - 7);
	BSOD("Dev not available exception");
}


/* void double_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 8 */
void double_fault_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 9;
	}
	halt_c(USER_PROGRAM_CRASH - 8);
	BSOD("Double fault exception");
}

/* void coprocessor_segment_overrun_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 9 */
void coprocessor_segment_overrun_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 10;
	}
	halt_c(USER_PROGRAM_CRASH - 9);
	BSOD("Coprocessor segment overrun exception");

}

/* void invalid_TSS_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 10 */
void invalid_TSS_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 11;
	}
	halt_c(USER_PROGRAM_CRASH - 10);
	BSOD("Invalid TSS exception");

}

/* void segment_not_present_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 11 */
void segment_not_present_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 12;
	}
	halt_c(USER_PROGRAM_CRASH - 11);
	BSOD("Segment not present exception");

}

/* void stack_segment_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 12 */
void stack_segment_fault_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 13;
	}
	halt_c(USER_PROGRAM_CRASH - 12);
	BSOD("Stack segment fault exception");

}

/* void general_protection_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 13 */
void general_protection_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 14;
	}
	halt_c(USER_PROGRAM_CRASH - 13);
	BSOD("General protection exception");

}

/* void page_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 14 */
void page_fault_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 15;
	}
	halt_c(USER_PROGRAM_CRASH - 14);
	BSOD("Page fault exception");
}

/* void FPU_floating_point_error_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 16 */
void FPU_floating_point_error_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 16;
	}
	halt_c(USER_PROGRAM_CRASH - 15);
	BSOD("FPU floating point error exception");

}

/* void alignment_check_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 17 */
void alignment_check_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 17;
	}
	halt_c(USER_PROGRAM_CRASH - 16);
	BSOD("Alignment check exception");

}

/* void machine_check_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 18 */
void machine_check_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 18;
	}
	halt_c(USER_PROGRAM_CRASH - 17);
	BSOD("Machine check exception");

}

/* void SIMD_floating_point_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Shows BSOD for interrupt vector 19 */
void SIMD_floating_point_exception() {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 19;
	}
	halt_c(USER_PROGRAM_CRASH - 18);
	BSOD("SIMD floating point exception");

}

/* void unused_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: Does nothing. Just for the purpose of filling exceptions */
void unused_exception () {
	if(pcb_ptr_array[cur_pid] != NULL){
		pcb_ptr_array[cur_pid]->error_flag = 20;
	}
	halt_c(USER_PROGRAM_CRASH - 19);
	BSOD("Unused exception");

}

/* void mouse_handler(void);
 * Inputs: void
 * Return Value: none
 * Function: Handles mouse interrupts */
void mouse_handler() {

}

