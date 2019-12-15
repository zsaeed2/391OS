#ifndef _SCHED_H
#define _SCHED_H

#include "system_calls.h"
#include "i8259.h"
#include "paging.h"

// frequencies in Hz
#define PIT_FREQ 1193182 
// 50 Hz gives cycles 20 ms
#define FREQ 50
#define COUNT (PIT_FREQ/FREQ)
#define FREQ_HIGH_BYTE (COUNT >> 0x8)
#define FREQ_LOW_BYTE  (COUNT & 0xFF)
// Channel0 [7:6] = 00, Access Mode Lobyte/Hibyte [5:4] 11, Mode 3 [3:1] 111, binary mode [0] 0
#define COMMAND 0x36 
#define CMD_REG 0x43
#define CH0_PORT 0x40

extern void PIT_init();
extern void PIT_interrupt_handler();
void sched();

#endif
