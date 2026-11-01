/**
 * 500ms마다 LED를 깜박이며 log를 log buffer에 저장한다. 10초 뒤에 UART를 붙이면 log buffer에 남은 log가
 * 출력되고 그 뒤로는 바로 출력된다.
 *
 * Flashes the LED every 500ms and keeps the log in the log buffer. The UART is attached after 10 seconds: it gets
 * what the log buffer still holds, and every line after it as it comes.
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
