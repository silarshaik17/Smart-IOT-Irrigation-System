#include "typedef.h"

/* -------------------------------------------------------------
   delay_us()
   Busy-wait (software) delay of approximately 'dlys' microseconds.
   Implemented as an empty loop; the multiplier (12) is tuned to
   the CPU clock speed to approximate 1us per count.
   ------------------------------------------------------------- */
void delay_us(u32 dlys)
{
	for(dlys *= 12; dlys > 0; dlys--);
}

/* -------------------------------------------------------------
   delay_ms()
   Busy-wait delay of approximately 'dlys' milliseconds.
   Same technique as delay_us(), scaled up by 1000x.
   ------------------------------------------------------------- */
void delay_ms(u32 dlys)
{
	for(dlys *= 12000; dlys > 0; dlys--);
}

/* -------------------------------------------------------------
   delay_s()
   Busy-wait delay of approximately 'dlys' seconds.
   Same technique, scaled up by 1,000,000x.
   ------------------------------------------------------------- */
void delay_s(u32 dlys)
{
	for(dlys *= 12000000; dlys > 0; dlys--);
}
