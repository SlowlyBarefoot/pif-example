/**
 * Xmodem으로 random data 1000 bytes를 serial port에 보낸다. 받는 쪽이 보내는 'C'로 전송을 시작한다.
 * Serial port는 /dev/ttyACM1~9, /dev/ttyUSB0~9 중 처음 열리는 것을 쓴다.
 *
 * Sends 1000 bytes of random data to a serial port by Xmodem. The transfer starts with the 'C' of the receiver.
 * The serial port is the first of /dev/ttyACM1~9 and /dev/ttyUSB0~9 that opens.
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
