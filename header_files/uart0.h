#ifndef _UART0_H_
#define _UART0_H_

#define UART_INT_ENABLE 1

//BAUD RATE CALCULATION & DIVISOR
#define FOSC      12000000   //Hz
#define CCLK  	  5*FOSC
#define PCLK  	  CCLK/4
#define BAUD  	  9600
#define DIVISOR   (PCLK/(16 * BAUD))


void InitUART0(void); /* Initialize Serial Interface       */ 
void UART0_Tx(char ch);  
char UART0_Rx(void); 
void UART0_Str(char *);
void UART0_Int(unsigned int);
void UART0_Float(float);


#endif
