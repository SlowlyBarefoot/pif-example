#include "core/pif_task.h"
#include "i2c.h"

#include <avr/io.h>      // Contains all the I/O Register Macros
#include <avr/interrupt.h> // Contains all interrupt vectors


#define I2C_MODE_WRITE		0
#define I2C_MODE_READ		1


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

static BOOL _startTransmission(uint8_t addr, uint8_t mode)
{
	if (!_waitTransmission((1 << TWINT) | (1 << TWSTA) | (1 << TWEN))) return FALSE;	// send REPEAT START condition and wait until transmission completed
	TWDR = (addr << 1) | mode;                              							// send device address
	if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;            	  	// wail until transmission completed
	return TRUE;
}

static void _stopTransmission()
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void I2C_Init(uint32_t clock)
{
	TWSR = 0;                                   // no prescaler => prescaler = 1
	TWBR = ((F_CPU / clock) - 16) / 2;          // set the I2C clock rate to 400kHz
	TWCR = 1 << TWEN;                           // enable twi module, no interrupt
}

BOOL I2C_ReadAddr(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size)
{
	int i;

	if (isize > 0) {
		if (!_startTransmission(addr, I2C_MODE_WRITE)) return FALSE;

		for (i = isize - 1; i >= 0; i--) {
			TWDR = (iaddr >> (i * 8)) & 0xFF;
			if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
		}
	}

	if (!_startTransmission(addr, I2C_MODE_READ)) return FALSE;

	for (i = 0; i < size - 1; i++) {
		if (!_waitTransmission((1 << TWINT) | (1 << TWEN) | (1 << TWEA))) return FALSE;
		p_data[i] = TWDR;
	}
	if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
	p_data[i] = TWDR;

	_stopTransmission();
	return TRUE;
}

BOOL I2C_Read(uint8_t addr, uint8_t* p_data, uint8_t size)
{
	return I2C_ReadAddr(addr, 0, 0, p_data, size);
}

BOOL I2C_WriteAddr(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size)
{
	int i;

	if (!_startTransmission(addr, I2C_MODE_WRITE)) return FALSE;

	if (isize > 0) {
		for (i = isize - 1; i >= 0; i--) {
			TWDR = (iaddr >> (i * 8)) & 0xFF;
			if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
		}
	}
	for (i = 0; i < size; i++) {
		TWDR = p_data[i];
		if (!_waitTransmission((1 << TWINT) | (1 << TWEN))) return FALSE;
	}

	_stopTransmission();
	return TRUE;
}

BOOL I2C_Write(uint8_t addr, uint8_t* p_data, uint8_t size)
{
	return I2C_WriteAddr(addr, 0, 0, p_data, size);
}

