/* keyboard.c - Functions to interact with the keyboard
 *
 */

#include "keyboard.h"

#define TAKETHATL 0x26

static uint8_t key_buffer[NUM_OF_TERMINALS][BUFFER_SIZE];
static int buffer_index[NUM_OF_TERMINALS] = {0,0,0};

uint8_t get_char(uint8_t s_code);
int key_code_flags[KEY_CODE_FLAG_AMOUNT];

//pre declaration
void clear_buffer();

//flags for special inputs
volatile uint8_t shift_l = 0;
volatile uint8_t shift_r = 0;
volatile uint8_t caps = 0;
volatile uint8_t ctrl = 0;
volatile uint8_t alt_l = 0;
volatile uint8_t alt_r = 0;

/*look up table for scancode to ascii conversion*/
//scan code table for no shift/caps
uint8_t s_code_table[58] = {0,0,'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', ASCII_BACKSPACE,
                     ASCII_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
                     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ASCII_SINGLE_QUOTE, '`',
                     0,ASCII_LEFT_SLANT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', ASCII_RIGHT_SLANT, 0, '*',
                     0, ASCII_SPACE};
//scan code table for caps only
uint8_t s_code_table_caps[58] = {0,0,'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', ASCII_BACKSPACE,
                    ASCII_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
                    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ASCII_SINGLE_QUOTE, '`',
                    0,ASCII_LEFT_SLANT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', ASCII_RIGHT_SLANT, 0, '*',
                    0, ASCII_SPACE};
//scan code table for shift only
uint8_t s_code_table_shift[58] = {0,0,'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', ASCII_BACKSPACE,
                    ASCII_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
                    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '"', '~',
                    0,'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
                    0, ASCII_SPACE};
//scan code table for both shift and caps.
//named because why you gotta do me like that.
uint8_t s_code_table_why[58] = {0,0,'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', ASCII_BACKSPACE,
                    ASCII_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
                    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '"', '~',
                    0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, '*',
                    0, ASCII_SPACE};



/*
 * keyboard_init(void)
 * DESCRIPTION: intialises keyboard
 * INPUT: none
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: enables keyboard interrupt line on PIC
 */

void keyboard_init(void){
  int i;
  cur_term = 0;
  for(i = 0; i< NUM_OF_TERMINALS; i++){
    hit_enter_key[i] = 0;
  }
  enable_irq(IRQ_KEYBOARD);
}


/*
 * keyboard_interrupt_handler()
 * DESCRIPTION: handles keyboard generated interrupt
 * INPUT: none
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: reads from keyboard por
 *               prints charecters to screen
 *               sends EOI for keyboard so another interrupt can be sent
 */
void keyboard_interrupt_handler(){
    uint8_t output = 0;
    uint8_t input = 0;

    //grab input from keyboard port, keep pulling till valid output is given
    while(input == 0){
        input = inb(PS2_PORT);
    }
    switch(input) {
      case SC_SHIFT_LEFT: //pressed shift key
        shift_l = 1;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_SHIFT_LEFT_REL: //released shift key
        shift_l = 0;
        send_eoi(IRQ_KEYBOARD);
        return;
  	  case SC_SHIFT_RIGHT:
        shift_r = 1;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_SHIFT_RIGHT_REL:
        shift_r = 0;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_CAPS: //pressed caps lock
        caps = !caps;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_CAPS_REL: // released caps key
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_CTRL: //pressed control key
        ctrl = 1;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_CTRL_REL: //released control key
        ctrl = 0;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ALT_LEFT:
        alt_l = 1;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ALT_LEFT_REL:
        alt_l = 0;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ALT_RIGHT:
        alt_r = 1;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ALT_RIGHT_REL:
        alt_r = 0;
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ENTER: //pressed enter key
        putc(s_code_table[28]);
        if(buffer_index[cur_term] < BUFFER_SIZE) {
          key_buffer[cur_term][buffer_index[cur_term]] = '\n';
          buffer_index[cur_term]++;
        }
        /* Pass buffer */
        copy_to_terminal_buffer((char*)key_buffer[cur_term], buffer_index[cur_term]);
        hit_enter_key[cur_term] = 1;

        clear_buffer(cur_term);
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_ENTER_REL: //released enter key
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_BACKSPACE: //pressed enter key
        if(buffer_index[cur_term] > 0){
          buffer_index[cur_term]--;
          key_buffer[cur_term][buffer_index[cur_term]] = ' ';
          output = get_char(input);
          putc(output);
        }
        send_eoi(IRQ_KEYBOARD);
        return;
      case SC_BACKSPACE_REL: //released enter key
        send_eoi(IRQ_KEYBOARD);
        return;
      default:     
        //ctrl + L should clear screen
        if(ctrl == 1 && input == TAKETHATL && (shift_r||shift_l) != 1) {
          clear();
          reset_cursor(); //reset cursor to (0,0) since we cleared screen
          clear_buffer(cur_term);
          send_eoi(IRQ_KEYBOARD);
          return;
        }
        // check to see if key pressed is printable
        if(input <= SC_SPACE && ctrl != 1 && buffer_index[cur_term]<BUFFER_SIZE-1){
          output = get_char(input);
          key_buffer[cur_term][buffer_index[cur_term]] = output;
          buffer_index[cur_term]++;
          putc(output);
        }
        
        break;
    }
    
    
    if(alt_l || alt_r){
      send_eoi(IRQ_KEYBOARD);
      switch (input)
      {
      case SC_F1:
        //term 1 is index 0
        switch_term(0);
        break;
      case SC_F2:
        //term 2 is index 1
        switch_term(1);
        break;
      case SC_F3:
        //term 3 is index 2
        switch_term(2);
        break;
      
      default:
        break;
      }
    }
    // send end of interrupt signal
    send_eoi(IRQ_KEYBOARD);
}

/*
 * get_char(uint8_t s_code)
 * DESCRIPTION: converts scan code to ascii character
 * INPUT: scan_code
 * OUTPUT: none
 * RETURNS: ascii_character
 * SIDE EFFECTS: none
 */
uint8_t get_char(uint8_t s_code){
  //check both shifts
  int shift = (shift_l || shift_r);

  if(shift == 0 && caps == 0) //neither shift nor caps active
    return s_code_table[s_code];
  else if(shift == 0 && caps == 1) // caps active
    return s_code_table_caps[s_code];
  else if(shift == 1 && caps == 0) //shift active
    return s_code_table_shift[s_code];
  else //both active
    return s_code_table_why[s_code];
}

/*
 * clear_buffer(int term_num)
 * DESCRIPTION: clears appropriate keyboard buffer
 * INPUT: terminal number
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: buffer is cleared and index is reset
 */
void clear_buffer(int term_num){
  int i;
  for(i = 0; i < BUFFER_SIZE; i++){
    key_buffer[term_num][i] = ' ';
  }
  buffer_index[cur_term] = 0;
}
