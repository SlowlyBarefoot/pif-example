/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다. 500ms마다 이벤트를 발생시키고 이벤트에서 LED를 깜박가리게 한다.
 * LED는 console에 그린다.
 *
 * Create one pulse and connect it to the 1ms timer. Generate an event every 500ms and flash the LED in the event.
 * The LED is drawn on the console.
 */
#include "host.h"
#include "app_port.h"


int main()
{
	pif_act_irq_disable = hostIrqDisable;
	pif_act_irq_restore = hostIrqRestore;

	if (!hostInit(portTick1ms)) return 1;

	if (pif_Init(hostMicros, portSetup, appSetup)) {
		while (hostIsRunning()) {
			pifTaskManager_Loop();
			hostYield();
		}
		pif_Exit();
	}

	hostExit();
	return pif_error;
}
