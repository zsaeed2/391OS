#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

//#include "x86_desc.h"
//#include "types.h"
//#include "lib.h"
#include "keyboard.h"
#include "int_wrapper.h"
//#include "i8259.h"
//#include "rtc.h"
#include "system_calls_asm.h"
//#include "system_calls.h"


/* Interrupt vector numbers */
#define PIT_INT 0x20
#define KEYBOARD_INT 0x21
#define RTC_INT 0x28
#define MOUSE_INT 0x3C
#define SYS_CALL 0x80
/* Privilege number */
#define PRIVILEGED 0
#define UNPRIVILEGED 3
#define USER_PROGRAM_CRASH 255

extern void intialize_interrupts();

extern void init_task_gate(int vector, void* func, int priv);
extern void init_trap_gate(int vector, void* func, int priv);
extern void init_int_gate_unused(int vector, void* func, int priv);
extern void init_int_gate(int vector, void* func, int priv);

extern void divide_error_exception();

extern void debug_exception();

extern void NMI_interrupt();

extern void breakpoint_exception();

extern void overflow_exception();

extern void BOUND_range_exceeded_exception();

extern void invalid_opcode_exception();

extern void dev_not_available_exception();

extern void double_fault_exception();

extern void coprocessor_segment_overrun_exception();

extern void invalid_TSS_exception();

extern void segment_not_present_exception();

extern void stack_segment_fault_exception();

extern void general_protection_exception();

extern void page_fault_exception();

extern void FPU_floating_point_error_exception();

extern void alignment_check_exception();

extern void machine_check_exception();

extern void SIMD_floating_point_exception();

extern void unused_exception ();

extern void mouse_handler();

//extern void system_call_handler();

extern void keyboard_interrupt();

#endif
