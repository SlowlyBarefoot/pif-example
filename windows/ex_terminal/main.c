/**
 * Console에서 log의 command를 받는다. "help"로 command 목록을 볼 수 있고 "blink t"로 LED를 깜박이게 한다.
 *
 * Takes the commands of the log from the console. "help" lists them and "blink t" makes the LED blink.
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
