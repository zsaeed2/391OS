#include "system_calls.h"

fops_t stdin_ops  =	{&bad_call_open, 	&terminal_read, &bad_call_write, 	 &bad_call_close};
fops_t stdout_ops = {&bad_call_open, 	&bad_call_read, &terminal_write,	 &bad_call_close};
fops_t rtc_ops    =	{&file_open, 		&rtc_read, 		&rtc_write,      	 &file_close};
fops_t file_ops   =	{&file_open, 		&file_read,		&bad_call_write,	 &file_close};
fops_t dir_ops    =	{&file_open, 		&dir_read,		&bad_call_write,	 &file_close};
fops_t bad_ops    = {&bad_call_open, 	&bad_call_read, &bad_call_write, 	 &bad_call_close};

int32_t cur_pid = -1;
int32_t cur_term = 0;
int32_t running_terms[NUM_TERMS] = {1,0,0};
pcb_t* pcb_ptr_array[MAX_PCBS] = {NULL};
/* Function to parse the typed buffer */
void parse_buff(const uint8_t* buff, uint8_t* command, uint8_t* args);

/* 
int32_t bad_call_open(const uint8_t* str) 
Description: Placeholder for a bad system call for open
Inputs: str - a string name
Returns: always -1
Side effects: None
*/
int32_t bad_call_open(const uint8_t* str) {
	return -1;
}
/* 
int32_t bad_call_close(int32_t fd) 
Description: Placeholder for a bad system call for close
Inputs: fd - a file descriptor
Returns: always -1
Side effects: None
*/
int32_t bad_call_close(int32_t fd) {
	return -1;
}

/* 
int32_t bad_call_read(int32_t fd, void* buf, int32_t nbytes)
Description: Placeholder for a bad system call for read
Inputs: fd - a file descriptor
		buf - a buffer
		nbytes - num bytes to copy
Returns: always -1
Side effects: None
*/
int32_t bad_call_read(int32_t fd, void* buf, int32_t nbytes) {
	return -1;
}

/* 
int32_t bad_call_write(int32_t fd, const void* buf, int32_t nbytes)
Description: Placeholder for a bad system call for write
Inputs: fd - a file descriptor
		buf - a buffer
		nbytes - num bytes to copy
Returns: always -1
Side effects: None
*/
int32_t bad_call_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}



/*
 * halt_c()
 * DESCRIPTION: halts current process
 * INPUT: uint8_t status: return status of process
 * OUTPUT: n/a
 * RETURNS: n/a
 * SIDE EFFECTS: kills current process, closes its files, updates pcb_table,
 * 				 returns to parent process or executes shell
 */
int32_t halt_c (uint8_t status) {
	uint32_t  i;
	uint32_t j;
	int32_t term_in_use;
	// prevent sched from changes cur_pid
	cli();
	int32_t pid = cur_pid;

	if(pcb_ptr_array[cur_pid]->vidmap_ptr != NULL){
		vidmap_close();
	}

	int32_t parent_pid = pcb_ptr_array[pid]->parent_pid;

	for(i = 0; i< MAX_OPEN_FILES; i++){
		close_c(i);
	}

	pcb_ptr_array[pid] = NULL;

	cur_pid = parent_pid;

	if(cur_pid == -1){
		uint8_t *shell = (uint8_t*)"shell";
		execute_c(shell);
	}

	for(i = 0; i < NUM_TERMS; i++){
		if(running_terms[i]){
			term_in_use = 0;
			// there should be at least 1 process with term_id == i
			for(j = 1; j < MAX_PCBS; j++){
				if(pcb_ptr_array[j] != NULL){
					if(pcb_ptr_array[j]->term_id == i){
						term_in_use = 1;
						break;
					}
				}
			}
			// if term should be running but no process, edit new pcb to that term
			if(!term_in_use){
				uint8_t *shell = (uint8_t*)"shell";
				execute_c(shell);
			}
		}
	}


	//update PTE for 128MB
	new_task_page(parent_pid);

	// set parent to have no children
	pcb_ptr_array[cur_pid]->isParent = 0;

	tss.esp0 = pcb_ptr_array[cur_pid]->kernel_esp;

	sti();
	//JMP to execute return
	asm volatile(
		"movl %0, %%esp \n"
		"movl %1, %%ebp \n"
		"movl %2, %%eax \n"
		"jmp RETURN_ON_HALT"
		::"r"(pcb_ptr_array[parent_pid]->user_esp), "r"(pcb_ptr_array[parent_pid]->user_ebp), "r"((uint32_t)status)
		: "esp", "ebp", "eax"
	);

	return 0;
}
/*
 * execute_c
 * DESCRIPTION: executes current process
 * INPUT: uint8_t* command: command to be executed
 * OUTPUT: depends which command is called
 * RETURNS: 0 on success, -1 on failure
 * SIDE EFFECTS: executes current process
 */
int32_t execute_c (const uint8_t* command) {
	int i;
	int j;
	int term_in_use;
	uint8_t cmd[BUF_LEN] = {0};
	uint8_t args[BUF_LEN] = {0};
	uint8_t buf[NUM_OF_MAGIC_NUMBERS];

	if(command == NULL) {
		return -1;
	}

	// critical section added to avoid switching terms while editing global variables
	cli();

	parse_buff(command,cmd,args);

	dentry_t file;

	int32_t e = read_dentry_by_name(cmd, &file);
	if(e == -1) {
		//printf("File not found.");
		return -1;
	}
	e = read_data(file.inode_index, 0, buf, NUM_OF_MAGIC_NUMBERS);

	if(buf[0] != MAGIC_NUM_0 || buf[1] != MAGIC_NUM_1 || buf[2] != MAGIC_NUM_2 || buf[3] != MAGIC_NUM_3){
		return -1;
	}

	/* Start process */
	int32_t new_pid = init_pcb(args, strlen((int8_t*)args));

	/* Couldn't make a PCB */
	if(new_pid == -1) {
		printf("Too many running processes.");
		return -1;
	}

	// save cmd in cb for debugging purposes
	memcpy(pcb_ptr_array[new_pid]->cmd,cmd,strlen((int8_t*)cmd));

	if(!strncmp((int8_t*)cmd,(int8_t*)"shell",strlen((int8_t*)cmd))){
		/* Check if the new process should be in a new term by checking term_running and looping over the pcbs*/
		for(i = 0; i < NUM_TERMS; i++){
			if(running_terms[i]){
				term_in_use = 0;
				// there should be at least 1 process with term_id == i
				for(j = 1; j < MAX_PCBS; j++){
					if(pcb_ptr_array[j] != NULL){
						if(pcb_ptr_array[j]->term_id == i){
							term_in_use = 1;
							break;
						}
					}
				}
				// if term should be running but no process, edit new pcb to that term
				if(!term_in_use){
					pcb_ptr_array[new_pid]->term_id = i;
					pcb_ptr_array[new_pid]->parent_pid = -1;
				}
			}
		}
	}
	/* Create new page for process */
	new_task_page(new_pid);

	/* Copy program data */
	uint32_t num_bytes_copied = (uint32_t)read_data(file.inode_index, 0, (uint8_t*)MB128, file.file_size);
	if(num_bytes_copied != file.file_size) {
		printf("Error copying program data.");
		return -1;
	}

	/* Context switch */

	//save parent's values, if exists
	if(cur_pid > -1) {
		pcb_t* cur_pcb = pcb_ptr_array[cur_pid];
		// if the new process is on the same term, cur process must be a parent of the new process
		if(pcb_ptr_array[new_pid]->term_id == cur_pcb->term_id){	
			cur_pcb->isParent = 1;
		}
		
		asm volatile("movl %%esp, %0 	\n"
					 "movl %%ebp, %1 	\n"
					: "=r"(cur_pcb->user_esp),
					  "=g"(cur_pcb->user_ebp)
		);

	}

	/* Update PCB data */
	cur_pid = new_pid;
	pcb_t* new_pcb = pcb_ptr_array[new_pid];
	//esp points to bottom of PCB data segment
	tss.esp0 = new_pcb->kernel_esp;
	tss.ss0 = KERNEL_DS;

	/* Return to executable (IRET) */

	/* Get instruction pointer from file */
	e = read_data(file.inode_index, 24, buf, 4);
	if (e != 4) {
		printf("Couldn't load instruction pointer of file.");
		return -1;
	}

	uint32_t eip = (((uint32_t) buf[3]) << 24 | ((uint32_t) buf[2]) << 16 | ((uint32_t) buf[1]) << 8 | ((uint32_t) buf[0]));

	sti();
	/* Modify stack for faux IRET */

	asm volatile("          \n\
		cli 				\n\
		movw  %0, %%ax      \n\
		movw %%ax, %%ds		\n\
		pushl %0			\n\
		pushl %1			\n\
		pushfl          	\n\
		popl %%eax			\n\
		orl $0x200, %%eax   \n\
		pushl %%eax			\n\
		pushl %2			\n\
		pushl %3			\n\
		iret 				\n\
		RETURN_ON_HALT:     \n\
		"
		:
		: "g"(USER_DS), "g"(new_pcb->user_esp), "g"(USER_CS), "g"(eip)
		: "eax", "memory"
	);

	/* Return to user */
	return 0;
}
/*
 * read_c
 * DESCRIPTION: reads contents of file
 * INPUT: file directory, buffer, and number of bytes
 * OUTPUT: none
 * RETURNS: -1 on failure
 * SIDE EFFECTS: reads file
 */
int32_t read_c (int32_t fd, void* buf, int32_t nbytes) {
	/* Sanity checks */
	if(fd < 0 || fd >= MAX_OPEN_FILES)
		return -1;
	if(buf == 0) {
		return -1;
	}
	if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0)
		return -1;
	if(nbytes < 0) {
		return -1;
	}

	//get pcb array

	return (pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table.read)(fd, buf, nbytes);
}

/*
 * write_c
 * DESCRIPTION: writes to terminal
 * INPUT:file directory, buffer, and number of bytes
 * OUTPUT: text to terminal
 * RETURNS: -1 on failure
 * SIDE EFFECTS: writes to terminal
 */
int32_t write_c (int32_t fd, const void* buf, int32_t nbytes) {
	/* Sanity checks */
	if(fd < 0 || fd >= MAX_OPEN_FILES)
		return -1;
	if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0)
		return -1;
	if(buf == 0) {
		return -1;
	}
	if(nbytes < 0) {
		return -1;
	}
	return (pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table.write)(fd, buf, nbytes);
}
/*
 * open_c
 * DESCRIPTION: calls file_open via a jump table
 * INPUT: file name
 * OUTPUT: none
 * RETURNS: -1 on failure, name of fd on success
 * SIDE EFFECTS: opens up file
 */
int32_t open_c (const uint8_t* file_name) {
	
    return file_ops.open(file_name);
}
/*
 * close_c
 * DESCRIPTION: calls close_file via the file descriptor
 * INPUT: file directory
 * OUTPUT: none
 * RETURNS: -1 on failure, 0 on success
 * SIDE EFFECTS: closes file
 */
int32_t close_c (int32_t fd) {
		
	// sanity checks
    if(cur_pid == INVALID_ENTRY || cur_pid >= MAX_PCBS){
        printf("\nERROR: No process running. Can't open file.");
        return -1;
    }
    else if(pcb_ptr_array[cur_pid] == NULL){
        printf("\nERROR: PCB not initialized. Can't open file.");
        return -1;
    }

	if(fd < 0 || fd >= MAX_OPEN_FILES || fd == 1 || fd == 0)
		return -1;

    else if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0){ // a file is not open in fd
        return -1;
    }
    return (pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table.close)(fd);
}
/*
 * vidmap_c
 * DESCRIPTION: writes to video memory
 * INPUT: double pointer to start of screen
 * OUTPUT: none
 * RETURNS: -1 on failure, 0 on success
 * SIDE EFFECTS: allocates a new page to write to video memory
 */
int32_t vidmap_c(uint8_t** screen_start) {

	/*sanity checks*/
	if(screen_start == NULL){
		return -1;
	}

	if((uint32_t)screen_start < MB128 || (uint32_t)screen_start > MB132){
		return -1;
	}

	cli();
	
	//helper function in paging
	*screen_start = vidmap_init();
	//created flag to test if writing to screen
	pcb_ptr_array[cur_pid]->vidmap_ptr = *screen_start;

	sti();
	return 0;
}

/*
 * getargs_c (uint8_t* buf, int32_t nbytes)
 * DESCRIPTION: Copies arguments into userspace
 * INPUT: buf - buffer to copy into
 		  nbytes - number of bytes to copy
 * OUTPUT: n/a
 * RETURNS: -1 if failure
 			0 if success
 * SIDE EFFECTS: n/a
 */

int32_t getargs_c (uint8_t* buf, int32_t nbytes) {
	int32_t len, i;
	uint8_t* input;
	if((uint32_t)buf < MB128 || (uint32_t)buf > MB132)
		return -1;

	if(buf == NULL || nbytes <= 0) return -1;

	input = pcb_ptr_array[cur_pid]->args;

  //Step 1: Get length
    len = 0;
    while(input[len] != '\0') {
        len++;
    }

	for(i =0; i<nbytes; i++) {
		buf[i] = '\0';
	}
	if(len == 0)
		return -1;

    //Copy to userspace
    if(len > nbytes) len = nbytes;
    memcpy(buf, input, len);
    return 0;
    //return copy_to_user(buf, &input + str_start, nbytes);

}

/*
 * parse_buff()
 * DESCRIPTION: processes buff str
 * INPUT: char* buff: sting containting buff
 * 		  char* command: destination of command name
 * 		  char* args:    destination of args
 * OUTPUT: command: contains name of command
 * 		   args:    contains args with leading spaces removed
 * RETURNS: none
 * SIDE EFFECTS:none
 */
void parse_buff(const uint8_t* buff, uint8_t* command, uint8_t* args){
	int i = 0;
	int j = 0;
	int k = 0;
	int space_counter = 0;
	uint32_t buff_length = strlen((int8_t*)buff);

	if(buff == NULL || command == NULL || args == NULL) return;

	//remove all initial spaces
	while(buff[i] == ' ' && i < buff_length){
		i++;
	}
	j = i;
	//read buff
	while(buff[i] != '\n' && buff[i] != ' ' && buff[i] != '\0'){
		i++;
	}

	//copy command with NULL term at end
	for(; j < i; j++){
		command[k] = buff[j];
		k++;
	}
	command[k] = '\0';

	//remove all initial spaces
	while(buff[i] == ' ' && i < buff_length){
		i++;
	}
	k=0;
	//copy args with NULL term at end, no leading spaces
	while(i < buff_length){
		if(buff[i] == ' ' ){
			if(space_counter == 0){
				args[k] = buff[i];
				space_counter++;
			}else{
				k++;
			}
		}
		else{
			args[k] = buff[i];
			space_counter = 0;
		}
		k++;
		i++;
	}
	i = 0;
    while(args[i] != '\0') {
        i++;
    }
	i--;
	if(args[i] == ' ') {
		args[i] = '\0';
	}
	/*
	i--;
	//delete trailing spaces
    if( i > 0 && args[i] == ' '){
		i--;
	}
	*/
	//null terminate
	args[++i] = '\0';
}

/*
    function initializes new PCB.
    Initializes an empty file descriptor array.
    sets the parent process to current process.
    sets pid to next available pid in pcb_ptr_array
    not sure what to do yet with esp and other vales.
    return value of pid or returns -1 for failure.
    INPUT: string buf and int length to copy over args to the PCB
    OUTPUT: pid of the new process 0-5 or -1 for failure
    SIDEEFFECT: Allocates space in kernel memory for a PCB struct
        and edits global pcb_ptr_array
*/
int32_t init_pcb(uint8_t* buf, int32_t len){
    int i;
    int32_t new_pid;
	if(buf == NULL || len < 0)
		return -1;
    // check for a valid pid entry
    for(i = 0; i < MAX_PCBS; i++){
        if(pcb_ptr_array[i] == NULL){
            break;
        }
        else if(i == MAX_PCBS - 1){
            return -1;
        }
    }
    // set the new processes pid
    new_pid = i;
    // initialize the new entry in the array in kernel memory
    pcb_ptr_array[new_pid] = (pcb_t*) (MB8 - (new_pid+1)*KB8);
    pcb_ptr_array[new_pid]->pid = new_pid;
    pcb_ptr_array[new_pid]->parent_pid = cur_pid;
	pcb_ptr_array[new_pid]->term_id = cur_term;

    // initialize the file descriptor array
    for(i = 0; i < MAX_OPEN_FILES; i++){
        if(i == 0){
            pcb_ptr_array[new_pid]->file_desc_array[i].ops_table = stdin_ops;
            pcb_ptr_array[new_pid]->file_desc_array[i].inode_index = -1;
            pcb_ptr_array[new_pid]->file_desc_array[i].file_position = 0;
            pcb_ptr_array[new_pid]->file_desc_array[i].flags = 1;
        }
        else if(i == 1){
            pcb_ptr_array[new_pid]->file_desc_array[i].ops_table = stdout_ops;
            pcb_ptr_array[new_pid]->file_desc_array[i].inode_index = -1;
            pcb_ptr_array[new_pid]->file_desc_array[i].file_position = 0;
            pcb_ptr_array[new_pid]->file_desc_array[i].flags = 1;
        }
        else{
            pcb_ptr_array[new_pid]->file_desc_array[i].ops_table = bad_ops;
            pcb_ptr_array[new_pid]->file_desc_array[i].inode_index = -1;
            pcb_ptr_array[new_pid]->file_desc_array[i].file_position = 0;
            pcb_ptr_array[new_pid]->file_desc_array[i].flags = 0;
        }
    }

    // copy the args buffer over to the pcb
    for(i = 0; i < len && i < BUF_LENGTH; i++){
        pcb_ptr_array[new_pid]->args[i] = buf[i];
    }
    pcb_ptr_array[new_pid]->args[i++] = '\0';

    pcb_ptr_array[new_pid]->kernel_esp = MB8 - new_pid*KB8 - aligned_1;
    pcb_ptr_array[new_pid]->user_esp = MB132 - aligned_1;
    pcb_ptr_array[new_pid]->user_ebp = MB132 - aligned_1;
	pcb_ptr_array[new_pid]->vidmap_ptr = NULL;
	pcb_ptr_array[new_pid]->error_flag = 0;
	pcb_ptr_array[new_pid]->isParent = 0;
    return new_pid;
}

/*
 * file_open
 * DESCRIPTION: Attempts to open new file by checking the PCB and its file descriptor array.
 * 	Should only be called via that file ops tables
 * INPUT: string file_name
 * OUTPUT: int fd
 * RETURNS: -1 on failure, int fd on success
 * SIDE EFFECTS: edits current PCB and its file descriptor array
 */
int32_t file_open (const uint8_t* file_name) {
	int i;
    int e;
    int32_t fd;
    dentry_t file;

	if(file_name == NULL || !strlen((int8_t*)file_name)) {
		return -1;
	}

    for(i = 2; i < MAX_OPEN_FILES; i++){
        if(pcb_ptr_array[cur_pid]->file_desc_array[i].flags == 0){
            break;
        }
        // no file entries left
        else if(i == MAX_OPEN_FILES-1){
            return -1;
        }
    }
    // i contains index of empty file_desc_array entry
    fd = i;
    if(0 != (e = read_dentry_by_name(file_name, &file))){
        return e;
    }
    // check type of file to set file descriptor entries
    if(file.file_type == FILE_TYPE){
        pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table = file_ops;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].inode_index = file.inode_index;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position = 0;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].flags = 1;
    }
    else if(file.file_type == DIR_TYPE){
        pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table = dir_ops;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].inode_index = INVALID_ENTRY;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position = 0;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].flags = 1;
    }
    else if(file.file_type == RTC_TYPE){
        pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table = rtc_ops;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].inode_index = INVALID_ENTRY;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position = INVALID_ENTRY;
        pcb_ptr_array[cur_pid]->file_desc_array[fd].flags = 1;
        rtc_open(file_name);
    }
    else{
        printf("\nERROR: File type unrecognized.\nFile Type: %d", file.file_type);
        return -1;
    }

    return fd;
}

/*
 * file_close
 * DESCRIPTION: closes file
 * INPUT: file descriptor entry
 * OUTPUT: int 
 * RETURNS: -1 on failure, 0 on success
 * SIDE EFFECTS: closes file
 */
int32_t file_close (int32_t fd) {
	// sanity checks
	if(fd < 0 || fd >= MAX_OPEN_FILES || fd == 1 || fd == 0)
		return -1;
    if(cur_pid < 0 || cur_pid >= MAX_PCBS){     // cur_pid is not junk value
        return 0;
    }
    else if(pcb_ptr_array[cur_pid] == NULL){    // process has been initialized
        return 0;
    }
    else if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0){ // a file is not open in fd
        return -1;
    }

	// clear values of the pcb, set flag = 0 (not allocated)
    pcb_ptr_array[cur_pid]->file_desc_array[fd].ops_table = bad_ops;
    pcb_ptr_array[cur_pid]->file_desc_array[fd].inode_index = INVALID_ENTRY;
    pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position = INVALID_ENTRY;
    pcb_ptr_array[cur_pid]->file_desc_array[fd].flags = 0;
    return 0;
}
