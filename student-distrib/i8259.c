/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */



/* 
 * i8259_init(void)
 * DESCRIPTION: intialises master and slave pic
 * INPUT: none
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: starts up master and slave pic
 *               masks all interrupts on the pic
 */

void i8259_init(void) {
    // close interrupts and save the flags
    uint32_t flags;
    cli_and_save(flags);

    /* initial mask will mask all interrupts, 0xFF masks all intterrupts*/
    master_mask = 0xFF;
    slave_mask = 0xFF;

    outb(master_mask, MASTER_8259_PORT2); /*mask all interupts to master*/
    outb(slave_mask, SLAVE_8259_PORT2); /*mask all interupts to slave*/

    /*send command words to slave and master PIC, first word to first port
      remaining words to second port */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT2);
    outb(ICW3_MASTER, MASTER_8259_PORT2);
    outb(ICW4, MASTER_8259_PORT2);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT2);
    outb(ICW3_SLAVE, SLAVE_8259_PORT2);
    outb(ICW4, SLAVE_8259_PORT2);

    /*resend mask to mask all interrupts*/
    outb(master_mask, MASTER_8259_PORT2); /*mask all interrupts on master and slave*/
    outb(slave_mask, SLAVE_8259_PORT2);

    // restore the flags
    restore_flags(flags);
}

/* 
 * enable_irq(uint32_t irq_num)
 * DESCRIPTION: enables irq on corrosponding pic
 * INPUT: irq_number: irq number
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: enables irq_num on PIC
 */
void enable_irq(uint32_t irq_num) {
    if(irq_num >= SLAVE_OFFSET){
        slave_mask = ~((~slave_mask) | irq_num_to_mask(irq_num-8));
        outb(slave_mask, SLAVE_8259_PORT2);
    }else{
        master_mask = ~((~master_mask) | irq_num_to_mask(irq_num));
        outb(master_mask, MASTER_8259_PORT2);
    }
}

/* 
 * disable_irq(uint32_t irq_num)
 * DESCRIPTION: disables irq on corrosponding pic
 * INPUT: irq_number: irq number
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: disables irq_num on PIC
 */
void disable_irq(uint32_t irq_num) {
    if(irq_num >= SLAVE_OFFSET){
        slave_mask = slave_mask | irq_num_to_mask(irq_num-SLAVE_OFFSET);
        outb(slave_mask, SLAVE_8259_PORT2);
    }else{
        master_mask = master_mask | irq_num_to_mask(irq_num);
        outb(master_mask, MASTER_8259_PORT2);
    }
}

/* 
 * send_eoi(uint32_t irq_num)
 * DESCRIPTION: sends EOI to PIC
 * INPUT: irq_num: irq number
 * OUTPUT: none
 * RETURNS: none
 * SIDE EFFECTS: sends eoi signal to pic with correct interrupt line
 *               when irq > 8, also sends eoi to master pic for line 2
 */
void send_eoi(uint32_t irq_num) {
    //send eoi to master pic if irq is on master pic
    if (irq_num >= 0 && irq_num < SLAVE_OFFSET){
        outb(EOI | (irq_num & 0x7), MASTER_8259_PORT); //0x7 is a three bit mask
    }//send eoi to both master and slave pic
    else if (irq_num >= SLAVE_OFFSET && irq_num < SLAVE_OFFSET * 2){
        outb((EOI | ((irq_num-SLAVE_OFFSET) & 0x7)), SLAVE_8259_PORT); //0x7 is a three bit mask
        outb((EOI | IRQ_SLAVE),MASTER_8259_PORT ); 
    }
    
}

/* 
 * irq_num_to_mask(uint32_t irq_num)
 * DESCRIPTION: converts irq_num to corosponding mask
 * INPUT: irq_num: irq number
 * OUTPUT: none
 * RETURNS: mask for corosponding irq_number
 * SIDE EFFECTS: none
 */
uint8_t irq_num_to_mask(uint32_t irq_num) {
    if((irq_num < 0) || (irq_num >= SLAVE_OFFSET)){
        return 0;
    }
    uint8_t mask = 1; // mask value for ir1_num = 0
    mask = mask << irq_num;
    return mask;
}
