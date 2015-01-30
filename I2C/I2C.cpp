/* 
* I2C.cpp
*
* Created: 30-1-2015 14:28:22
* Author: Richard
*/
#include <inttypes.h>
#include <compat/twi.h>

#include "I2C.h"

#ifndef F_CPU
#define F_CPU 4000000UL
#endif

/* I2C clock in Hz */
#define SCL_CLOCK  100000L

// default constructor
I2C::I2C():address(0) {}

I2C::I2C(unsigned char newaddress) : address(newaddress) {}

void I2C::init(void)
{
	 /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
	 TWSR = 0;                         /* no prescaler */
	 TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */
}

void I2C::write_address(unsigned char newaddress)
{
	address = newaddress;
}

unsigned char I2C::start(unsigned char RW)
{
	uint8_t   twst;
	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));
	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
	// send device address
	TWDR = address + RW;
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));
	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	return 0;
}

unsigned char I2C::rep_start(unsigned char RW)
{
	return start(RW);
}

void I2C::start_wait(unsigned char RW)
{
	uint8_t   twst;
	while ( 1 )
	{
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);						// send START condition
		while(!(TWCR & (1<<TWINT)));									// wait until transmission completed
		twst = TW_STATUS & 0xF8;										// check value of TWI Status Register. Mask prescaler bits.
		if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
		TWDR = address + RW;											// send device address
		TWCR = (1<<TWINT) | (1<<TWEN);
		while(!(TWCR & (1<<TWINT)));									// wail until transmission completed
		twst = TW_STATUS & 0xF8;										// check value of TWI Status Register. Mask prescaler bits.
		if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) )
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);					//device busy, send stop condition to terminate write operation
			while(TWCR & (1<<TWSTO));									// wait until stop condition is executed and bus released
			continue;
		}
		break;															//if( twst != TW_MT_SLA_ACK) return 1;
	}
}

void I2C::stop(void)
{
	 TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);						//send stop condition
	 while(TWCR & (1<<TWSTO));											// wait until stop condition is executed and bus released
}


unsigned char I2C::write(unsigned char data)
{
	uint8_t   twst;
	TWDR = data;														// send data to the previously addressed device
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));										// wait until transmission completed
	
	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;											
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}

unsigned char I2C::readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

unsigned char I2C::readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));	
	return TWDR;
}

unsigned char I2C::read_address(void)
{
	return address;
}

// default destructor
I2C::~I2C()
{
} //~I2C