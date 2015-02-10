#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd.h"
#include "I2C.h"

#define address 0xD0

char buffer[17];
unsigned char time[13];
unsigned char data[7];
unsigned char temp;

I2C TWI(address);

void init_T0(void)
{
	TCCR0 = (1<<CS02) && (1<<CS00);
	TCNT0 = 0;
	TIMSK = (1<<TOIE0);
}

unsigned char bcdToDec(unsigned char val)
{
	return ((val/16*10) + (val%16) );
}

unsigned char decToBcd(unsigned char val)
{
	return ((val/10*16) + (val%10) );
}

unsigned char timeIn[8] = {0x00,	 decToBcd(30), decToBcd(41), decToBcd(17), decToBcd(5),	 decToBcd(6), decToBcd(2), decToBcd(15)};
//						   Address	,Seconds,	  Minutes,	   Hours,	    Day of the week, Date,		  month,	   Year.

void writeScreen(void)
{
	lcd_home();
	snprintf(buffer, sizeof buffer, "%d%d:%d%d:%d%d",time[5],time[4],time[3],time[2],time[1],time[0]);
	lcd_puts(buffer);
	lcd_goto(1,0);
	snprintf(buffer, sizeof buffer, "%d%d:%d%d:%d%d",time[8],time[7],time[10],time[9],time[12],time[11]);
	lcd_puts(buffer);
}

void changeTime(void)
{
	lcd_cursor(true,true);
	lcd_home();
	 while (!(PIND & (1<<0))){
		if (PIND & (1<<1))
		{
			
		}
		else if(PIND & (1<<2))
		{
			
		}
		else if (PIND & (1<<3))
		{
			
		}
	 }
}

ISR(TIMER0_OVF_vect) {
	static unsigned char x = 0;

	if ( x == 15 )
	{
		TWI.start_wait(I2C_WRITE);			// set device address and write mode
		TWI.readXBytes(data,7,0x00);
		TWI.stop();
		for (uint8_t i = 0, j = 0; i <= 6 ; i++, j++)
		{
			if ( i != 3 )
			{
				time[j] = bcdToDec((data[i] & 0x0F));
				time[j + 1] = bcdToDec(((data[i] >> 4) & 0x0F));
//				time[i] = bcdToDec(data[i]);
				j++;
			}
			else
			{
				time[j] = bcdToDec(data[i]);
			}
		}
		
		x = 0;
	}
	else
	{
		x++;
	}
	TCNT0 = 0;
}

int main(void)
{
	DDRD = 0x00;
	lcd_init();
	lcd_cursor(false, false);							//  cursor off
	lcd_home();
	snprintf(buffer, sizeof buffer, "int");
	lcd_puts(buffer);
	lcd_home();
	TWI.init();									// Function to initialize TWI
	int ret = TWI.start(I2C_WRITE);				// set device address and write mode
	if ( ret ) 
	{										// failed to issue start condition, possibly no device found
		TWI.stop();
		snprintf(buffer, sizeof buffer, "Failed");
		lcd_puts(buffer);
		lcd_home();
	}
	else 
	{
//		TWI.stop();
//		TWI.start_wait(I2C_WRITE);
//		TWI.writeXBytes(timeIn,8);
//		TWI.stop();

		init_T0();
		sei();
		while(1)
		{
			cli();
			writeScreen();
			sei();
//			if (PIND & (1<<0))
//			{
//				changeTime();	
//			}
		}
	}
}