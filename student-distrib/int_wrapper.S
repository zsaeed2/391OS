
#define ASM     1

.globl keyboard_interrupt, RTC_interrupt, PIT_interrupt
# ASM wrapper for keyboard interrupts, pushes flags and registers before calling handler
# then pops flags and calls IRET
keyboard_interrupt:
    pushl %eax
    pushl %ebp
    pushl %ebx
    pushl %ecx
    pushl %edi
    pushl %edx
    pushl %esi
    pushfl
    call keyboard_interrupt_handler
    popfl 
    popl %esi
    popl %edx
    popl %edi
    popl %ecx
    popl %ebx
    popl %ebp
    popl %eax
    iret 


# ASM wrapper for RTC interrupts, pushes flags and registers before calling handler
# then pops flags and calls IRET

RTC_interrupt:
    pushl %eax
    pushl %ebp
    pushl %ebx
    pushl %ecx
    pushl %edi
    pushl %edx
    pushl %esi
    pushfl
    call rtc_interrupt_handler
    popfl 
    popl %esi
    popl %edx
    popl %edi
    popl %ecx
    popl %ebx
    popl %ebp
    popl %eax
    iret 

# ASM wrapper for PIT interrupts, pushes flags and registers before calling handler
# then pops flags and calls IRET

PIT_interrupt:
    pushl %eax
    pushl %ebp
    pushl %ebx
    pushl %ecx
    pushl %edi
    pushl %edx
    pushl %esi
    pushfl
    call PIT_interrupt_handler
    popfl 
    popl %esi
    popl %edx
    popl %edi
    popl %ecx
    popl %ebx
    popl %ebp
    popl %eax
    iret 
