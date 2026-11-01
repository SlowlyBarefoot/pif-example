/**
 * 500ms마다 LED를 깜박이고 그때마다 log를 출력한다. LED는 그 log가 대신한다.
 *
 * Flashes the LED every 500ms and prints a log each time. The log stands for the LED here.
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
