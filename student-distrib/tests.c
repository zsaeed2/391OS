#include "tests.h"
//#include "x86_desc.h"
//#include "lib.h"
//#include "paging_asm.h"
//#include "paging.h"
//#include "fs.h"
//#include "rtc.h"
//#include "terminal.h"
//#include "keyboard.h"
#include "system_calls.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide by 0 test
 *
 * divides by 0 to see if divide by 0 exception occurs
 * Inputs: None
 * Outputs: None
 * Side Effects: displays BSOD with divide by 0 exception (if test passes)
 * Coverage: IDT, interrupts
 * Files: interrupts.h/c
 */
// Should display a divide by 0 exception with BSOD
void divide_by_0_test(){
	int x = 0;
	printf("Testing divide by 0 exception\n");
	int i = 42/x; //dividing by 0
	printf("FAIL. i = %d", i); //prints if test fails
}

/* Paging test 1
 *
 * Prints all values of page table and page directory that are not set to initialized values
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints all values actually set
 * Coverage: Paging
 * Files: Paging.h/S/c
 */
// Outputs all values are not set to "NOT PRESENT" for page_directory
// and all values not set to the scaled value for the table
void page_test1() {
	int i;
	for(i = 0; i < 1024; i++) {
		if(page_table[i] != (NOT_PRESENT | (i * SCALE))) {
			printf("table:%x %x \n",i, page_table[i]);
		}
		if(page_directory[i] != NOT_PRESENT ) {
			printf("directory:%x %x \n",i, page_directory[i]);
		}
	}
	printf("If values are 02020202, then PASS.");
}

/* Paging test 2
 *
 * Derefences NULL address to see if page fault exception occurs
 * Inputs: None
 * Outputs: None
 * Side Effects: displays BSOD with page fault exception (if test passes)
 * Coverage: Paging
 * Files: Paging.h/S/c
 */
//Tests to see what happens if we dereference NULL address
//Test passes if BSOD appears with page fault exception
void page_test2() {
	printf("Testing dereferencing of page part 1\n");
	uint32_t* nul = (uint32_t*) NULL; //null address
	uint32_t temp = *nul;
	printf("FAIL. temp = %d", temp); //prints if test failed
}

/* Paging test 2
 *
 * Derefences address in vid mem to see if page fault exception occurs
 * Inputs: None
 * Outputs: None
 * Side Effects: displays text "PASSED" if test case passes
 * 							 (dereferencing video memory should not create an exception
 *							 since we have already allocated space for it)
 * Coverage: Paging
 * Files: Paging.h/S/c
 */
//Tests to see if we can dereference a page in video memory.
//Should not produce a page fault exception.
void page_test3() {
	printf("Testing dereferencing of page part 2\n");
	uint32_t* vid_mem = (uint32_t*) 0x696969; //in vid mem
	uint32_t temp = *vid_mem;
	printf("PASSED. temp = %d", temp); //prints if test passed
}

/* Paging test 3
 *
 * Derefences address outside virtual memory to see if page fault exception occurs
 * Inputs: None
 * Outputs: None
 * Side Effects: displays BSOD with page fault exception (if test passes)
 * Coverage: Paging
 * Files: Paging.h/S/c
 */
// TEST should cause page fault for being outside defined virtual memory
// test passes if BSOD appears with page fault exception
void page_test4() {
	printf("Testing dereferencing of page part 3\n");
	uint32_t* out_of_bounds = (uint32_t*) 0x1234567; //outside bounds of virtual mem
	uint32_t temp = *out_of_bounds;
	printf("FAIL. temp = %d", temp); //prints if test failed
}


/* Checkpoint 2 tests */
/*
 * Function tests all read and open functionalities for our single directory
 * Tests: opening correct and incorrect directory names, passing bad parameters, 
 * and prints number of files and all file names in boot block 
*/
/*
void dir_test(){
	uint8_t buf[B32*4];
	int32_t count;

	if(-1 == open_c((uint8_t*)"Random name")){
		printf("\n PASS BAD DIRECTORY NAME TEST");
	}
	else{
		printf("\n FAIL BAD DIRECTORY NAME TEST");
	}

	if(-1 == open_c((uint8_t*)".")){
		printf("\n FAILED DIRECTORY CORRECT NAME TEST");
	}
	else{
		printf("\n PASSED DIRECTORY CORRECT NAME TEST");
	}
	if(0 != dir_read(buf, -10)){
		printf("\n FAILED BAD NUMBER BYTES TO COPY TEST");
	}
	else{
		printf("\n PASSED BAD NUMBER BYTES TO COPY TEST");
	}
	printf("\n\n Number of Directory entires: %d", fs.num_dir_entries);
	while(0 != (count = dir_read(buf,B32*4))){
		buf[count] = '\0';
		printf("\n %d FILE NAME: %s", fs.dir_cursor,buf);
	}
	// clear screen to avoid page fault
	clear();
	printf("\n END DIRECTORY TEST.");
	return;
}
*/
/*
 * Function tests all read and open functionalities for a file less than the size of one
 * data block. 
 * Tests: opening correct and incorrect file names, trying to open more than one file(not allowed at cp2),
 * correct number of bytes copied, correct bytes copied, and end of file. 
*/
/*
void file_test_one_block(){
	uint8_t buf[KB4*4];
	uint32_t bytes_copied;
	clear();
	printf(" START OF SINGLE DATA BLOCK FILE TEST.");
	if(-1 != open_c((uint8_t*)"Random Name")){
		printf("\n FAILED BAD FILE NAME TEST.");
	}
	else{
		printf("\n PASSED BAD FILE NAME TEST.");
	}
	if(0 != open_c((uint8_t*)"frame0.txt")){
		printf("\n FAILED GOOD FILE NAME TEST.");
	}
	else{
		printf("\n PASSED GOOD FILE NAME TEST.");
	}

	//clear();
	bytes_copied = file_read(buf, KB4*4);
	buf[bytes_copied] = '\0';

	if(bytes_copied != fs.open_file.file_size){
		printf("\n FAILED copy whole file test.");
		printf("\n Bytes copied: %d	Bytes in file: %d", bytes_copied, fs.open_file.file_size);
	}
	else{
		if(!strncmp((int8_t*)(fs_ptr + KB4*(1+fs.num_inodes) + KB4*(*(fs_ptr + KB4*(1+fs.open_file.inode_index) + B4))),(int8_t*)buf, bytes_copied)){
			printf("\n PASSED copy whole file test.");
		}
		else{
			printf("\n FAILED copy whole file test. copied bytes number is correct, but strings do not match.");
		}
	}
	if(0 != (bytes_copied = file_read(buf, KB4*4))){
		printf("\n FAILED end of file test.");
	}
	else{
		printf("\n PASSED END OF FILE TEST.");
	}
	printf("\n %s",buf);
	printf("\n END OF SINGLE DATA BLOCK FILE TEST.");
	//clear();
	file_close();
}
*/
/*
 * Function test copying data from multi block files. Uses fish file since it is largest.
 * checks that correct number of bytes are copied and checks correctness using strncmp
*/
/*
void file_test_multi_block(){
	uint32_t strncmp_sum = 0;
	uint32_t file_size;
	int32_t bytes_copied;
	uint32_t* data_block_index_ptr;
	uint32_t* data_block_ptr;
	uint8_t* buf_ptr;
	
	clear();
	printf(" START OF MULTI DATA BLOCK FILE TEST.");

	if(0 != file_open((uint8_t*)"fish")){
		printf("\n FAILED to open file fish.");
		file_size = 0;
	}
	else{
		printf("\n Opened file of name fish.");
		file_size = fs.open_file.file_size;
	}
	
	uint8_t buf[file_size];
	bytes_copied = (int32_t)file_read(buf, file_size);
	buf[bytes_copied] = '\0';

	if(bytes_copied != fs.open_file.file_size){
		printf("\n FAILED copy whole file test.");
		printf("\n Bytes copied: %d	Bytes in file: %d", bytes_copied, fs.open_file.file_size);
	}
	else{
		printf("\n PASSED copying correct number of bytes test.");
	}
	
	data_block_index_ptr = fs_ptr + KB4 + KB4*fs.open_file.inode_index + B4;
	buf_ptr = buf;

	while(bytes_copied > 0){
		data_block_ptr = fs_ptr + (1 + fs.num_inodes + *data_block_index_ptr)*KB4;
		if(bytes_copied > KB4*4){
			strncmp_sum += (uint32_t)strncmp((int8_t*)buf_ptr, (int8_t*)data_block_ptr, KB4*4);
		}
		else{
			strncmp_sum += (uint32_t)strncmp((int8_t*)buf_ptr, (int8_t*)data_block_ptr, bytes_copied);
		}
		data_block_index_ptr++;
		buf_ptr += KB4*4;
		bytes_copied -= KB4*4;
	}
	if(strncmp_sum != 0){
		printf("\n FAILED multi data block copy test.\nBytes off = %d", strncmp_sum);
	}
	else
	{
		printf("\n PASSED multi block data comparison test.");
	}
	printf("\n END OF MULTI DATA BLOCK FILE TEST.");
}
*/
/* RTC Test
 *
 * tests read and write functions to rtc
 * Inputs: None
 * Outputs: None
 * Side Effects: displays charaacters with increaasing frequency
 * Coverage: RTC, Interrupts
 * 
 */
void rtc_test1(){
	
	int test = 0;
	uint32_t frequency = 8;

	clear();
	//test open
	test += rtc_write(NULL,&frequency, 4);
	test += rtc_read(NULL, NULL, NULL);
	if(test == -2){
		printf(" PASSED rtc unusable till opened\n");
	}else{
		printf(" FAILED rtc usable before opened\n");
	}
	rtc_open(NULL);

	//test null buf
	test = rtc_write(NULL,NULL, 4);
	if(test == 0){
		printf(" FAILED NULL pointer passed\n");
	}else{
		printf(" PASSED NULL pointer leads to failure\n");
	}
	//test bad size
	test = rtc_write(NULL,&frequency, 2);
	if(test == 0){
		printf(" FAILED bad nbytes passed\n");
	}else{
		printf(" PASSED bad nybytes leads to failure\n");
	}

	//test bad frequency
	frequency = 0;
	test = rtc_write(NULL,&frequency, 4);
	if(test == 0){
		printf(" FAILED frequency %d passed\n");
	}else{
		printf(" PASSED frequency %d leads to failure\n");
	}
	frequency = 15;
	test = rtc_write(NULL,&frequency, 4);
	if(test == 0){
		printf(" FAILED frequency %d passed\n");
	}else{
		printf(" PASSED frequency %d leads to failure\n");
	}
	frequency = 2048;
	test = rtc_write(NULL,&frequency, 4);
	if(test == 0){
		printf(" FAILED frequency %d passed\n");
	}else{
		printf(" PASSED frequency %d leads to failure\n");
	}	
}

/*
* Function test: tests frequency of rtc interrupt by printing out
* keys in a frequency given by the paramter (must be in a power of 2)
*/
void rtc_test2(int freq){
	uint32_t frequency = freq;
	int counter = 0;
	int sec = 0;
	clear();
	rtc_open(NULL);
	rtc_write(NULL, &frequency, 4);
	clear();
	while(sec < 10){
		printf("Frequency: %d   Seconds: %d \n", frequency, sec);
		while(counter < frequency){
			rtc_read(NULL, NULL, NULL);
			counter++;
			putc('1');
		}
		sec++;
		counter = 0;
		clear();
	}
	printf("\nEND OF RTC2 TEST.");
	
}

/*
* terminal_write test
* Function test: tests to see if number of bytes written to terminal is equal to
* the expected number of bytes. prints passed if passed and failed if failed
*/
void test_terminal_write() {
    int32_t cnt;
	char* string = "Hi! This is a string I made to test the functionality of the terminal write function! If you're seeing this, then something might've worked!";
	//char* str = "Hi! This is another string I made to test the functionality of the terminal write function!";

	if(strlen(string) != (cnt = terminal_write (1, string, strlen(string)))){
		printf("\nFAILED NUMBER OF BYTES WRITTEN TEST.\nNumber of byte: %d   strlen: %d", cnt, strlen(string));
	}
	else{
		printf("\nPASSED NUMBER OF BYTES WRITTEN TEST.");
	}
	
	//terminal_write (1, str, strlen(str));

}

/*
*terminal_read test
* type something into terminal. should echo it back along with the number
* of bytes copied. quits out of this test only if the letter q is passed
*/
void test_terminal_read() {
	//int i;
	int32_t cnt;
	int buf_length = 127;
    char buf[buf_length];
	char* str = "The following is a terminal read test. Inputs will be echoed to terminal. Pass q to quit.\n";
	
	terminal_write(1,str, strlen(str));
	while(1){
		cnt = terminal_read(0, buf, buf_length);
		buf[cnt] = '\0';

		if(buf[0] == 'q' && buf[1] =='\0'){
			printf("\n End of test.");
			return;
		}
		printf("\nNumber of bytes copied: %d\n",cnt);
		terminal_write(1,buf, strlen(buf));
	}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("test_exceptions", test_exceptions());
	//dir_test();
	//file_test_one_block();
	//file_test_multi_block();
	// launch your tests here
	//rtc_test();
	//test_terminal_read();
	//test_terminal_write();
	//rtc_test1();
	//rtc_test2(32);
}
