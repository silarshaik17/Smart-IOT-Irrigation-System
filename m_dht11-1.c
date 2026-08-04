#include <lpc214x.h>
#include "delay.h"
#include "defines.h"

#define input  0
#define output 1

#define DHT11 4   // DHT11 data pin connected to P0.4

/* -------------------------------------------------------------
   dht11_request()
   Sends the "start" signal to the DHT11 sensor:
   pull the data line LOW for ~20ms, then release it HIGH
   so the sensor can begin its response.
   ------------------------------------------------------------- */
void dht11_request(void)
{
	WRITEBIT(IODIR0, DHT11, output);   // Configure DHT11 pin as output
	WRITEBIT(IOPIN0, DHT11, 0);        // Pull pin LOW (start signal, min 18ms required)
	delay_ms(20);
	WRITEBIT(IOPIN0, DHT11, 1);        // Release pin HIGH and wait for sensor's response
}

/* -------------------------------------------------------------
   dht11_response()
   Waits for and validates the DHT11's acknowledgement sequence
   (LOW then HIGH pulse) after the request signal, confirming
   the sensor is ready to send data.
   ------------------------------------------------------------- */
void dht11_response(void)
{
	WRITEBIT(IODIR0, DHT11, input);          // Switch pin to input to read sensor's response

	while(READBIT(IOPIN0, DHT11) == 1);      // Wait until line goes LOW (sensor pulls it down)
	while(READBIT(IOPIN0, DHT11) == 0);      // Wait until line goes HIGH (sensor's ack pulse)
	while(READBIT(IOPIN0, DHT11) == 1);      // Wait until line goes LOW again (end of response, ready for data)
}

/* -------------------------------------------------------------
   dht11_data()
   Reads one byte (8 bits) of data from the DHT11 sensor.
   Each bit is encoded by the duration the line stays HIGH
   after an initial LOW pulse:
     - short HIGH  -> bit 0
     - long HIGH   -> bit 1
   Call this function 4 times to get: humidity integer,
   humidity decimal, temperature integer, temperature decimal
   (plus a 5th call for the checksum byte).
   ------------------------------------------------------------- */
unsigned char dht11_data(void)
{
	unsigned char count;
	unsigned char data = 0;

	for(count = 0; count < 8; count++)          // Read 8 bits, MSB first
	{
		while(READBIT(IOPIN0, DHT11) == 0);     // Wait for line to go HIGH (start of this bit)

		delay_us(30);                           // Sample after ~30us (longer than a '0' bit's HIGH time)

		if(READBIT(IOPIN0, DHT11))               // Still HIGH after 30us -> bit is 1
			data = ((data << 1) | 0x01);
		else                                      // Already LOW -> bit is 0
			data = (data << 1);

		while(READBIT(IOPIN0, DHT11));           // Wait for line to go LOW (end of this bit, only if bit was 1)
	}

	return data;
}
