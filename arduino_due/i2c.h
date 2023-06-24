#ifndef I2C_H
#define I2C_H


#define I2C_PORT_0			0
#define I2C_PORT_1			1

#define I2C_CLOCK_100KHz	100000
#define I2C_CLOCK_400KHz	400000


#ifdef __cplusplus
extern "C" {
#endif

void I2C_Init(uint8_t port, uint32_t clock);
void I2C_Exit(uint8_t port);

void I2C_XmitTimeout(uint8_t port, uint32_t timeout);
void I2C_RecvTimeout(uint8_t port, uint32_t timeout);

BOOL I2C_ReadAddr(uint8_t port, uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size);
BOOL I2C_Read(uint8_t port, uint8_t addr, uint8_t* p_data, uint8_t size);

BOOL I2C_WriteAddr(uint8_t port, uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size);
BOOL I2C_Write(uint8_t port, uint8_t addr, uint8_t* p_data, uint8_t size);

#ifdef __cplusplus
}
#endif


#endif	// I2C_H
