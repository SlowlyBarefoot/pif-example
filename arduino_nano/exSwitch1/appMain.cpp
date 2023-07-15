#include "appMain.h"
#include "exSwitch1.h"

#include "core/pif_log.h"
#include "sensor/pif_sensor_switch.h"


static PifSensorSwitch s_push_switch;
static PifSensorSwitch s_tilt_switch;


void appSetup()
{
	static PifUart s_uart_log;

    pif_Init(NULL);

    if (!pifTaskManager_Init(4)) return;

    pifLog_Init();

	if (!pifUart_Init(&s_uart_log, PIF_ID_AUTO)) return;
    if (!pifUart_AttachTask(&s_uart_log, TM_PERIOD_MS, 1, NULL)) return;			   	// 1ms
	s_uart_log.act_send_data = actLogSendData;

	if (!pifLog_AttachUart(&s_uart_log)) return;

    if (!pifSensorSwitch_Init(&s_push_switch, PIF_ID_AUTO, OFF, actPushSwitchAcquire, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&s_push_switch, TM_RATIO, 3, TRUE)) return;	// 3%
    pifSensor_AttachEvtChange(&s_push_switch.parent, evtPushSwitchChange);

	if (!pifSensorSwitch_Init(&s_tilt_switch, PIF_ID_AUTO, OFF, actTiltSwitchAcquire, NULL)) return;
    if (!pifSensorSwitch_AttachTaskAcquire(&s_tilt_switch, TM_RATIO, 3, TRUE)) return;	// 3%
	pifSensor_AttachEvtChange(&s_tilt_switch.parent, evtTiltSwitchChange);

    if (!pifTaskManager_Add(TM_PERIOD_MS, 500, taskLedToggle, NULL, TRUE)) return;		// 500ms

	pifLog_Printf(LT_INFO, "Task=%d\n", pifTaskManager_Count());
}
