#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "keyboard.h"
#include "lib.h"
#include "system_calls.h"

#define BUF_LENGTH 128
#define SCR_WIDTH 80
#define SCR_HEIGHT 25
#define VID_ADDR 0x000B8000
#define TERM1_ADDR (VID_ADDR + KB4*4)
#define TERM2_ADDR (VID_ADDR + 2*KB4*4)
#define TERM3_ADDR (VID_ADDR + 3*KB4*4)

/* Terminal's buffer */
char terminal_buf[BUF_LENGTH];


/* Functions used by terminal.c */
extern int32_t terminal_open (const uint8_t* filename);
extern int32_t terminal_write (int32_t fd, const void* given_buf, int32_t length);
extern int32_t terminal_read (int32_t fd, void* given_buf, int32_t length);
extern int32_t terminal_close (int32_t fd);
extern int32_t switch_term(int32_t term_id);
int32_t copy_to_terminal_buffer(char* key_buf, int32_t length);
void clear_screen();

#endif
