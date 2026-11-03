/**
 * pifMotionProfile로 가상의 스텝 모터를 움직인다. 1ms마다 profile을 sample하고 그 위치까지 STEP 펄스를 낸다.
 * 이동 중에 목표를 바꾸는 경우(역방향, 연장)와 정지도 보여 주며, 진행 상황은 console에 log로 찍는다.
 *
 * Moves a virtual step motor with pifMotionProfile. Every millisecond it samples the profile and puts out STEP
 * pulses up to that position. It also shows a change of the target during a move (back and further on) and a
 * stop, and logs the progress on the console.
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
