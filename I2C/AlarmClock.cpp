#ifndef F_CPU
#define F_CPU 4000000UL
#endif

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
unsigned char realTime[7];
unsigned char data[8];

unsigned char alarmTime[7] = {0,0,0,0,0,0,0};
	
bool alarmSet = false;

I2C TWI(address);

unsigned char index[2][16] ={
							{ 2, 2, 16, 1, 1, 16, 0, 0, 16, 16, 16, 16, 16, 16, 16, 16 },
							{ 4, 4, 16, 5, 5, 16, 6, 6, 16 ,16, 16, 16, 16, 16, 16, 16 }
							};

unsigned char MaxVal[2][8] = {
							 { 23,23,0,59,59,0,59,59 },
							 { 31,31,0,12,12,0,99,99 }
							 };
							 
unsigned char bcdToDec(unsigned char val)
{
	return ((val/16*10) + (val%16) );
}

unsigned char decToBcd(unsigned char val)
{
	return ((val/10*16) + (val%10) );
}

void init_T0(void)
{
	TCCR0 = (1<<CS02) && (1<<CS00);
	TCNT0 = 0;
	TIMSK = (1<<TOIE0);
}

void init_lcd(void)
{
	lcd_init();
	lcd_cursor(false, false);								//  cursor off
	lcd_home();
}

void init(void)
{
	DDRD = 0x00;
	DDRB = 0xFF;
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
	TWI.start_wait(I2C_WRITE);						// set device address and write mode
	TWI.readXBytes(data,7,0x00);
	TWI.stop();
	for (uint8_t i = 0; i <= 6 ; i++)
	{
		realTime[i] = bcdToDec(data[i]);
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

void Alarm_start(void)
{
	PORTB = 0x01;
}

void Alarm_stop(void)
{
	PORTB = 0x00;
}

//
//unsigned char timeIn[7] = {	decToBcd(30), decToBcd(41), decToBcd(17), decToBcd(5),	 decToBcd(6), decToBcd(2), decToBcd(15)};
//								Seconds,	  Minutes,	   Hours,	    Day of the week, Date,		  month,	   Year.

void writeScreen(unsigned char time[])
{
	lcd_home();
	for(int8_t i = 2;i>=0;i--)
	{
		if (time[i] < 10)
		{
			lcd_putc('0');	
		}
		snprintf(buffer,sizeof buffer,"%d",time[i]);
		lcd_puts(buffer);
		if(i>0)
		{
			lcd_putc(':');	
		}
	}
	lcd_goto(1,0);
	for(int8_t i = 4;i<=6;i++)
	{
		if (time[i] < 10)
		{
			lcd_putc('0');
		}
		snprintf(buffer,sizeof buffer,"%d",time[i]);
		lcd_puts(buffer);
		if(i<6)
		{
			lcd_putc(':');
		}
	}
	lcd_goto(0,15);
	if (alarmSet == true){
		lcd_putc('A');
	}
	else
	{
		lcd_putc(' ');
	}	
}

void changeTime(bool alarm,unsigned char time[])
{
	uint8_t x = 0, y = 0;
	writeScreen(time);
	t1_Stop_TOI();
	lcd_cursor(true,true);
	lcd_home();
	if (alarm == false)
	{
		while ((PIND & (1<<PD0)) == (1<<PD0)) {}
		_delay_ms(25);
	}
	else
	{
		while ((PIND & (1<<PD4)) == (1<<PD4)) {}
		_delay_ms(25);
	}
	while ((PIND & (1<<PD0)) != (1<<PD0))						//button 1
	{
		if ((PIND & (1<<PD1)) == (1<<PD1))						//button 2
		{
			if (x == 7)
			{
				x = 0;
				y = 1 - y;
			}
			else
			{
				if (x == 1 || x == 4)
				{
					x++;	
				}
				x++;
			}
			lcd_goto(y,x);
			while((PIND & (1<<PD1)) == 2) {}
			_delay_ms(25);
		}
		else if ((PIND & (1<<PD2)) == (1<<PD2))				//button 3	UP
		{
			if (x == 0 || x == 3 || x == 6)
			{
				time[index[lcd_get_row()][lcd_get_column()]] += 10;
			}
			else
			{
				time[index[lcd_get_row()][lcd_get_column()]] ++;
			}
			if (time[index[lcd_get_row()][lcd_get_column()]] >= MaxVal[lcd_get_row()][lcd_get_column()])
			{
				time[index[lcd_get_row()][lcd_get_column()]] -= MaxVal[lcd_get_row()][lcd_get_column()];
			}
			writeScreen(time);
			lcd_goto(y,x);
			while ((PIND & (1<<PD2)) == 4) {}
			_delay_ms(25);
		}
		else if ((PIND & (1<<PD3)) == (1<<PD3))				//button 4	Down
		{
			if (x == 0 || x == 3 || x == 6){
				time[index[lcd_get_row()][lcd_get_column()]] -= 10;
			}
			else
			{
				time[index[lcd_get_row()][lcd_get_column()]] --;
			}
			if (time[index[lcd_get_row()][lcd_get_column()]] >= MaxVal[lcd_get_row()][lcd_get_column()])
			{
				time[index[lcd_get_row()][lcd_get_column()]] += MaxVal[lcd_get_row()][lcd_get_column()];
			}
			writeScreen(time);
			lcd_goto(y,x);
			while ((PIND & (1<<PD3)) == (1<<PD3)) {}
			_delay_ms(25);
		}
	}
	if(alarm == false)
	{
		for (uint8_t i = 1; i <= 7 ; i++)
		{
			data[i] = decToBcd(time[i-1]);
		}
		data[0] = 0;
		TWI.start_wait(I2C_WRITE);
		TWI.writeXBytes(data,8);
		TWI.stop();
	}
	else
	{
		writeScreen(realTime);	
	}
	t1_Start_TOI();
	lcd_cursor(false,false);
}

ISR(TIMER0_OVF_vect) {
	static unsigned char x = 0;
	if ( x == 15 )
	{
		TWI.start_wait(I2C_WRITE);						// set device address and write mode
		TWI.readXBytes(data,7,0x00);
		TWI.stop();
		for (uint8_t i = 0; i <= 6 ; i++)
		{
			realTime[i] = bcdToDec(data[i]);
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
		writeScreen(realTime);
		sei();
		if ((PIND & (1<<PD0)) == (1<<PD0))
		{
			changeTime(false, realTime);
			while((PIND & (1<<PD0)) == (1<<PD0)) {}
			_delay_ms(25);
		}
		if ((PIND & (1<<PD4)) == (1<<PD4))
		{
			changeTime(true, alarmTime);
			while((PIND & (1<<PD0)) == (1<<PD0)) {}
			_delay_ms(25);
		}
		if ((PIND & (1<<PD6)) == (1<<PD6))
		{
			alarmSet = true;
			if (realTime[0] == alarmTime[0] && realTime[1] == alarmTime[1] && realTime[2] == alarmTime[2])
			{
				Alarm_start();
			}
		}
		else
		{
			alarmSet = false;
			Alarm_stop();
		}
	}
}
