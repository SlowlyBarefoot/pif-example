/**
 * Pulse 1개는 1ms Timer에 연결하고 또 1개는 100us Timer에 연결한다.
 * 2 Timer 모두 500ms 마다 이벤트를 발생시키고 이벤트에서 LED를 깜박가리게 한다.
 * 여기서 Scale 사용법을 알 수 있다.
 *
 * One pulse is connected to the 1 ms timer, and the other is connected to the 100us timer.
 * 2 Both Timers generate an event every 500 ms and flash the LED at the event.
 * Here you can see how to use the scale.
 */

// Do not remove the include below
#include "exTimer2.h"
#include "app_port.h"

//The setup function is called once at startup of the sketch
void setup()
{
	pif_Init((PifActTimer1us)micros, portSetup, NULL);
}

// The loop function is called in an endless loop
void loop()
{
	pifTaskManager_Loop();
}
