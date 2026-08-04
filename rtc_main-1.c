#include <lpc214x.h>

// LCD header files
#include "lcd.h"
#include "lcd_defines.h"

// System clock and peripheral clock macros
#define FOSC 12000000
#define CCLK (5 * FOSC)
#define PCLK (CCLK / 4)

// RTC Prescaler Calculation Macros
// RTC requires a 32.768 kHz clock for 1-second increments.
// PREINT and PREFRAC registers divide PCLK to generate 32.768 kHz.
//
// PREINT  = int(PCLK / 32768) - 1
// PREFRAC = PCLK - ((PREINT + 1) * 32768)
// Note: This information was collected from the LPC2129 manual.
#define PREINT_VAL  (int)((PCLK / 32768) - 1)
#define PREFRAC_VAL (PCLK - ((PREINT_VAL + 1) * 32768))

// RTC Control Register (CCR) Bit Definitions
// Bit 0 - Clock Enable: 1 = Enable RTC counters, 0 = Disable RTC counters
#define RTC_ENABLE (1 << 0)

// Bit 1 - Clock Reset: 1 = Reset RTC counters, 0 = Normal operation
#define RTC_RESET (1 << 1)

// Only for LPC2148
// Bit 4 - Clock Source Select
// 1 = Use external 32.768 kHz oscillator
// 0 = Use internal PCLK as RTC clock source
#define RTC_CLKSRC (1 << 4)

#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

void RTC_Init(void);
void GetRTCTimeInfo(s32 *, s32 *, s32 *);
void DisplayRTCTime(u32, u32, u32);
void GetRTCDateInfo(s32 *, s32 *, s32 *);
void DisplayRTCDate(u32, u32, u32);

void SetRTCTimeInfo(u32, u32, u32);
void SetRTCDateInfo(u32, u32, u32);

void GetRTCDay(s32 *);
void DisplayRTCDay(u32);
void SetRTCDay(u32);

s32 hour, min, sec, date, month, year, day;

// Names of the days of the week, indexed 0 (Sun) to 6 (Sat)
char week[][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

#define _LPC2148

/* -------------------------------------------------------------
   main()
   Initializes the RTC and LCD, sets a starting time/date/day,
   then continuously reads the RTC and displays the current
   time, date, and day of the week on the LCD.
   ------------------------------------------------------------- */
int main()
{
	RTC_Init();     // Initialize the RTC
	InitLCD();       // Initialize the LCD

	SetRTCTimeInfo(14, 50, 00);     // Set initial time: 14:50:00
	SetRTCDateInfo(2, 3, 2026);     // Set initial date: 02/03/2026
	SetRTCDay(MON);                 // Set initial day: Monday

	while(1)
	{
		// Read and display the current time
		GetRTCTimeInfo(&hour, &min, &sec);
		DisplayRTCTime(hour, min, sec);

		// Read and display the current date
		GetRTCDateInfo(&date, &month, &year);
		DisplayRTCDate(date, month, year);

		// Read and display the current day of the week
		GetRTCDay(&day);
		DisplayRTCDay(day);
	}
}

/* -------------------------------------------------------------
   RTC_Init()
   Initializes the Real-Time Clock: disables/resets the RTC,
   configures the prescaler so it ticks accurately at 1 second
   per increment, then enables it. On LPC2148, an external
   32.768 kHz crystal is used as the clock source instead of
   the calculated prescaler.
   ------------------------------------------------------------- */
void RTC_Init(void)
{
	CCR = RTC_RESET;   // Disable and reset the RTC

#ifndef _LPC2148
	// Set prescaler integer and fractional parts (for chips without
	// a dedicated external RTC oscillator input)
	PREINT = PREINT_VAL;
	PREFRAC = PREFRAC_VAL;

	CCR = RTC_ENABLE;   // Enable the RTC
#else
	CCR = RTC_ENABLE | RTC_CLKSRC;   // Enable the RTC using the external 32.768 kHz oscillator
#endif
}

/* -------------------------------------------------------------
   GetRTCTimeInfo()
   Reads the current hour, minute, and second from the RTC
   hardware registers into the given pointers.
   ------------------------------------------------------------- */
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
	*hour = HOUR;
	*minute = MIN;
	*second = SEC;
}

/* -------------------------------------------------------------
   DisplayRTCTime()
   Displays the time on the LCD's first line in HH:MM:SS format.
   ------------------------------------------------------------- */
void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
	CmdLCD(GOTO_LINE1_POS0);
	CharLCD(hour / 10 + 48);
	CharLCD(hour % 10 + 48);
	CharLCD(':');
	CharLCD(minute / 10 + 48);
	CharLCD(minute % 10 + 48);
	CharLCD(':');
	CharLCD(second / 10 + 48);
	CharLCD(second % 10 + 48);
}

/* -------------------------------------------------------------
   GetRTCDateInfo()
   Reads the current day of month, month, and year from the
   RTC hardware registers into the given pointers.
   ------------------------------------------------------------- */
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
	*date = DOM;
	*month = MONTH;
	*year = YEAR;
}

/* -------------------------------------------------------------
   DisplayRTCDate()
   Displays the date on the LCD's second line in DD/MM/YYYY
   format.
   ------------------------------------------------------------- */
void DisplayRTCDate(u32 date, u32 month, u32 year)
{
	CmdLCD(GOTO_LINE2_POS0);
	CharLCD(date / 10 + 48);
	CharLCD(date % 10 + 48);
	CharLCD('/');
	CharLCD(month / 10 + 48);
	CharLCD(month % 10 + 48);
	CharLCD('/');
	U32LCD(year);
}

/* -------------------------------------------------------------
   SetRTCTimeInfo()
   Sets the RTC's hour, minute, and second registers.
   ------------------------------------------------------------- */
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
	HOUR = hour;
	MIN = minute;
	SEC = second;
}

/* -------------------------------------------------------------
   SetRTCDateInfo()
   Sets the RTC's day-of-month, month, and year registers.
   ------------------------------------------------------------- */
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
	DOM = date;
	MONTH = month;
	YEAR = year;
}

/* -------------------------------------------------------------
   GetRTCDay()
   Reads the current day of the week from the RTC
   (0 = Sunday, ..., 6 = Saturday).
   ------------------------------------------------------------- */
void GetRTCDay(s32 *dow)
{
	*dow = DOW;
}

/* -------------------------------------------------------------
   DisplayRTCDay()
   Displays the day of the week (as text, e.g. "MON") on the
   LCD, appended after the time on the first line.
   ------------------------------------------------------------- */
void DisplayRTCDay(u32 dow)
{
	CmdLCD(GOTO_LINE1_POS0 + 10);
	StrLCD(week[dow]);
}

/* -------------------------------------------------------------
   SetRTCDay()
   Sets the RTC's day-of-week register
   (0 = Sunday, ..., 6 = Saturday).
   ------------------------------------------------------------- */
void SetRTCDay(u32 dow)
{
	DOW = dow;
}
