/* 
* I2C.h
*
* Created: 30-1-2015 14:28:22
* Author: Richard
*/
#include <avr/io.h>

#ifndef __I2C_H__
#define __I2C_H__



class I2C
{
//variables
public:
protected:
private:

#define I2C_READ    1
#define I2C_WRITE   0

unsigned char address;

//functions
public:
	I2C();
	I2C(unsigned char newaddress);
	
	void init(void);
	unsigned char start(unsigned char RW);
	unsigned char rep_start(unsigned char RW);
	void start_wait(unsigned char RW);
	void stop(void);
		
	unsigned char write(unsigned char data);
		
	unsigned char readAck(void);
	unsigned char readNak(void);
		
	unsigned char i2c_read(unsigned char ack);
	#define i2c_read(ack)  (ack) ? i2c_readAck() : i2c_readNak();
	unsigned char read_address(void);
	void write_address(unsigned char newaddress);
	
	~I2C();
protected:
private:
	I2C( const I2C &c );
	I2C& operator=( const I2C &c );
	

	
}; //I2C

#endif //__I2C_H__
