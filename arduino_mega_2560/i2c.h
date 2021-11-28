#ifndef I2C_H
#define I2C_H


#define I2C_CLOCK_100KHz	100000
#define I2C_CLOCK_400KHz	400000

#define I2C_MODE_WRITE		0
#define I2C_MODE_READ		1


#ifdef __cplusplus
extern "C" {
#endif

void I2C_Init(uint32_t clock);

BOOL I2C_Start(uint8_t address, uint8_t mode);

BOOL I2C_Write(uint8_t* p_data, uint8_t size);
BOOL I2C_Read(uint8_t* p_data, uint8_t size);

void I2C_Stop(uint8_t delay_1ms);

#ifdef __cplusplus
}
#endif


#endif	// I2C_H
