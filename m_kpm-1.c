#include <lpc21xx.h>
#include "typedef.h"
#include "defines.h"
#include <string.h>
#include "m_delay.h"
#include "kpm_defines.h"
#include "lcd.h"

/* 4x4 matrix keypad lookup table: [row][col] -> character.
   Rows/cols are scanned in hardware; this table maps the
   detected row/column position to the actual key label. */
u8 kpmLUT[4][4] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

/* -------------------------------------------------------------
   Initkpm()
   Configures the keypad's row pins as outputs (4 bits, starting
   at ROW0) so they can be driven to scan columns.
   ------------------------------------------------------------- */
void Initkpm(void)
{
	WNIBBLE(IODIR1, ROW0, 15);   // Set all 4 row pins as output (0b1111)
}

/* -------------------------------------------------------------
   ColScan()
   Quick check of the column pins to see if ANY key is currently
   pressed. Returns 0 if a key is pressed (a column pin reads
   LOW), 1 if no key is pressed (all columns read HIGH).
   ------------------------------------------------------------- */
u32 ColScan(void)
{
	return ((RNIBBLE(IOPIN1, COL0) < 15) ? 0 : 1);
}

/* -------------------------------------------------------------
   RowCheck()
   Drives each row LOW one at a time and checks the columns
   after each, to find which row the pressed key is in.
   Returns the row index (0-3) where a key press was detected.
   ------------------------------------------------------------- */
u32 RowCheck(void)
{
	u32 rno;

	for(rno = 0; rno < 4; rno++)
	{
		WNIBBLE(IOPIN1, ROW0, ~(1 << rno));   // Drive only this row LOW, others HIGH
		if(ColScan() == 0)                     // If a key press is detected on this row...
		{
			break;                              // ...stop here, rno holds the row
		}
	}

	WNIBBLE(IOPIN1, ROW0, 0x0);   // Reset all rows back to default (all LOW/driven)
	return rno;
}

/* -------------------------------------------------------------
   ColCheck()
   Checks each column pin individually to find which column
   the pressed key is in. Returns the column index (0-3).
   ------------------------------------------------------------- */
u32 ColCheck(void)
{
	u32 cno;

	for(cno = 0; cno < 4; cno++)
	{
		if(RBIT(IOPIN1, (COL0 + cno)) == 0)   // This column reads LOW -> key found here
		{
			break;
		}
	}

	return cno;
}

/* -------------------------------------------------------------
   keyScan()
   Blocks until a key is pressed, determines its row/column,
   looks up the corresponding character, then waits for the
   key to be released before returning. This debounces the
   press/release transitions by design (wait for press, wait
   for release).
   ------------------------------------------------------------- */
u32 keyScan(void)
{
	u32 rno, cno, keyV;

	while(ColScan());          // Wait until a key is pressed

	rno = RowCheck();          // Determine which row was pressed
	cno = ColCheck();          // Determine which column was pressed

	keyV = kpmLUT[rno][cno];   // Translate row/column into the key's character

	while(!ColScan());         // Wait until the key is released

	return keyV;
}

/* -------------------------------------------------------------
   ReadNum()
   Reads digits from the keypad one at a time, building up a
   multi-digit number, until the '=' key is pressed. Each digit
   typed is also echoed to the LCD.
   ------------------------------------------------------------- */
u32 ReadNum(void)
{
	u8 key;
	u32 sum = 0;

	while(1)
	{
		key = keyScan();

		if(key >= '0' && key <= '9')
		{
			LCD_Init();
			Write_LCD(key);
			sum = (sum * 10) + (key - '0');   // Shift previous digits left, add new digit
		}
		else if(key == '=')
		{
			break;   // '=' key signals end of number entry
		}
	}

	return sum;
}

/* -------------------------------------------------------------
   Get2DigitValue()
   Prompts the user with a message on the LCD and reads exactly
   2 digits from the keypad to form a value (0-99). Supports:
     - 'C' to correct/backspace the last digit entered
     - '*' to skip/cancel (returns -1)
   Also validates the value based on keywords in the prompt
   message: "TEMP" limits to 0-50, "HUM" limits to 0-99.
   If invalid, shows an error and restarts entry.
   ------------------------------------------------------------- */
int Get2DigitValue(char *msg)
{
	char arr[3] = {0};
	char key;
	int i = 0;
	int val;

again:
	i = 0;
	arr[0] = arr[1] = 0;

	Write_CMD_LCD(0x01);      // Clear LCD

	// Row 1: show the prompt message
	Write_CMD_LCD(0x80);
	Write_str_LCD(msg);

	// Row 4: cursor position for user input
	Write_CMD_LCD(0xD4);

	while(i < 2)
	{
		key = keyScan();

		if(key >= '0' && key <= '9')
		{
			arr[i++] = key;
			Write_DAT_LCD(key);
			delay_ms(100);
		}
		else if(key == 'C' && i > 0)
		{
			// Backspace: erase the last entered digit on the LCD
			i--;
			Write_CMD_LCD(0x10);
			Write_DAT_LCD(' ');
			Write_CMD_LCD(0x10);
		}
		else if(key == '*')
		{
			// Skip/cancel entry
			Write_CMD_LCD(0x01);
			Write_str_LCD("Skipped");
			delay_ms(500);
			return -1;
		}
	}

	val = (arr[0] - '0') * 10 + (arr[1] - '0');   // Combine the two digits into one number

	// Temperature validation: only allow 0-50 when prompt mentions "TEMP"
	if(strstr(msg, "TEMP") != 0 && val > 50)
	{
		Write_CMD_LCD(0x01);
		Write_str_LCD("Invalid Temp!");
		Write_CMD_LCD(0xC0);
		Write_str_LCD("Enter 0-50");
		delay_ms(1000);
		goto again;
	}

	// Humidity validation: only allow 0-99 when prompt mentions "HUM"
	if(strstr(msg, "HUM") != 0 && val > 99)
	{
		Write_CMD_LCD(0x01);
		Write_str_LCD("Invalid Hum!");
		Write_CMD_LCD(0xC0);
		Write_str_LCD("Enter 0-99");
		delay_ms(1000);
		goto again;
	}

	return val;
}

/* -------------------------------------------------------------
   Get1DigitValue()
   Prompts the user with a message on the LCD and reads a
   single digit restricted to the range 0-3 (e.g. for a mode
   or option selection). Supports '*' to skip/cancel (returns -1).
   Re-prompts on invalid input.
   ------------------------------------------------------------- */
int Get1DigitValue(char *msg)
{
	char key;

again:
	Write_CMD_LCD(0x01);      // Clear LCD

	// Row 1: show the prompt message
	Write_CMD_LCD(0x80);
	Write_str_LCD(msg);

	// Row 4: cursor position for user input
	Write_CMD_LCD(0xD4);

	while(1)
	{
		key = keyScan();

		if(key >= '0' && key <= '9')
		{
			// Only digits 0-3 are valid choices
			if(key >= '0' && key <= '3')
			{
				Write_DAT_LCD(key);
				delay_ms(500);
				return (key - '0');
			}
			else
			{
				Write_CMD_LCD(0x01);
				Write_str_LCD("Invalid!");
				Write_CMD_LCD(0xC0);
				Write_str_LCD("Enter 0-3 only");
				delay_ms(1000);
				goto again;
			}
		}
		else if(key == '*')
		{
			// Skip/cancel entry
			Write_CMD_LCD(0x01);
			Write_str_LCD("Skipped");
			delay_ms(500);
			return -1;
		}
	}
}
