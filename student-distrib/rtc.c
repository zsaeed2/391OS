#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "system_calls.h"

/* 
 * rtc_init(void)
 * DESCRIPTION: intialises rtc
 * INPUT: none
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: enables rtc interrupt and slave line on PIC
 *               changes reg B on RTC to enable periodic interrupts
 */

volatile int interrupt_occurred[MAX_PCBS];
int opened[MAX_PCBS];
int frequencies[MAX_PCBS];
volatile int counts[MAX_PCBS];


/* void rtc_init();
 * Inputs: void
 * Return Value: none
 * Function: Init rtc */
void rtc_init(){
    uint8_t register_b;
    int i;
    for( i = 0; i< MAX_PCBS; i ++){     
        interrupt_occurred[i] = 0;
        opened[i] = 0;
        frequencies[i] = DEFAULT_FREQUENCY;
        counts[i] = MAX_FREQUENCY/DEFAULT_FREQUENCY;
    }

    //select Reg B and read
    outb(REG_B, RTC_PORT);
    register_b = inb(CMOS_PORT);

    //select Reg B and turn on
    outb(REG_B, RTC_PORT);
    outb(((register_b)|(TURN_ON_BIT)), CMOS_PORT);

    enable_irq(IRQ_SLAVE);      // enable the slave to interrupt master since it is responsible for RTC
    enable_irq(IRQ_RTC);        // enable the RTC interrupt line

    write_frequency(MAX_FREQUENCY);
}



/* void rtc_interrupt_handler(void);
 * Inputs: void
 * Return Value: none
 * Function: Runs interrupt for Real-Time clock */
void rtc_interrupt_handler(){
    //read from reg c so another interrupt can occur
    uint8_t junk;
    int i;

	outb(REG_C,RTC_PORT);
	junk = inb(CMOS_PORT);

    for(i = 0; i < MAX_PCBS; i++){
        counts[i]--;
        if( counts[i] <= 0){
            //reset count
            counts[i] = MAX_FREQUENCY/frequencies[i];
            //set flag for interrupt occuring
            interrupt_occurred[i] = 1;
            
        }
        
    }


    //renable PIC for RTC
	send_eoi(IRQ_RTC);

    
}

/* void rtc_read (int32_t fd, void* buf, int32_t nbytes);
 * Inputs: fd: file directory
 *         buf: pointer to data
 *         nbytes: number of bytes
 * Return Value: 0 for success
 *              -1 for failure
 * Return Value: none
 * Side Effects: returns zero after interrupt occurs
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){
    //check if rtc is opened
    if(opened[cur_pid] != 1){
        return -1;
    }

    //wait for interrupt
    while(interrupt_occurred[cur_pid] != 1);
    
    //reset flag
    interrupt_occurred[cur_pid] = 0;

    return 0;
}

/* void rtc_write (int32_t fd, void* buf, int32_t nbytes);
 * Inputs: fd: file directory
 *         buf: pointer to frequency
 *         nbytes: number of bytes
 * Outputs: void
 * Return Value: 0 for success
 *              -1 for failure
 * Side Effects: changes frequency of rtc to int in buf
 */

int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes){
    /*take 4 byte integer and write as frequency to RTC
      limit to 2024 HZ*/

    //check if rtc is opened
    if(opened[cur_pid] != 1){
        return -1;
    }

    uint32_t frequency;
    //check number if is four bytes
    if(nbytes != 4){
        return -1;
    }

    //check if buffer is null
    if(buf == NULL){
        return -1;
    }

    //read frequency 
    frequency = *((uint32_t*)buf);

    return set_frequency(frequency);
}

/* void rtc_open (const uint8_t* filename);
 * Inputs: filename: name of file
 * Outputs: none
 * Return Value: 0 for success!
 * Side Effects: allows rtc to written/read, sets rtc to default
 * frequency
 */
int32_t rtc_open (const uint8_t* filename){

    opened[cur_pid] = 1;
    interrupt_occurred[cur_pid] = 0;
    set_frequency(DEFAULT_FREQUENCY);
    return 0;
}

/* void rtc_close (int32_t fd);
 * Inputs: fd: file directory
 * Outputs: void
 * Return Value: none
 * Side Effects: none
 */
int32_t rtc_close (int32_t fd){
    return 0;
}

/* void set_frequency(uint32_t frequency);
 * Inputs: frequency: frequency in HZ
 * Outputs: 
 * Return Value: 0 for success
 *              -1 for failure
 * Side Effects: none
 */
int32_t set_frequency(uint32_t frequency){
    int rate;
    int log = 0;
    uint32_t shifted_frequency = frequency;

    if(frequency%2 !=0){
        return -1;
    }

    //calcuate rate from log2(frequncy)
    while(shifted_frequency != 0){
        
        if(((shifted_frequency & (0x01)) == 1) && shifted_frequency >1){
            return -1;
        }

        log++;
        shifted_frequency = frequency >> log;
        
    }

    rate = RATE_ADJUSTMENT - log;

    //limit rate to 1024 HZ - 2HZ
    if(rate < RATE_1024){
        return -1;
    }
    if(rate > RATE_2){
        return -1;
    }
 
    frequencies[cur_pid] = frequency;

    counts[cur_pid] = MAX_FREQUENCY/frequency;

    return 0;
}
/* void write_frequency(uint32_t frequency);
 * Inputs: frequency: frequency in HZ
 * Outputs: 
 * Return Value: 0 for success
 *              -1 for failure
 * Side Effects: none
 */
int32_t write_frequency(uint32_t frequency){
    int rate;
    int log = 0;
    int8_t reg_a;
    uint32_t shifted_frequency = frequency;

    if(frequency%2 !=0){
        return -1;
    }

    //calcuate rate from log2(frequncy)
    while(shifted_frequency != 0){
        
        if(((shifted_frequency & (0x01)) == 1) && shifted_frequency >1){
            return -1;
        }

        log++;
        shifted_frequency = frequency >> log;
        
    }

    rate = RATE_ADJUSTMENT - log;

    //limit rate to 1024 HZ - 2HZ
    if(rate < RATE_1024){
        return -1;
    }
    if(rate > RATE_2){
        return -1;
    }

    /*critical section: read/write rate to RTC*/
    disable_irq(IRQ_RTC);
    
    //read from reg_a
    outb(REG_A, RTC_PORT);
    reg_a = inb(CMOS_PORT);
    
    //set frequency
    reg_a = ((reg_a & RATE_MASK) | rate);

    //write to reg_a
    outb(REG_A, RTC_PORT);
    outb(reg_a, CMOS_PORT);

    enable_irq(IRQ_RTC);

    return 0;
}

