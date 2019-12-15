#include "terminal.h"


/* int32_t terminal_open (const uint8_t* filename);
 * Inputs: filename - unused
 * Return Value: none
 * Function: Initializes the terminal */
int32_t terminal_open (const uint8_t* filename) {
	int i;

	/* Empty the buffer */
	for(i=0; i<BUF_LENGTH; i++) {
		terminal_buf[i] = 0;
	}

	/* Clear the terminal */
	clear();
	reset_cursor(); //reset cursor to (0,0)

	/* Initialize the keyboard */
	keyboard_init();

	return 0;
}


/* int32_t terminal_write (int32_t fd, const void* given_buf, int32_t length);
 * Inputs: int32_t fd - stdin(1) / stdout(0)
           const void* given_buf - the buffer that is printed to the screen
           int32_t length - number of bytes to copy
 * Return Value: none
 * Writes to the screen from buf, return # bytes written */
int32_t terminal_write (int32_t fd, const void* given_buf, int32_t length) {

	char* buf = (char*) given_buf;
	int i;

	/* We expect stdout */
	if(fd != 1) return -1;

	/* Sanity checks */
	if(length < 0) return -1;
	if(given_buf == NULL) return -1;


	for(i=0; i<length; i++) {
		putc_syscall(buf[i]);
	}
	//putc('\n');
	return i;
}

/* int32_t terminal_write (int32_t fd, const void* given_buf, int32_t length);
 * Inputs: int32_t fd - stdin(1) / stdout(0)
           const void* given_buf - the buffer where input is copied
           int32_t length - number of bytes to copy
 * Return Value: none
 * Reads from keyboard buffer to buf, return # bytes read */
int32_t terminal_read (int32_t fd, void* given_buf, int32_t length) {
	char* buf = (char*) given_buf;

	/* We expect stdin */
	if(fd != 0) return -1;

	/* Sanity checks */
	if(length < 0) return -1;
	if(given_buf == NULL) return -1;


	//wait until we hit the enter key to copy
	hit_enter_key[pcb_ptr_array[cur_pid]->term_id] = 0;
	while(!hit_enter_key[pcb_ptr_array[cur_pid]->term_id]) {}
	//then copy

	int i; //number of bytes read
	for (i=0; i<length && i<BUF_LENGTH; i++) {
		buf[i] = terminal_buf[i];
		/* Stop copying when newline is hit */
		if(terminal_buf[i] == '\n') {
			return i+1;
		}
	}
	return i+1;
}


/* int32_t terminal_write (int32_t fd, const void* given_buf, int32_t length);
 * Inputs: int32_t fd - stdin(1) / stdout(0)
           const void* given_buf - the buffer where input is copied
           int32_t length - number of bytes to copy
 * Return Value: none
 * Reads from keyboard buffer to buf, return # bytes read */
int32_t copy_to_terminal_buffer(char* key_buf, int32_t length) {

	/* Sanity check */
	if(length < 0) return -1;
	if(key_buf == NULL) return -1;

	/* Copy keyboard */
	int i;
	for(i=0; i<length; i++) {
		terminal_buf[i] = key_buf[i];
	}

	return length;
}


/* int32_t terminal_close (const uint8_t* filename);
 * Inputs: filename - unused
 * Return Value: none
 * Function: Closes the terminal */
int32_t terminal_close (int32_t fd) {
	return 0;
}


/* void clear_screen (void);
 * Inputs: None
 * Return Value: none
 * Function: Clears the screen */
void clear_screen() {
	clear();
	update_cursor();
}

/*
 * Function switches between the 3 terminals given a terminal to switch to. This is done using 3 terminal pages
 * each of size 4KB. When a switch is done, all the processes using vidmap must be mapped to the respective term page
 * INPUT: int term_id of end terminal
 * OUTPUT: NONE
 * SIDEEFFECTS: Edits global cur_term, moves data to and from video memory
*/
int32_t switch_term(int32_t term_id){

	int e;
	// prevent interrupts while editing video memory and cur_term

	cli();
	if(term_id == cur_term || term_id > 2 || term_id < 0){
		sti();
		return -1;
	} 

	int32_t old_term_id = cur_term;

	cur_term = term_id;			// update cur_term

	// note that KB4 is multiplied by 4 since it was defined for a 4B pointer
	// memcopy from video mem to the old term
	memcpy((uint8_t*)(VID_ADDR + KB4*4*(old_term_id + 1)), (uint8_t*)VID_ADDR, KB4*4);

	// runs if terminal is already running a process
	if(running_terms[cur_term] == 1){
		// memcopy from term_id to video mem
		memcpy((uint8_t*)VID_ADDR, (uint8_t*)(VID_ADDR + KB4*4*(term_id + 1)), KB4*4);
		update_cursor();
		sti();						// allow interrupts

		// should call excute shell from here if process not already running on term
		return 0;
	}

	// if process is not running on terminal, need to execute shell
	else{
		clear();
		running_terms[cur_term] = 1;
		sti();
		update_cursor();
		e = execute_c((uint8_t*)"shell");
		if(e == -1){
			running_terms[cur_term] = 0;
		}
	}
	return 0;
}

