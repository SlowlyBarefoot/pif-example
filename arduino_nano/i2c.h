#ifndef I2C_H
#define I2C_H


#define I2C_CLOCK_100KHz	100000
#define I2C_CLOCK_400KHz	400000


#ifdef __cplusplus
extern "C" {
#endif

void I2C_Init(uint32_t clock);

BOOL I2C_ReadAddr(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size);
BOOL I2C_Read(uint8_t addr, uint8_t* p_data, uint8_t size);

BOOL I2C_WriteAddr(uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size);
BOOL I2C_Write(uint8_t addr, uint8_t* p_data, uint8_t size);

#ifdef __cplusplus
}
#endif


#endif	// I2C_H
