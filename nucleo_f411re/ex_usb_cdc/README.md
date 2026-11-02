# ex_usb_cdc (Nucleo-F411RE)

pif의 `pifUsbCdc` 예제. USB CDC(가상 COM 포트)로 pifLog의 로그와 명령줄을 쓴다.
명령: `help`, `version`, `task`, `blink t|f|<ms>`, `cdc`(포트 열림, TX 대기 바이트, 닫혀 있는 동안 버린 바이트).

SEGGER RTT 예제는 `ex_rtt`에 따로 있다.

## 1. 하드웨어

Nucleo-F411RE에는 USB 사용자 커넥터가 없다. USB 커넥터(브레이크아웃)를 morpho 헤더에 연결한다.

| USB | F411RE | Morpho |
| --- | --- | --- |
| D- | PA11 | CN10-14 |
| D+ | PA12 | CN10-12 |
| GND | GND | CN10-9 등 |
| VBUS | 연결하지 않음 (보드는 ST-LINK USB로 전원) | |

## 2. 프로젝트

STM32CubeIDE 프로젝트다. `ex_terminal`의 CubeMX 설정에서 다음을 바꿨다.

- **RCC**: HSE = BYPASS(ST-LINK의 MCO 8 MHz). USB는 48 MHz ±0.25%가 필요해서 HSI(±1%)로는 안 된다.
- **Clock**: PLL Source = HSE, `/M = 8`, `*N = 336`, `/P = 4` → SYSCLK 84 MHz, `/Q = 7` → 48 MHz.
  APB1 `/2`, APB2 `/1`, TIM2 Prescaler `84-1`(1 µs)은 그대로다.
- **USB_OTG_FS**: Device_Only, VBUS sensing 끔.
- **USB_DEVICE**: Communication Device Class (Virtual Port Com). `USB_DEVICE/`, `Middlewares/ST/STM32_USB_Device_Library/`.

USART2는 이 예제에서 쓰지 않는다. pif는 다른 예제처럼 `pif/include`가 include path에, `pif/source`의 필요한 파일이
`Core/pif` 링크 폴더에 들어 있다. `pif_usb_cdc.c`가 들어 있는 pif 버전이어야 한다.

## 3. USER CODE

CubeMX로 다시 생성해도 남도록 pif 연결은 USER CODE 안에 있다.

- `Core/Src/main.c`: `pif_Init()`, `pifTaskManager_Loop()`. `MX_USB_DEVICE_Init()`은 그 앞에서 불린다.
- `Core/Src/stm32f4xx_it.c`: `SysTick_Handler`의 `pif_sigTimer1ms()`.
- `USB_DEVICE/App/usbd_cdc_if.c`
  - `CDC_Control_FS()`의 `CDC_SET_CONTROL_LINE_STATE`: `portCdcLineState()`로 DTR(터미널이 포트를 열었는지)을 알린다.
    데이터가 없는 요청은 `pbuf`가 SETUP 요청 자체다.
  - `CDC_Receive_FS()`: 받은 패킷을 `portCdcReceive()`로 RX 버퍼에 넣고, 다음 패킷을 받을 자리가 있을 때만 다시 받는다.
    자리가 없으면 호스트가 기다리고(NAK), `port_main.c`의 태스크가 자리가 생기면 `CDC_ArmReceive()`를 부른다.
  - `CDC_TransmitCplt_FS()`: `portCdcTxComplete()`로 다음 패킷을 바로 보낸다.
  - `CDC_ArmReceive()`: 태스크에서도 불리므로 USB 인터럽트를 잠시 막고 다시 받는다.

## 4. 송신 방식

`CDC_Transmit_FS()`는 넘겨받은 포인터에서 나중에 보낸다. 그래서 `port_main.c`의 `act_get_room`은 클래스가
전송 중(`TxState`)이면 0을, 아니면 64를 돌려준다. pifUsbCdc는 패킷을 자기 `_packet`에 복사해 넘기고,
`act_get_room`이 0인 동안은 그 버퍼를 다시 쓰지 않는다. 터미널이 포트를 열지 않았으면(DTR 꺼짐) 로그는 버려지고
`cdc` 명령의 `dropped`에 센다. 열려 있을 때까지 모아 두려면 `g_cdc.hold_while_closed = TRUE`로 한다.

## 5. 실행

PC에서 새 COM 포트(Windows 10 이상은 드라이버 불필요)를 터미널로 연다. 속도 설정은 의미 없다. `help`를 입력한다.
