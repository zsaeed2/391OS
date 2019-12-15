#ifndef _SYSTEM_CALLS_ASM_H
#define _SYSTEM_CALLS_ASM_H

#ifndef ASM 
#include "system_calls.h"
//#include "fs.h"

/*function to call when system call occurs
jumps to correct system call function */
extern int system_call_handler();
//extern long file_ops_table;
//extern long dir_ops_table;

#endif
#endif
