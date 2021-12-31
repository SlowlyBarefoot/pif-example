#include <avr/io.h>      // Contains all the I/O Register Macros
#include <avr/interrupt.h> // Contains all interrupt vectors

#include "pif_task.h"
#include "i2c.h"


static BOOL _waitTransmission(uint8_t twcr)
{
	TWCR = twcr;
	uint8_t count = 255;
	while (!(TWCR & (1 << TWINT))) {
		pifTaskManager_Yield();

	    count--;
	    if (count == 0) {              			// we are in a blocking state => we don't insist
	    	TWCR = 0;                  			// and we force a reset on TWINT register
	    	pif_error = E_TIMEOUT;
	    	return FALSE;
	    }
	}
	return TRUE;
}

void I2C_Init(uint32_t clock)
{
	TWSR = 0;                                   // no prescaler => prescaler = 1
	TWBR = ((F_CPU / clock) - 16) / 2;          // set the I2C clock rate to 400kHz
	TWCR = 1 << TWEN;                           // enable twi module, no interrupt
}

BOOL I2C_Start(uint8_t address, uint8_t mode)
{
	if (!_waitTransmission((1 << TWINT) | (1 << TWSTA) | (1 << TWEN))) return FALSE;	// send REPEAT START condition and wait until transmission completed
	TWDR = (address << 1) | mode;                              							// send device address
	if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;            	  	// wail until transmission completed
	return TRUE;
}

BOOL I2C_Write(uint8_t* p_data, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size; i++) {
		TWDR = p_data[i];                       // send data to the previously addressed device
		if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
	}
	return TRUE;
}

BOOL I2C_Read(uint8_t* p_data, uint8_t size)
{
	uint8_t i;

	for (i = 0; i < size - 1; i++) {
		if (!_waitTransmission((1 << TWINT) | (1 << TWEN) | (1 << TWEA))) return FALSE;
		p_data[i] = TWDR;
	}
	if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
	p_data[i] = TWDR;
	return TRUE;
}

void I2C_Stop(uint8_t delay_1ms)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

	if (delay_1ms) pifTaskManager_YieldMs(delay_1ms);
}
