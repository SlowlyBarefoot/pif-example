/**
 * Pulse 1개를 생성후 이를 1ms Timer에 연결한다.
 * 이 Pulse로 200ms마다 적색 LED를 10번 깜박이고 500ms마다 노란색 LED를 10회 깜박이고
 * 1000ms마다 녹색 LED를 10회 깜박이는 이 과정을 반복한다.
 * 여기서 Pulse의 시작과 종료 함수 사용법과 이벤트 함수의 매개변수 사용법을 알 수 있다.
 *
 * Create one pulse and connect it to the 1 ms timer.
 * Repeat this process of flashing red LED 10 times every 200 ms, flashing yellow LED 10 times every 500 ms and
 * green LED 10 times every 1000 ms with this pulse.
 * Here you can see how to use the start and end functions of Pulse and how to use parameters of the event function.
 */
// Do not remove the include below
#include "exTimer4.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
    pif_Init((PifActTimer1us)micros, portSetup, appSetup);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
