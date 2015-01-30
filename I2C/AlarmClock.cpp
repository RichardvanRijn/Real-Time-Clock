#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <util/delay.h>
#include <inttypes.h>
#include "lcd.h"
#include "I2C.h"

#define address 0xD0

char buffer[30];
unsigned char time[7];
unsigned char temp;

//unsigned char address=0xD0;

int main(void)
{
	I2C TWI(address);
	lcd_init();
	lcd_cursor(false, false);							//  cursor off
	lcd_home();
	TWI.init();											// Function to initialize TWI
	snprintf(buffer, sizeof buffer, "int");
	lcd_puts(buffer);
	lcd_home();
	int ret = TWI.start(I2C_WRITE);				// set device address and write mode
	if ( ret ) {										// failed to issue start condition, possibly no device found
		TWI.stop();
		snprintf(buffer, sizeof buffer, "Failed");
		lcd_puts(buffer);
		lcd_home();
		}
	else 
	{
		TWI.stop();
		TWI.start_wait(I2C_WRITE);
		TWI.write(0x00);						//location
		TWI.write(0x80);						//Seconds	Bit 7 stops the clock
		TWI.write(0x00);						//Minutes
		TWI.write(0x03);						//Hours bit 6 zero = 24H
		TWI.write(0x00);						//Day of the week
		TWI.write(0x00);						//Date
		TWI.write(0x00);						//month
		TWI.write(0x00);						//Year	00-99
		TWI.stop();
		while(1)
		{
			TWI.start_wait(I2C_WRITE);			// set device address and write mode
			TWI.write(0x00);							// write address = 0
			TWI.rep_start(I2C_READ);			// set device address and read mode
			time[0] = TWI.readAck();                    // read one byte
			time[1] = TWI.readAck();
			time[2] = TWI.readAck();
			time[3] = TWI.readAck();
			time[4] = TWI.readAck();
			time[5] = TWI.readAck();
			time[6] = TWI.readNak();			TWI.stop();
			_delay_ms(800);
		}
	}
}

