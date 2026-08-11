#include <lpc214x.h>
#include "uart0.h"
#include "m_delay.h"
#include "lcd.h"
#include "esp01.h"
#include "dht11.h"
#include "kpm.h"

#define FOSC         12000000
#define CCLK         5 * FOSC
#define PCLK         CCLK / 4
#define PREINT_VAL   ((PCLK / 32768) - 1)
#define PREFRAC_VAL  (PCLK - ((PREINT_VAL + 1) * 32768))

/* -------------------------------------------------------------
   rtc_init()
   Sets the initial time (hour/min/sec) for the RTC registers.
   Note: this only initializes the SEC/MIN/HOUR values; the RTC
   prescaler (PREINT/PREFRAC) and clock-enable (CCR) must be
   configured elsewhere for the RTC to actually run.
   ------------------------------------------------------------- */
void rtc_init()
{
	SEC = 20;    // Initial seconds
	MIN = 21;    // Initial minutes
	HOUR = 12;   // Initial hour
}

extern char buff[100], dummy;
extern unsigned char i, ch, r_flag;

char I_t1, I_t2, I_t3;               // Irrigation durations (minutes) for 3 threshold levels
char T_t1, H_t1, T_t2, H_t2, T_t3, H_t3;   // Temperature/Humidity thresholds for the 3 levels

extern volatile int flag;            // Set by external interrupt to open the menu

unsigned char min, sec, humidity_integer, humidity_decimal, temp_integer, temp_decimal, checksum;

extern int humidity, temperature;

int flag1 = 0, flag2 = 0;   // flag1: irrigation timing configured, flag2: temp/hum thresholds configured
                             // (irrigation logic only runs once both are set)

/* -------------------------------------------------------------
   SetIrrigationTiming()
   Menu screen to configure how long the motor should run
   (in minutes) for each of 3 condition levels:
     I_t1 - High Temp / Low Humidity
     I_t2 - Moderate Temp / Moderate Humidity
     I_t3 - Low Temp / High Humidity
   ------------------------------------------------------------- */
void SetIrrigationTiming(void)
{
	int val;
	flag1 = 1;

	Write_CMD_LCD(0x01);
	Write_str_LCD("Set Motor Time");
	delay_ms(800);

	val = Get1DigitValue("Enter HighTem LowHum:");
	if(val != -1) I_t1 = val;
	delay_ms(800);

	Write_CMD_LCD(0x01);
	val = Get1DigitValue("Enter ModTem ModHum:");
	if(val != -1) I_t2 = val;
	delay_ms(800);

	Write_CMD_LCD(0x01);
	val = Get1DigitValue("Enter LowTemp HighHum:");
	if(val != -1) I_t3 = val;
	delay_ms(800);

	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("Saved Successfully");
	delay_ms(1000);

	Write_CMD_LCD(0x01);
}

/* -------------------------------------------------------------
   Set_Temp_Humidity()
   Menu screen to configure the temperature/humidity threshold
   pairs used by compare_temp_hum() to decide which irrigation
   duration (I_t1/I_t2/I_t3) applies.
   ------------------------------------------------------------- */
void Set_Temp_Humidity(void)
{
	int val;
	flag2 = 1;

	Write_CMD_LCD(0x01);
	Write_str_LCD("Set Temp & Hum");
	delay_ms(800);
	delay_ms(800);

	// Level 1: High temperature / Low humidity
	val = Get2DigitValue("1.LOW TEMPRATURE:");
	Write_str_LCD("Temp Range (0-50)");
	if(val != -1) T_t1 = val;

	delay_ms(800);
	val = Get2DigitValue("1.HIGH HUMIDITY:");
	Write_str_LCD("Humrange is (0-99)");
	if(val != -1) H_t1 = val;

	// Level 2: Moderate temperature / Moderate humidity
	Write_str_LCD("Temp Range (0-50)");
	delay_ms(800);
	val = Get2DigitValue("2.MOD TEMPRATURE:");
	Write_str_LCD("Temp Range (0-50)");
	if(val != -1) T_t2 = val;

	val = Get2DigitValue("2.MOD HUMIDITY:");
	Write_str_LCD("Humrange is (0-100)");
	if(val != -1) H_t2 = val;

	// Level 3: Low temperature / High humidity
	delay_ms(800);
	val = Get2DigitValue("3.HIGH TEMPRATURE:");
	Write_str_LCD("Temp Range (0-50)");
	if(val != -1) T_t3 = val;

	delay_ms(800);
	val = Get2DigitValue("3.LOW HUMIDITY:");
	Write_str_LCD("Humrange is (0-100)");
	if(val != -1) H_t3 = val;

	Write_CMD_LCD(0x01);
	Write_str_LCD("Saved");
	delay_ms(1000);

	Write_CMD_LCD(0x01);
}

/* -------------------------------------------------------------
   menu_base_int()
   Shows the main settings menu when the external interrupt
   flag is set (e.g. a menu button was pressed). Lets the user
   choose to configure irrigation timing, temp/humidity
   thresholds, or exit the menu.
   ------------------------------------------------------------- */
void menu_base_int(void)
{
	char ch;

	if(flag == 1)
	{
		flag = 0;   // Clear the flag now that we're handling it

		Write_CMD_LCD(0x01);
		Write_CMD_LCD(0x80);
		Write_str_LCD("1.Irrigation Time");
		Write_CMD_LCD(0xC0);
		Write_str_LCD("2.Temp & Humidity");
		Write_CMD_LCD(0x94);
		Write_str_LCD("3.exit");
		Write_CMD_LCD(0xD4);
		Write_str_LCD("Select Option:");

		ch = keyScan();

		switch(ch)
		{
			case '1':
				SetIrrigationTiming();
				break;

			case '2':
				Set_Temp_Humidity();
				break;

			case '3':
				Write_CMD_LCD(0x01);
				return;

			default:
				Write_CMD_LCD(0x01);
				Write_str_LCD("Invalid Option");
				delay_ms(1000);
		}
	}
}

/* -------------------------------------------------------------
   compare_temp_hum()
   Core irrigation control logic. Runs only after both the
   irrigation timing (flag1) and temp/humidity thresholds
   (flag2) have been configured via the menu.

   Checks soil moisture first (only proceeds if soil is dry).
   Then compares current temperature/humidity against the 3
   configured threshold levels to decide how long to run the
   motor. While running, it shows a live countdown on the LCD
   and stops early if soil becomes wet. Motor ON/OFF events are
   also reported to ThingSpeak.
   ------------------------------------------------------------- */
void compare_temp_hum(void)
{
	int delay_time;

	if((flag1 == 1) && (flag2 == 1))
	{
		delay_time = 0;

		// Check soil moisture first: bit is 1 = dry, 0 = wet
		if((IOPIN0 >> 21) & 1)
		{
			int cnt = 0;

			// Determine irrigation duration based on which threshold level matches
			if((temperature >= T_t1) && (humidity <= H_t1))
				delay_time = I_t1;
			else if((temperature >= T_t2) && (humidity >= H_t2))
				delay_time = I_t2;
			else if((temperature <= T_t3) && (humidity <= H_t3))
				delay_time = I_t3;

			// If a matching condition was found, run the motor
			if(delay_time > 0)
			{
				Write_CMD_LCD(0x01);
				Write_str_LCD("MOTOR STATUS");

				IOSET0 = 1 << 20;   // Motor ON

				Write_CMD_LCD(0xC0);
				Write_str_LCD("SENDING TO CLOUD");
				esp01_sendToThingspeak3(1);   // Report motor ON status to ThingSpeak

				// Set up countdown timer (mm:ss) for how long the motor should run
				sec = 59;
				min = delay_time;
				if(min == 1)
					min = 0;

				Write_CMD_LCD(0x01);

				while(1)
				{
					Write_CMD_LCD(0x80);
					Write_str_LCD("MOTOR TURN ON");
					Write_CMD_LCD(0xC0);

					cnt++;
					delay_s(1);

					// Stop if full duration elapsed, or soil has become wet
					if((cnt == (delay_time * 60)) || ((IOPIN0 >> 21) & 1) == 0)
						break;

					if(((IOPIN0 >> 21) & 1) == 0)
						break;

					// Countdown display update
					if(sec == 0)
					{
						min--;
						sec = 59;
					}

					Write_DAT_LCD((min / 10) + '0');
					Write_DAT_LCD((min % 10) + '0');
					Write_DAT_LCD(':');
					Write_DAT_LCD((sec / 10) + '0');
					Write_DAT_LCD((sec % 10) + '0');
					sec--;

					delay_ms(1);
				}

				IOCLR0 = 1 << 20;   // Motor OFF

				Write_CMD_LCD(0x01);
				Write_str_LCD("MOTOR TURN OFF");

				esp01_sendToThingspeak3(0);   // Report motor OFF status to ThingSpeak

				Write_CMD_LCD(0x01);
			}
		}
	}
}
