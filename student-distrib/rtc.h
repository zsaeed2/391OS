#ifndef _RTC_H
#define _RTC_H


#include "types.h"

/*register number and disable NMI*/
#define RTC_PORT            0x70

/* r/w to cmos ram */
#define CMOS_PORT           0x71

/* Use with Register values to select
   registor to write to and disables NMI */

#define REG_A       0x8A
#define REG_B       0x8B
#define REG_C       0x8C

/* which bit in reg B corosponds to periodic interrupts*/
#define TURN_ON_BIT 0x40

/* rate consttants*/
#define RATE_ADJUSTMENT   17
#define RATE_1024         6
#define RATE_2            15
#define RATE_MASK         0xF0
#define DEFAULT_FREQUENCY 2
#define MAX_FREQUENCY     1024


/*initializes rtc*/
extern void rtc_init();

/*interrupt handler for rtc*/
extern void rtc_interrupt_handler();

/*driver function to edit frequency per process*/
int32_t set_frequency(uint32_t frequency);

/*driver function to edit frequency on rtc*/
int32_t write_frequency(uint32_t frequency);

/*system calls*/

/*returns 0 after interrupt occurs*/
extern int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

/*changes frequency on rtc*/
extern int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);

/*opens rtc, sets default frequency */
extern int32_t rtc_open (const uint8_t* filename);

/*NOT IMPLEMENTED*/
extern int32_t rtc_close (int32_t fd); 

#endif /* _RTC_H */
