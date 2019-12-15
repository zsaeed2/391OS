/* keyboard.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H


#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

/* Ports that each PS/2 controller reads from */
#define PS2_PORT    0x60

/*number of key code flags*/
#define KEY_CODE_FLAG_AMOUNT    (0x7D)

/*buffer constants*/
#define BUFFER_SIZE          128
#define NUM_OF_TERMINALS       3

/* Opcodes for keyboard */

#define RB_TEST_PASSED      0xAA
#define RB_ACK              0xFA
#define RB_TEST_FAILED_A    0xFC
#define RB_TEST_FAILED_B    0xFD
#define RB_RESEND           0xFE
#define CB_RESET            0xFF

/*ascii characters*/

#define ASCII_TAB           0x09
#define ASCII_SINGLE_QUOTE  0x27
#define ASCII_LEFT_SLANT    0x5C
#define ASCII_RIGHT_SLANT   0x2F
#define ASCII_SPACE         0x20
#define ASCII_BACKSPACE     0x08

/*scan_codes*/
#define SC_SPACE            0x39
#define SC_SHIFT_LEFT       0x2A
#define SC_SHIFT_LEFT_REL   0xAA
#define SC_SHIFT_RIGHT		0x36
#define SC_SHIFT_RIGHT_REL	0xB6
#define SC_CTRL             0x1D
#define SC_CTRL_REL         0x9D
#define SC_CAPS             0x3A
#define SC_CAPS_REL         0xBA
#define SC_ENTER            0x1C
#define SC_ENTER_REL        0x9C
#define SC_BACKSPACE        0x0E
#define SC_BACKSPACE_REL    0x8E
#define SC_ALT_LEFT			0x38
#define SC_ALT_LEFT_REL		0xB8
#define SC_ALT_RIGHT		0x54
#define SC_ALT_RIGHT_REL	0xD4
#define SC_F1               0x3B
#define SC_F2               0x3C
#define SC_F3               0x3D


/* Used for inputs */
volatile uint8_t hit_enter_key[3];

/* Externally-visible functions */

/* Initialize keyboard */
extern void keyboard_init(void);

/*interrupt handler*/
extern void keyboard_interrupt_handler();


#endif /* _KEYBOARD_H */
