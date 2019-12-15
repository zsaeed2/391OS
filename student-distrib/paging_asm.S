#define ASM 1

# code obtained from wiki.osdev.org/Paging

.text

.globl loadPageDirectory
.globl enablePaging
.globl flush_tlb

# Description: Loads page directory to enable paging
# Inputs: page directory address
# Return values: None
# Side effects: writes to cr0, cr4 registers

loadPageDirectory:
  pushl %ebp
  movl  %esp, %ebp
  movl  8(%esp), %eax
  movl  %eax, %cr3
  # Next 3 lines used to specify kernel memory
  movl %cr4, %eax
  orl $0x00000010, %eax
  movl %eax, %cr4
  movl  %ebp, %esp
  popl  %ebp
  ret

# Description: enables Paging
# Inputs: none
# Outputs: none
# Side effects: enables paging

enablePaging:
  pushl %ebp
  movl  %esp, %ebp
  movl  %cr0, %eax
  orl   $0x80000001, %eax
  movl  %eax, %cr0
  movl  %ebp, %esp
  popl  %ebp
  ret


