#include <LPC21xx.H>   /* LPC21xx definitions */
#include <string.h>

#define UART_INT_ENABLE 1

void InitUART0(void);   /* Initialize Serial Interface */
void UART0_Tx(char ch);
char UART0_Rx(void);

char buff[10], dummy;
unsigned char i = 0, ch, r_flag;

/* -------------------------------------------------------------
   UART0_isr()
   Interrupt Service Routine for UART0. Fires on both receive
   and transmit interrupts:
     - On receive: reads the incoming byte into buff[] (up to
       200 bytes), advancing index i. Reading U0RBR also clears
       the receive interrupt.
     - On transmit (or other cause): reads U0IIR just to clear
       the interrupt (byte discarded into 'dummy').
   ------------------------------------------------------------- */
void UART0_isr(void) __irq
{
	if((U0IIR & 0x04))   // Check if this is a receive interrupt
	{
		ch = U0RBR;   // Reading U0RBR clears the receive interrupt

		if(i < 200)
			buff[i++] = ch;
	}
	else
	{
		dummy = U0IIR;   // Read to clear transmit (or other) interrupt
	}

	VICVectAddr = 0;   // Acknowledge interrupt to the VIC (dummy write)
}

/* -------------------------------------------------------------
   InitUART0()
   Configures UART0 for 9600 baud, 8 data bits, no parity,
   1 stop bit. If UART_INT_ENABLE is set, also configures the
   VIC to route UART0 interrupts to UART0_isr() and enables
   RX and THRE (transmit) interrupts.
   ------------------------------------------------------------- */
void InitUART0(void)
{
	PINSEL0 = 0x00000005;   // Enable RxD0 and TxD0 pin functions

	U0LCR = 0x83;   // DLAB = 1, 8 bits, no parity, 1 stop bit (to access divisor latch)
	U0DLL = DIVISOR;     // Divisor for 9600 baud @ CCLK/4 peripheral clock //97
	U0LCR = 0x03;   // DLAB = 0, back to normal register access (8N1)

#if UART_INT_ENABLE > 0
	VICIntSelect = 0x00000000;         // Route all interrupts as IRQ (not FIQ)
	VICVectAddr0 = (unsigned)UART0_isr;   // Assign ISR address to VIC slot 0
	VICVectCntl0 = 0x20 | 6;            // Enable slot 0, assign UART0's interrupt source (6)
	VICIntEnable = 1 << 6;              // Enable UART0 interrupt in the VIC

	U0IER = 0x03;   // Enable UART0 RX and THRE (transmit) interrupts
#endif
}

/* -------------------------------------------------------------
   UART0_Tx()
   Sends a single character over UART0, blocking until the
   transmit holding register is empty (THRE bit set).
   ------------------------------------------------------------- */
void UART0_Tx(char ch)
{
	while(!(U0LSR & 0x20));   // Wait until THR is empty
	U0THR = ch;
}

/* -------------------------------------------------------------
   UART0_Rx()
   Reads a single character from UART0, blocking until a byte
   has been received (RDR bit set).
   ------------------------------------------------------------- */
char UART0_Rx(void)
{
	while(!(U0LSR & 0x01));   // Wait until a byte is available
	return (U0RBR);
}

/* -------------------------------------------------------------
   UART0_Str()
   Sends a null-terminated string over UART0, one character
   at a time.
   ------------------------------------------------------------- */
void UART0_Str(char *s)
{
	while(*s)
		UART0_Tx(*s++);
}

/* -------------------------------------------------------------
   UART0_Int()
   Sends an unsigned integer over UART0 as ASCII decimal digits
   (e.g. 123 -> '1','2','3'). Extracts digits least-significant
   first into a temp array, then sends them in reverse order.
   ------------------------------------------------------------- */
void UART0_Int(unsigned int n)
{
	unsigned char a[10] = {0,0,0,0,0,0,0,0,0,0};
	int i = 0;

	if(n == 0)
	{
		UART0_Tx('0');
		return;
	}
	else
	{
		while(n > 0)
		{
			a[i++] = (n % 10) + 48;   // Store each digit as ASCII, least-significant first
			n = n / 10;
		}

		--i;
		for(; i >= 0; i--)   // Send digits back in correct (most-significant-first) order
		{
			UART0_Tx(a[i]);
		}
	}
}

/* -------------------------------------------------------------
   UART0_Float()
   Sends a float over UART0 as "integer.fractional" with 2
   decimal places (e.g. 23.456 -> "23.45"). Simple truncation,
   not rounding.
   ------------------------------------------------------------- */
void UART0_Float(float f)
{
	int x;
	float temp;

	x = f;              // Integer part
	UART0_Int(x);
	UART0_Tx('.');

	temp = (f - x) * 100;   // First 2 decimal digits
	x = temp;
	UART0_Int(x);
}

/* -------------------------------------------------------------
   DelayS()
   Busy-wait (software) delay of approximately 'dly' seconds.
   ------------------------------------------------------------- */
void DelayS(unsigned int dly)
{
	unsigned int i;

	for(; dly > 0; dly--)
		for(i = 1200000; i > 0; i--);
}
