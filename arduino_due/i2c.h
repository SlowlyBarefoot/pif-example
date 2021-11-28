#ifndef I2C_H
#define I2C_H


#define I2C_CLOCK_100KHz	100000
#define I2C_CLOCK_400KHz	400000


#ifdef __cplusplus
extern "C" {
#endif

void I2C_Init(uint32_t clock);
void I2C_Exit();

void I2C_XmitTimeout(uint32_t timeout);
void I2C_RecvTimeout(uint32_t timeout);

BOOL I2C_Write(uint8_t address, uint8_t* p_data, uint8_t size);
BOOL I2C_Read(uint8_t address, uint8_t* p_data, uint8_t size);

#ifdef __cplusplus
}
#endif


#endif	// I2C_H
