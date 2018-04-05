#ifndef  F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/twi.h>

#include "i2c_master.h"

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val 0x20 // calculated: ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c_init(void)
{
	TWBR = (char)TWBR_val;
}

char i2c_start(char address)
{
	// reset TWI control register
	//TWCR = 0;
	// transmit START condition 
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	// check if the device has acknowledged the READ / WRITE mode
	char twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) {return 2;}
	

	return 0;
}

char i2c_write(char data)
{
	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	return 0;
}

char i2c_read_ack(void)
{
	
	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA); 
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) ) {;}
	// return received data from TWDR
	return TWDR;
}

char i2c_read_nack(void)
{
	
	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) ) {;}
	// return received data from TWDR
	return TWDR;
}

char i2c_transmit(char address, char* data, int length)
{
	if (i2c_start(address | I2C_WRITE)) {return 1;}
	
	for (int i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) {return 1;}
	}
	
	i2c_stop();
	
	return 0;
}

char i2c_receive(char address, char* data, int length)
{
	if (i2c_start(address | I2C_READ)) {return 1;}
	
	for (int i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();
	
	i2c_stop();
	
	return 0;
}

char i2c_writeReg(char devaddr, char regaddr, char* data, int length)
{
	if (i2c_start(devaddr | 0x00)) { return 1; }

	i2c_write(regaddr);

	for (int i = 0; i < length; i++)
	{
		if (i2c_write(data[i])){ return 1; }
	}

	i2c_stop();

	return 0;
}

char i2c_readReg(char devaddr, char regaddr, char* data, int length)
{
	if (i2c_start(devaddr)) { return 1; }

	i2c_write(regaddr);

	if (i2c_start(devaddr | 0x01)) { return 1; }

	for (int i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	return 0;
}

void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}
