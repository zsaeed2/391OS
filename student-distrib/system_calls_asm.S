#define ASM		1
.data
	NUM_SYS_CALLS = 8
.globl system_call_handler

	# ASM wrapper for keyboard interrupts, pushes flags and registers before calling handler
	# then pops flags and calls IRET
system_call_handler:
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %ebp
    pushl %edi
    pushl %esi
	pushw %ds
	pushw %es
	pushw %fs
	pushw %gs
    pushfl
    # compare call number against valid values
check_arg_below:
	cmpl $1, %eax
	jl invalid_args
check_arg_above:
	cmpl $NUM_SYS_CALLS, %eax
	jg invalid_args
valid_args:
	# push args
	pushl %edx
	pushl %ecx
	pushl %ebx
	# compare count with jumptable
	# indexing starts at one
	addl $-1, %eax
	jmp *system_calls(,%eax,4)

return:
    popfl
	popw %gs
	popw %fs
	popw %es
	popw %ds
    popl %esi
    popl %edi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx
    iret

fnx_return:
	addl $12, %esp
	jmp return

invalid_args:
	movl $-1, %eax
	jmp return

	# first argument in EBX, then ECX, then EDX

	# int32_t halt (uint8_t status)
	# Terminates a process.
	# Inputs:
		# status -
	# Outputs:
		# none
halt:
	call halt_c
	jmp fnx_return

	# int32_t execute (const uint8_t command)
	# Attempts to load and execute a new process.
	# Inputs:
		# command - the command to execute
	# Outputs:
		# none
execute:
	call execute_c
	jmp fnx_return

	# int32_t read (int32_t fd, void* buf, int32_t nbytes)
	# Reads from a keyboard, file, device, or directory.
	# Inputs:
		# fd -
		# buf -
		# nbyes -
	# Outputs:
		# none
read:
	call read_c
	jmp fnx_return

	# int32_t write (int32_t fd, const void* buf, int32_t nbytes)
	# Carries data to terminal or RTC.
	# Inputs:
		# fd -
		# buf -
		# nbyes -
	# Outputs:
		# none
write:
	call write_c
	jmp fnx_return

	# int32_t open (const uint8_t* filename)
	# Provides access to file system
	# Inputs:
		# filename - name of file to open
	# Outputs:
		# none
open:
	call open_c
	jmp fnx_return

	# int32_t close (int32_t fd)
	# Closes file descriptor
	# Inputs:
		# status -
	# Outputs:
		# none
close:
	call close_c
	jmp fnx_return
	# int32_t getargs (uint8_t* buf, int32_t nbytes)
	# Closes file descriptor
	# Inputs:
		# buffer and number of bytes -
	# Outputs:
		# none
getargs:
	call getargs_c
	jmp fnx_return
	# int32_t vidmap (uint8_t** screen_start)
	# Closes file descriptor
	# Inputs:
		# double pointer to start of screen -
	# Outputs:
		# none
vidmap:
	call vidmap_c
	jmp fnx_return

# Jump table containing all possible system calls
system_calls:
.long halt, execute, read, write, open, close, getargs, vidmap
