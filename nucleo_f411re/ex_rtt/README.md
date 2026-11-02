# ex_rtt (Nucleo-F411RE)

pif의 `pifRtt` 예제. SEGGER RTT 채널 0으로 pifLog의 로그와 명령줄을 쓴다.
명령: `help`, `version`, `task`, `blink t|f|<ms>`, `rtt`(채널, TX 대기 바이트).

RTT는 디버그 프로브가 SWD로 타깃 RAM의 버퍼를 읽고 쓰므로, UART 핀도 USB 배선도 필요 없다. ST-LINK USB만 연결한다.
USB CDC 예제는 `ex_usb_cdc`에 따로 있다.

## 1. 프로젝트

STM32CubeIDE 프로젝트다. `ex_terminal`과 같은 CubeMX 설정(클럭, TIM2, USART2)을 쓴다. USART2는 이 예제에서 쓰지 않는다.

- **pif**: `pif/include`가 include path에, `pif/source`의 필요한 파일이 `Core/pif` 링크 폴더에 들어 있다.
  `pif_rtt.c`가 들어 있는 pif 버전이어야 한다.
- **SEGGER RTT**: `Core/Inc/SEGGER_RTT.h`, `Core/Inc/SEGGER_RTT_Conf.h`, `Core/Src/SEGGER_RTT.c`(SEGGER RTT 소스 그대로).
  `pif_conf.h`에는 `PIF_USE_SEGGER_RTT`가 켜져 있다.

ex_terminal과 달리 타이머가 없으므로 `SysTick_Handler`에는 `pif_sigTimer1ms()`만 있다.

## 2. 송수신 방식

pifRtt는 `PifUart`이므로 pifLog가 일반 UART처럼 붙는다. TX 태스크(1 ms)는 RTT up 버퍼에 들어갈 만큼만 복사하고
나머지는 다음 실행에 보낸다. 프로브가 읽지 않으면 TX 버퍼에 쌓이고, 넘치는 로그는 pifLog 쪽에서 버려진다.
명령줄 입력은 RX 태스크(20 ms)가 down 버퍼에서 읽는다.

pyOCD나 J-Link RTT Viewer 같은 RTT 뷰어는 입력한 글자를 직접 화면에 표시하므로, `appSetup()`에서
`pifLog_SetEcho(FALSE)`로 입력 글자의 에코를 끈다. 줄바꿈, 지우기, 자동완성은 그대로 돌려보낸다.
직접 표시하지 않는 터미널(예: OpenOCD RTT 서버에 PuTTY Raw, Local echo Force off)을 쓰면 입력이 보이지 않으니
이 줄을 지운다.

채널 0의 버퍼 크기는 `SEGGER_RTT_Conf.h`의 `BUFFER_SIZE_UP`(기본 1024)과 `BUFFER_SIZE_DOWN`(기본 16)이다.

## 3. 실행

ST-LINK로 다음 중 하나를 쓴다.

- **OpenOCD**: `openocd -f board/st_nucleo_f4.cfg` 후 telnet 4444에서
  `rtt setup 0x20000000 0x20000 "SEGGER RTT"`, `rtt start`, `rtt server start 9090 0`, 그리고 `telnet localhost 9090`
- **pyOCD**: `pyocd rtt -t stm32f411re`
- **J-Link RTT Viewer**: ST-LINK를 J-Link OB로 바꾼 경우(SEGGER의 ST-LINK Reflash Utility)

STM32CubeIDE에서 디버깅하는 중에도 OpenOCD 디버그 구성이면 같은 방법으로 RTT를 볼 수 있다.
