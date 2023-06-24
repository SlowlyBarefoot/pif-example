#include "core/pif_task.h"
#include "i2c.h"

// Include Atmel CMSIS driver
#include <include/twi.h>

#include "variant.h"


typedef struct
{
	Twi *twi;
	uint8_t id;
	IRQn_Type irqn;

	uint32_t xmit_timeout;
	uint32_t recv_timeout;
} ArduinoDueI2c;


static ArduinoDueI2c s_i2c[2];


static BOOL TWI_WaitTransferComplete(Twi *_twi, uint32_t _timeout)
{
	uint32_t _status_reg = 0;

	while ((_status_reg & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK) {
			pif_error = E_RECEIVE_NACK;
			return FALSE;
		}

		if (--_timeout == 0) {
			pif_error = E_TIMEOUT;
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL TWI_WaitByteSent(Twi *_twi, uint32_t _timeout)
{
	uint32_t _status_reg = 0;

	while ((_status_reg & TWI_SR_TXRDY) != TWI_SR_TXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK) {
			pif_error = E_RECEIVE_NACK;
			return FALSE;
		}

		if (--_timeout == 0) {
			pif_error = E_TIMEOUT;
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL TWI_WaitByteReceived(Twi *_twi, uint32_t _timeout)
{
	uint32_t _status_reg = 0;

	while ((_status_reg & TWI_SR_RXRDY) != TWI_SR_RXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK) {
			pif_error = E_RECEIVE_NACK;
			return FALSE;
		}

		if (--_timeout == 0) {
			pif_error = E_TIMEOUT;
			return FALSE;
		}
	}

	return TRUE;
}

void I2C_Init(uint8_t port, uint32_t clock)
{
	ArduinoDueI2c* p_i2c = &s_i2c[port];

	switch (port) {
	case I2C_PORT_0:
		s_i2c[port].twi = WIRE_INTERFACE;
		s_i2c[port].id = WIRE_INTERFACE_ID;
		p_i2c->irqn = WIRE_ISR_ID;
		PIO_Configure(
				g_APinDescription[PIN_WIRE_SDA].pPort,
				g_APinDescription[PIN_WIRE_SDA].ulPinType,
				g_APinDescription[PIN_WIRE_SDA].ulPin,
				g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
		PIO_Configure(
				g_APinDescription[PIN_WIRE_SCL].pPort,
				g_APinDescription[PIN_WIRE_SCL].ulPinType,
				g_APinDescription[PIN_WIRE_SCL].ulPin,
				g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);
		break;

	case I2C_PORT_1:
		p_i2c->twi = WIRE1_INTERFACE;
		p_i2c->id = WIRE1_INTERFACE_ID;
		p_i2c->irqn = WIRE1_ISR_ID;
		PIO_Configure(
				g_APinDescription[PIN_WIRE1_SDA].pPort,
				g_APinDescription[PIN_WIRE1_SDA].ulPinType,
				g_APinDescription[PIN_WIRE1_SDA].ulPin,
				g_APinDescription[PIN_WIRE1_SDA].ulPinConfiguration);
		PIO_Configure(
				g_APinDescription[PIN_WIRE1_SCL].pPort,
				g_APinDescription[PIN_WIRE1_SCL].ulPinType,
				g_APinDescription[PIN_WIRE1_SCL].ulPin,
				g_APinDescription[PIN_WIRE1_SCL].ulPinConfiguration);
		break;

	default:
		return;
	}

	pmc_enable_periph_clk(p_i2c->id);

	NVIC_DisableIRQ(p_i2c->irqn);
	NVIC_ClearPendingIRQ(p_i2c->irqn);
	NVIC_SetPriority(p_i2c->irqn, 0);
	NVIC_EnableIRQ(p_i2c->irqn);

	// Disable PDC channel
	p_i2c->twi->TWI_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

	TWI_ConfigureMaster(p_i2c->twi, clock, VARIANT_MCK);
}

void I2C_Exit(uint8_t port)
{
	ArduinoDueI2c* p_i2c = &s_i2c[port];

	TWI_Disable(p_i2c->twi);

	// Enable PDC channel
	p_i2c->twi->TWI_PTCR &= ~(UART_PTCR_RXTDIS | UART_PTCR_TXTDIS);

	NVIC_DisableIRQ(p_i2c->irqn);
	NVIC_ClearPendingIRQ(p_i2c->irqn);

	pmc_disable_periph_clk(p_i2c->id);

	// no need to undo PIO_Configure,
	// as Peripheral A was enable by default before,
	// and pullups were not enabled
}

void I2C_XmitTimeout(uint8_t port, uint32_t timeout)
{
	s_i2c[port].xmit_timeout = timeout;
}

void I2C_RecvTimeout(uint8_t port, uint32_t timeout)
{
	s_i2c[port].recv_timeout = timeout;
}

BOOL I2C_ReadAddr(uint8_t port, uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size)
{
	ArduinoDueI2c* p_i2c = &s_i2c[port];
	int readed = 0;

	TWI_StartRead(p_i2c->twi, addr, iaddr, isize);
	do {
		// Stop condition must be set during the reception of last byte
		if (readed + 1 == size)
			TWI_SendSTOPCondition(p_i2c->twi);

		if (TWI_WaitByteReceived(p_i2c->twi, p_i2c->recv_timeout))
			p_data[readed++] = TWI_ReadByte(p_i2c->twi);
		else
			break;
	}
	while (readed < size);
	if (!TWI_WaitTransferComplete(p_i2c->twi, p_i2c->recv_timeout)) return FALSE;
	return readed >= size;
}

BOOL I2C_Read(uint8_t port, uint8_t addr, uint8_t* p_data, uint8_t size)
{
	return I2C_ReadAddr(port, addr, 0, 0, p_data, size);
}

BOOL I2C_WriteAddr(uint8_t port, uint8_t addr, uint32_t iaddr, uint8_t isize, uint8_t* p_data, uint8_t size)
{
	ArduinoDueI2c* p_i2c = &s_i2c[port];
	pif_error = E_SUCCESS;

	TWI_StartWrite(p_i2c->twi, addr, iaddr, isize, p_data[0]);
	TWI_WaitByteSent(p_i2c->twi, p_i2c->xmit_timeout);

	if (pif_error == E_SUCCESS) {
		uint16_t sent = 1;
		while (sent < size) {
			TWI_WriteByte(p_i2c->twi, p_data[sent++]);
			if (!TWI_WaitByteSent(p_i2c->twi, p_i2c->xmit_timeout)) break;
		}
	}

	if (pif_error == E_SUCCESS) {
		TWI_Stop(p_i2c->twi);
		TWI_WaitTransferComplete(p_i2c->twi, p_i2c->xmit_timeout);
	}
	return pif_error == E_SUCCESS;
}

BOOL I2C_Write(uint8_t port, uint8_t addr, uint8_t* p_data, uint8_t size)
{
	return I2C_WriteAddr(port, addr, 0, 0, p_data, size);
}

