#ifndef _INT_WRAPPER_H
#define _INT_WRAPPER_H

#ifndef ASM 

/*function to call when keyboard interrupt occurs
  wrapper of keyboard handler*/
extern void keyboard_interrupt();

/*function to call when RTC interrupt occurs
  wrapper of RTC handler*/
extern void RTC_interrupt();

/*function to call when PIT interrupt occurs
  wrapper of PIT handler*/
extern void PIT_interrupt();

#endif
#endif
