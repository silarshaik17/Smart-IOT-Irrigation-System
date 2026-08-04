#include <lpc21xx.h>
#include "typedef.h"
#include "m_delay.h"

#define EINT0_INPUT_PIN 0x000000C0   // Selects P0.1 as EINT0 function in PINSEL0
#define EINT0_VIC_CHNO  15           // VIC channel number for EINT0

volatile int flag = 0;               // Set by the ISR to signal that an external interrupt occurred

void eint0_isr(void) __irq;          // Forward declaration of the ISR

/* -------------------------------------------------------------
   init_int()
   Configures external interrupt EINT0 on the LPC21xx:
   - Sets the pin function to EINT0
   - Enables the EINT0 channel in the Vector Interrupt Controller (VIC)
   - Assigns the ISR address to the VIC slot
   - Sets EINT0 to edge-triggered mode
   ------------------------------------------------------------- */
void init_int(void)
{
	PINSEL0 |= EINT0_INPUT_PIN;                    // Configure pin as EINT0 input

	VICIntEnable = 1 << EINT0_VIC_CHNO;            // Enable EINT0 interrupt in VIC

	VICVectCntl1 = (1 << 5) | EINT0_VIC_CHNO;      // Assign EINT0 to VIC slot 1, mark as enabled

	VICVectAddr1 = (u32)eint0_isr;                 // Store ISR address in that VIC slot

	EXTMODE |= 1 << 1;                             // Set EINT0 to edge-triggered (not level-triggered)
}

/* -------------------------------------------------------------
   eint0_isr()
   Interrupt Service Routine for EINT0. Triggered when the
   external interrupt pin is activated (e.g. button press).
   Applies a small debounce delay, sets a flag for the main
   program to check, then clears the interrupt.
   ------------------------------------------------------------- */
void eint0_isr(void) __irq
{
	delay_ms(20);        // Basic debounce delay

	flag = 1;             // Signal to main program that interrupt occurred

	EXTINT = 1 << 1;      // Clear the EINT0 interrupt flag (write 1 to clear)

	VICVectAddr = 0;      // Acknowledge interrupt to the VIC (allows next interrupt)
}
