#include <lpc21xx.h>
#include "lcd.h"
#include "kpm.h"
#include "uart0.h"
#include "esp01.h"
#include "m_delay.h"
#include "m_int0.h"
#include "dht11.h"
#include "kpm_defines.h"
#include "menu_base.h"

#define SEND_INTERVAL 3   // Send sensor data to ThingSpeak every 3 minutes

extern int flag;          // Set by the external interrupt ISR (e.g. keypad/menu button)

int humidity, temperature;
int last_sent_min = -1;   // Tracks the last minute data was uploaded, to avoid duplicate sends

int main()
{
	unsigned char humidity_integer, humidity_decimal, temp_integer, temp_decimal, checksum;

	IODIR0 = 1 << 20;         // P0.20 as output, used to control the motor/pump
	IODIR0 &= ~(1 << 21);     // P0.21 as input, reads soil moisture sensor (0 = wet, 1 = dry)

	InitUART0();
	LCD_Init();

	Write_str_LCD("SMART IoT IRRIGATION");   // Startup splash message
	delay_ms(1000);
	Write_CMD_LCD(0x01);                      // Clear LCD

	Initkpm();          // Initialize keypad
	init_int();          // Initialize external interrupt (EINT0)
	rtc_init();           // Initialize real-time clock
	esp01_connectAP();    // Connect ESP01 Wi-Fi module to the access point

	delay_ms(1000);
	Write_CMD_LCD(0x01);

	while(1)
	{
		// If interrupt flag is set (e.g. menu button pressed), jump straight to menu
		if(flag == 1)
			goto a;

		delay_s(3);   // Wait between sensor readings

		// Read humidity & temperature from the DHT11 sensor
		dht11_request();
		dht11_response();

		humidity_integer = dht11_data();
		humidity_decimal = dht11_data();
		temp_integer     = dht11_data();
		temp_decimal     = dht11_data();
		checksum         = dht11_data();

		humidity    = humidity_integer + humidity_decimal;
		temperature = temp_integer + temp_decimal;

		// Validate the reading using DHT11's checksum byte
		if((humidity_integer + humidity_decimal + temp_integer + temp_decimal) != checksum)
		{
			Write_str_LCD("Checksum Error\r\n");
			delay_ms(500);
			Write_CMD_LCD(0x01);
		}
		else
		{
			// Display humidity and temperature readings on the LCD
			Write_CMD_LCD(0x80);
			Write_str_LCD("Humidity : ");
			Write_int_LCD(humidity_integer);
			Write_DAT_LCD('.');
			Write_int_LCD(humidity_decimal);
			Write_str_LCD(" % RH ");

			Write_CMD_LCD(0xC0);
			Write_str_LCD("Temperature : ");
			Write_int_LCD(temp_integer);
			Write_DAT_LCD('.');
			Write_int_LCD(temp_decimal);
			Write_DAT_LCD(223);   // Degree symbol
			Write_str_LCD("C");

			Write_CMD_LCD(0xD4);
			Write_str_LCD("Checksum : ");
			Write_int_LCD(checksum);

			delay_ms(3000);
		}

		// Upload to ThingSpeak once per SEND_INTERVAL minutes (avoids repeat sends within same minute)
		while((MIN % SEND_INTERVAL == 0) && (MIN != last_sent_min))
		{
			esp01_sendToThingspeak1(humidity, temperature);
			last_sent_min = MIN;
		}

	a:
		menu_base_int();     // Handle menu / keypad interaction
		compare_temp_hum();   // Check readings against thresholds (e.g. to control irrigation)
	}
}
