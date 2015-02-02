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

unsigned char bcdToDec(unsigned char val)
{
	return ((val/16*10) + (val%16) );
}

unsigned char decToBcd(unsigned char val)
{
	return ((val/10*16) + (val%10) );
}

char buffer[17];
unsigned char time[7];
unsigned char data[7];
unsigned char timeIn[8] {0x00,	 decToBcd(0), decToBcd(0), decToBcd(0), decToBcd(0),	 decToBcd(0), decToBcd(0), decToBcd(0),};
//	 Address,Seconds,	  Minutes,	   Hours,	    Day of the week, Date,		  month,	   Year.

int main(void)
{
	DDRD = 0x00;
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
/*		TWI.stop();
		TWI.start_wait(I2C_WRITE);
		TWI.writeXBytes(timeIn,8);
		TWI.stop();
*/
		while(1)
		{
			TWI.start_wait(I2C_WRITE);			// set device address and write mode	
			TWI.readXBytes(data,7,0x00);
			TWI.stop();
			for (uint8_t i = 0; i <= 7 ; i++)
			{
				time[i] = bcdToDec(data[i]);
			}
			lcd_home();
			snprintf(buffer, sizeof buffer,"%d:",time[2]);
			lcd_puts(buffer);
			lcd_goto(0,3);
			snprintf(buffer, sizeof buffer,"%d:",time[1]);
			lcd_puts(buffer);
			lcd_goto(0,6);
			snprintf(buffer, sizeof buffer,"%d ",time[0]);
			lcd_puts(buffer);
			lcd_goto(1,0);
			snprintf(buffer, sizeof buffer,"%d-",time[4]);
			lcd_puts(buffer);
			lcd_goto(1,3);
			snprintf(buffer, sizeof buffer,"%d-",time[5]);
			lcd_puts(buffer);
			lcd_goto(1,6);
			snprintf(buffer, sizeof buffer,"%d ",time[6]);
			lcd_puts(buffer);
			_delay_ms(1000);
			TWI.read(0x00);
		}
	}
}