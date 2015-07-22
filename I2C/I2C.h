/* 
* I2C.h
*
* Created: 30-1-2015 14:28:22
* Author: Richard
*/
#include <avr/io.h>
//#include <vector>

#ifndef __I2C_H__
#define __I2C_H__

#define I2C_READ    1
#define I2C_WRITE   0

#define adressAmmount 1

#if adressAmmount < 1
#error adressAmmount to low
#endif

class I2C
{
//variables
public:
protected:
private:
	unsigned char address;
	unsigned char addresses[adressAmmount];

//functions
public:
	I2C();
	I2C(unsigned char newaddress);
	I2C(unsigned char newaddress[]);
	
	void init(void);
	unsigned char start(unsigned char RW);
	unsigned char rep_start(unsigned char RW);
	void start_wait(unsigned char RW);
	void stop(void);
	
	void new_address(unsigned char newaddress);
	unsigned char get_address(void);
	
	void readXBytes(unsigned char data[], unsigned char amount, unsigned char startAddress);
	
	void writeXBytes(unsigned char data[], unsigned int amount);

	~I2C();
protected:
private:
	I2C( const I2C &c );
	I2C& operator=( const I2C &c );
	
	unsigned char write(unsigned char data);
	unsigned char readAck(void);
	unsigned char readNak(void);
		
}; //I2C

#endif //__I2C_H__
