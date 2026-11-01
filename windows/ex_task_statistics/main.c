/**
 * Task의 통계를 1초마다 수집해 CSV로 출력하고, "vcd" command로 VCD 파일을 보낸다.
 * PC는 실시간 OS가 아니므로 realtime task의 지연과 miss가 board보다 크게 나타난다.
 *
 * Collects the statistics of the tasks every second, prints them as CSV, and sends them as a VCD file on the
 * "vcd" command. A PC is not a realtime OS, so the delays and misses of the realtime task are larger than on a
 * board.
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
