#include <string.h>
#include "uart0.h"
#include "m_delay.h"
#include "lcd.h"

/* Shared buffer used to store the ESP8266's UART response,
   and 'i' tracks how many characters have been received. */
extern char buff[200];
extern unsigned char i;

/* -------------------------------------------------------------
   esp01_connectAP()
   Initializes the ESP01 (ESP8266) Wi-Fi module and connects it
   to a Wi-Fi Access Point using AT commands. Progress and
   results are shown on the LCD at each step.
   ------------------------------------------------------------- */
void esp01_connectAP()
{
	/* Step 1: Check if the module is responding using "AT" */
	Write_CMD_LCD(0x01);          // Clear LCD
	Write_CMD_LCD(0x80);          // Move cursor to first line
	Write_str_LCD("AT");          // Show current command on LCD
	delay_ms(1000);

	UART0_Str("AT\r\n");          // Send AT command to ESP01

	i = 0;
	memset(buff, '\0', 200);      // Clear receive buffer

	while(i < 4);                 // Wait until at least 4 bytes received (via UART ISR)
	delay_ms(500);
	buff[i] = '\0';                // Null-terminate the received string

	// Display raw response on LCD
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD(buff);
	delay_ms(2000);

	// Check if module replied "OK"
	if(strstr(buff, "OK"))
	{
		Write_CMD_LCD(0xC0);        // Move to second line
		Write_str_LCD("OK");
		delay_ms(1000);
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR");
		delay_ms(1000);
		return;                     // Abort if module not responding
	}

	/* Step 2: Disable command echo using "ATE0" */
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("ATE0");
	delay_ms(1000);

	UART0_Str("ATE0\r\n");
	i = 0;
	memset(buff, '\0', 200);

	while(i < 4);
	delay_ms(500);
	buff[i] = '\0';

	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD(buff);
	delay_ms(2000);

	if(strstr(buff, "OK"))
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("OK");
		delay_ms(1000);
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR");
		delay_ms(1000);
		return;
	}

	/* Step 3: Set single-connection mode using "AT+CIPMUX=0" */
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("AT+CIPMUX");
	delay_ms(1000);

	UART0_Str("AT+CIPMUX=0\r\n");
	i = 0;
	memset(buff, '\0', 200);

	while(i < 4);
	delay_ms(500);
	buff[i] = '\0';

	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD(buff);
	delay_ms(2000);

	if(strstr(buff, "OK"))
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("OK");
		delay_ms(1000);
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR");
		delay_ms(1000);
		return;
	}

	/* Step 4: Disconnect from any previously connected AP using "AT+CWQAP" */
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("AT+CWQAP");
	delay_ms(1000);

	UART0_Str("AT+CWQAP\r\n");
	i = 0;
	memset(buff, '\0', 200);

	while(i < 4);
	delay_ms(1500);
	buff[i] = '\0';

	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD(buff);
	delay_ms(2000);

	if(strstr(buff, "OK"))
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("OK");
		delay_ms(1000);
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR");
		delay_ms(1000);
		return;
	}

	/* Step 5: Connect to the Wi-Fi Access Point using "AT+CWJAP" */
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("AT+CWJAP");
	delay_ms(1000);

	// NOTE: Change the Wi-Fi SSID and password here as needed
	UART0_Str("AT+CWJAP=\"Motto\",\"123456789\"\r\n");

	i = 0;
	memset(buff, '\0', 200);

	while(i < 4);
	delay_ms(2500);               // Longer delay: AP connection takes more time
	buff[i] = '\0';

	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD(buff);
	delay_ms(2000);

	// Successful AP connection reports "WIFI CONNECTED"
	if(strstr(buff, "WIFI CONNECTED"))
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("OK");
		delay_ms(1000);
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR");
		delay_ms(1000);
		return;
	}
}

/* -------------------------------------------------------------
   esp01_sendToThingspeak1()
   Opens a TCP connection to ThingSpeak and uploads two sensor
   values (val1 = humidity, val2 = temperature) to fields 1 and 2
   of a ThingSpeak channel using an HTTP GET request.
   ------------------------------------------------------------- */
void esp01_sendToThingspeak1(float val1, float val2)
{
	delay_ms(1000);

	// Open TCP connection to ThingSpeak server on port 80
	UART0_Str("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");

	i = 0;
	memset(buff, '\0', 200);

	while(i < 5);
	delay_ms(2500);
	buff[i] = '\0';

	delay_ms(2000);

	// Proceed only if connection succeeded
	if(strstr(buff, "CONNECT") || strstr(buff, "ALREADY CONNECTED"))
	{
		delay_ms(1000);

		// Tell the module how many bytes will be sent next
		delay_ms(1000);
		UART0_Str("AT+CIPSEND=64\r\n");

		i = 0;
		memset(buff, '\0', 200);
		delay_ms(500);

		// NOTE: Change the ThingSpeak Write API key according to your channel
		UART0_Str("GET /update?api_key=IV36NOCQD1WKYP0Q&field1=");
		UART0_Float(val2);            // Send temperature to field1
		UART0_Str("&field2=");
		UART0_Float(val1);            // Send humidity to field2
		UART0_Str("\r\n\r\n");

		delay_ms(5000);
		delay_ms(5000);               // Wait for ThingSpeak server response

		buff[i] = '\0';

		delay_ms(2000);

		// Check if data was successfully sent
		if(strstr(buff, "SEND OK"))
		{
			delay_ms(1000);
		}
		else
		{
			Write_CMD_LCD(0x01);
			Write_str_LCD("TEMPRATURE IS NOT UPDATED");
			delay_ms(1000);

			Write_CMD_LCD(0x01);
		}
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR1");
		delay_ms(1000);

		Write_CMD_LCD(0x01);
		return;
	}
}

/* -------------------------------------------------------------
   esp01_sendToThingspeak2()
   Opens a TCP connection to ThingSpeak and uploads a single
   sensor value (val) to field1 of a ThingSpeak channel.
   ------------------------------------------------------------- */
void esp01_sendToThingspeak2(float val)
{
	delay_ms(1000);

	// Open TCP connection to ThingSpeak server on port 80
	UART0_Str("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");

	i = 0;
	memset(buff, '\0', 200);

	while(i < 5);
	delay_ms(2500);
	buff[i] = '\0';

	delay_ms(2000);

	if(strstr(buff, "CONNECT") || strstr(buff, "ALREADY CONNECTED"))
	{
		delay_ms(1000);

		delay_ms(1000);
		UART0_Str("AT+CIPSEND=51\r\n");

		i = 0;
		memset(buff, '\0', 200);
		delay_ms(500);

		// NOTE: Change the ThingSpeak Write API key according to your channel
		UART0_Str("GET /update?api_key=IV36NOCQD1WKYP0Q&field1=");
		UART0_Float(val);             // Send value to field1

		UART0_Str("\r\n\r\n");

		delay_ms(5000);
		delay_ms(5000);

		buff[i] = '\0';

		delay_ms(2000);

		if(strstr(buff, "SEND OK"))
		{
			delay_ms(1000);
		}
		else
		{
			Write_CMD_LCD(0x01);
			Write_str_LCD("HUMIDITY IS NOT UPADTED");
			delay_ms(1000);

			Write_CMD_LCD(0x01);
		}
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR2");
		delay_ms(1000);

		Write_CMD_LCD(0x01);
		return;
	}
}

/* -------------------------------------------------------------
   esp01_sendToThingspeak3()
   Opens a TCP connection to ThingSpeak and uploads a single
   sensor value (val) to field3 of a ThingSpeak channel
   (e.g. used for motor-related data).
   ------------------------------------------------------------- */
void esp01_sendToThingspeak3(float val)
{
	delay_ms(1000);

	// Open TCP connection to ThingSpeak server on port 80
	UART0_Str("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");

	i = 0;
	memset(buff, '\0', 200);

	while(i < 5);
	delay_ms(2500);
	buff[i] = '\0';

	delay_ms(2000);

	if(strstr(buff, "CONNECT") || strstr(buff, "ALREADY CONNECTED"))
	{
		delay_ms(1000);

		delay_ms(1000);
		UART0_Str("AT+CIPSEND=51\r\n");

		i = 0;
		memset(buff, '\0', 200);
		delay_ms(500);

		// Change the ThingSpeak Write API key according to your channel
		UART0_Str("GET /update?api_key=IV36NOCQD1WKYP0Q&field3=");

		UART0_Float(val);             // Send value to field3

		UART0_Str("\r\n\r\n");

		delay_ms(5000);
		delay_ms(5000);

		buff[i] = '\0';

		delay_ms(2000);

		if(strstr(buff, "SEND OK"))
		{
			delay_ms(1000);
		}
		else
		{
			Write_CMD_LCD(0x01);
			Write_str_LCD("MOTOR DATA NOT UPDATED");
			delay_ms(1000);

			Write_CMD_LCD(0x01);
		}
	}
	else
	{
		Write_CMD_LCD(0xC0);
		Write_str_LCD("ERROR2");
		delay_ms(1000);

		Write_CMD_LCD(0x01);
		return;
	}
}
