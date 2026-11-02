#include "main.h"
#include "app_port.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"


extern USBD_HandleTypeDef hUsbDeviceFS;

static volatile BOOL s_dtr;				// The terminal on the PC has the port open
static volatile BOOL s_rx_paused;		// A packet came that the RX buffer had no room for the next of


uint32_t actMicros()
{
    return htim2.Instance->CNT;
}

static BOOL _isConfigured(void)
{
	return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED;
}

// CDC_Transmit_FS() sends from the pointer it is given after it returns, so the packet of pifUsbCdc must not
// be written while the class sends it: no room while TxState is set.
static uint16_t _actCdcGetRoom(PifUsbCdc *p_cdc)
{
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;

	(void)p_cdc;
	if (!_isConfigured() || !hcdc || hcdc->TxState) return 0;
	return PIF_USB_CDC_PACKET_SIZE;
}

static uint16_t _actCdcWrite(PifUsbCdc *p_cdc, uint8_t *p_data, uint16_t size)
{
	(void)p_cdc;
	return CDC_Transmit_FS(p_data, size) == USBD_OK ? size : 0;
}

static BOOL _actCdcIsOpen(PifUsbCdc *p_cdc)
{
	(void)p_cdc;
	return _isConfigured() && s_dtr;
}

// CDC_Receive_FS: the packet goes into the RX buffer. The next one is armed only while there is room for a whole
// packet, so the host waits (NAK) instead of losing bytes.
BOOL portCdcReceive(uint8_t *p_data, uint32_t length)
{
	pifUsbCdc_SigReceive(&g_cdc, p_data, (uint16_t)length);
	if (pifUsbCdc_GetRxRoom(&g_cdc) >= PIF_USB_CDC_PACKET_SIZE) return TRUE;
	s_rx_paused = TRUE;
	return FALSE;
}

// CDC_TransmitCplt_FS
void portCdcTxComplete(void)
{
	pifUsbCdc_SigTxComplete(&g_cdc);
}

// CDC_Control_FS, CDC_SET_CONTROL_LINE_STATE: bit 0 is DTR.
void portCdcLineState(uint16_t state)
{
	s_dtr = (state & 1) != 0;
	if (s_dtr) pifUsbCdc_SigTxComplete(&g_cdc);		// what waits goes out
}

// Arms the receive again once the protocol made room.
static uint32_t _taskCdcRxResume(PifTask *p_task)
{
	(void)p_task;
	if (s_rx_paused && pifUsbCdc_GetRxRoom(&g_cdc) >= PIF_USB_CDC_PACKET_SIZE) {
		s_rx_paused = FALSE;
		CDC_ArmReceive();
	}
	return 0;
}

static void actLedLState(PifId usPifId, uint32_t unState)
{
	(void)usPifId;

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, unState & 1);
}

BOOL portSetup()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

	if (!pifTaskManager_Init(TASK_SIZE, 1)) return FALSE;

	// USB CDC: the log and the command line. Received packets come from CDC_Receive_FS into the RX buffer.
	if (!pifUsbCdc_Init(&g_cdc, PIF_ID_AUTO, 1024, 256)) return FALSE;
	g_cdc.act_get_room = _actCdcGetRoom;
	g_cdc.act_write = _actCdcWrite;
	g_cdc.act_is_open = _actCdcIsOpen;
	if (!pifUart_AttachTxTask(&g_cdc.parent, PIF_ID_AUTO, TM_PERIOD, 1000, "CdcTx")) return FALSE;		// 1ms
	if (!pifUart_AttachRxTask(&g_cdc.parent, PIF_ID_AUTO, TM_PERIOD, 10000, "CdcRx")) return FALSE;	// 10ms
	if (!pifTaskManager_Add(PIF_ID_AUTO, TM_PERIOD, 1000, _taskCdcRxResume, NULL, TRUE)) return FALSE;

	pifLog_Init();
	if (!pifLog_AttachUart(&g_cdc.parent, 256)) return FALSE;											// 256bytes

	if (!pifLed_Init(&g_led_l, PIF_ID_AUTO, 1, actLedLState)) return FALSE;
	return TRUE;
}
