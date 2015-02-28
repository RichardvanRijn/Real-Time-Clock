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

I2C TWI(address);

unsigned char index[2][16] ={
							{ 5, 4, 16, 3, 2, 16, 1, 0, 16, 16, 16, 16, 16, 16, 16, 16 },
							{ 8, 7, 16, 10, 9, 16, 12, 11, 16 ,16, 16, 16, 16, 16, 16, 16}
							};

void init_T0(void)
{
	TCCR0 = (1<<CS02) && (1<<CS00);
	TCNT0 = 0;
	TIMSK = (1<<TOIE0);
}

void init_lcd(void)
{
	lcd_init();
	lcd_cursor(false, false);							//  cursor off
	lcd_home();
}

void init(void)
{
	DDRD = 0x00;
	init_lcd();
	TWI.init();												// Function to initialize TWI
	unsigned char ret = TWI.start(I2C_WRITE);				// set device address and write mode
	while(ret)
	{
		TWI.stop();
		ret = TWI.start(I2C_WRITE);
		snprintf(buffer, sizeof buffer, "Failed");
		lcd_puts(buffer);
		lcd_home();
	}
	init_T0();
}

void t1_Stop_TOI(void)
{
	TIMSK &= ~(1<<TOIE0);
}

void t1_Start_TOI(void)
{
	TIMSK |= (1<<TOIE0);
}

unsigned char bcdToDec(unsigned char val)
{
	return ((val/16*10) + (val%16) );
}

unsigned char decToBcd(unsigned char val)
{
	return ((val/10*16) + (val%10) );
}

//unsigned char timeIn[8] = {0x00,	 decToBcd(30), decToBcd(41), decToBcd(17), decToBcd(5),	 decToBcd(6), decToBcd(2), decToBcd(15)};
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
	uint8_t x = 0, y = 0;
	t1_Stop_TOI();
	lcd_cursor(true,true);
	lcd_home();
	while ((PIND & (1<<PD0)) == 1) {}
	_delay_ms(100);
	while ((PIND & (1<<PD0)) != 1)						//button 1
	{
		if ((PIND & (1<<PD1)) == 2)						//button 2
		{
			if (x == 7)
			{
				x = 0;
				y = 1 - y;
				lcd_goto(y,x);
			}
			else
			{
				if (x == 1 || x == 4)
				{
					x++;	
				}
				x++;
				lcd_goto(y,x);
			}
			while((PIND & (1<<PD1)) == 2) {}
			_delay_ms(100);
		}
		else if ((PIND & (1<<PD2)) == 4)				//button 3
		{
			time[index[lcd_get_row()][lcd_get_column()]]++;
			writeScreen();
			lcd_goto(y,x);
			while ((PIND & (1<<PD2)) == 4) {}
			_delay_ms(100);
		}
		else if ((PIND & (1<<PD3)) == 8)				// button 4
		{
			time[index[lcd_get_row()][lcd_get_column()]]--;
			writeScreen();
			lcd_goto(y,x);
			while ((PIND & (1<<PD3)) == 8) {}
			_delay_ms(100);
		}
	}
	for (uint8_t i = 1, j = 0; i <= 7 ; i++, j++)
	{
		if ( i != 4 )
		{
			data[i] = ((time[j] & 0x0F) + (time[j+1] << 4));
			j++;
		}
		else
		{
			data[i] = time[j];
		}
	}
	data[0] = 0;
	TWI.start_wait(I2C_WRITE);
	TWI.writeXBytes(data,8);
	TWI.stop();
	t1_Start_TOI();
	lcd_cursor(false,false);
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
				time[j] = ((data[i] & 0x0F));
				time[j + 1] = (((data[i] >> 4) & 0x0F));
				j++;
			}
			else
			{
				time[j] = data[i];
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
	init();
	sei();
	while(1)
	{
		cli();
		writeScreen();
		sei();
		if ((PIND & (1<<PD0)) == 1)
		{
			changeTime();
			while((PIND & (1<<PD0)) == 1) {}
			_delay_ms(100);
		}
	}
}