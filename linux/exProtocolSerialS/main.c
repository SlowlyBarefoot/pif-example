/**
 * 1초마다 log를 출력한다.
 *
 * Output log every second.
 */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "timer.h"

#include "pifComm.h"
#include "pifLog.h"
#include "pifProtocol.h"


#define COMM_COUNT         		1
#define PROTOCOL_COUNT          1
#define PULSE_COUNT         	1
#define PULSE_ITEM_COUNT    	10
#define TASK_COUNT              4


static int s_fd;

static PIF_stPulse *s_pstTimer1ms = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket);

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolQuestion stProtocolQuestions[] = {
		{ 0x30, PF_enLogPrint_Yes, _fnProtocolQuestion30 },
		{ 0x31, PF_enLogPrint_Yes, _fnProtocolQuestion31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enResponse_Yes | PF_enLogPrint_Yes, _fnProtocolResponse20, 3, 300 },
		{ 0x21, PF_enResponse_Ack | PF_enLogPrint_Yes, _fnProtocolResponse21, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};

static struct {
	PIF_stPulseItem *pstDelay;
	uint8_t ucDataCount;
	uint8_t ucData[8];
} s_stProtocolTest[2] = {
		{ NULL, 0, },
		{ NULL, 0, }
};


static void _fnProtocolPrint(PIF_stProtocolPacket *pstPacket, const char *pcName)
{
	if (pstPacket) {
		pifLog_Printf(LT_enInfo, "%s: CNT=%u", pcName, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (uint16_t i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
		}
	}
	else {
		pifLog_Printf(LT_enInfo, pcName);
	}
}

static void _fnProtocolQuestion30(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question30");
	s_stProtocolTest[0].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[0].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	if (!pifProtocol_MakeAnswer(s_pstProtocol, pstPacket, stProtocolQuestions[0].enFlags, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "Question30: Error=%d", pif_enError);
	}
	else {
		pifPulse_StartItem(s_stProtocolTest[0].pstDelay, 500);
	}
}

static void _fnProtocolQuestion31(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Question31");
	s_stProtocolTest[1].ucDataCount = pstPacket->usDataCount;
	if (pstPacket->usDataCount) {
		memcpy(s_stProtocolTest[1].ucData, pstPacket->pucData, pstPacket->usDataCount);
	}

	pifPulse_StartItem(s_stProtocolTest[1].pstDelay, 500);
}

static void _fnProtocolResponse20(PIF_stProtocolPacket *pstPacket)
{
	_fnProtocolPrint(pstPacket, "Response20");
}

static void _fnProtocolResponse21(PIF_stProtocolPacket *pstPacket)
{
	(void)pstPacket;

	pifLog_Printf(LT_enInfo, "Response21: ACK");
}

static void _evtProtocolError(PIF_usId usPifId)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", usPifId);
}

static void _actLogPrint(char *pcString)
{
	printf("%s", pcString);
}

static void _evtDelay(void *pvIssuer)
{
	if (!pvIssuer) {
		pif_enError = E_enInvalidParam;
		return;
	}

	const PIF_stProtocolRequest *pstOwner = (PIF_stProtocolRequest *)pvIssuer;
	int index = pstOwner->ucCommand & 0x0F;

	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner, s_stProtocolTest[index].ucData, s_stProtocolTest[index].ucDataCount)) {
		pifLog_Printf(LT_enError, "Delay(%u): DC=%d E=%d", index, s_pstProtocol->_usPifId, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "Delay(%u): DC=%d CNT=%u", index, s_pstProtocol->_usPifId, s_stProtocolTest[index].ucDataCount);
		if (s_stProtocolTest[index].ucDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < s_stProtocolTest[index].ucDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", s_stProtocolTest[index].ucData[i]);
			}
		}
	}
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	uint8_t rxData[8];
	uint16_t size;

	(void)pstTask;

    while (pifComm_SendData(s_pstSerial, &txData)) {
        write(s_fd, &txData, 1);
    }

    size = pifComm_GetRemainSizeOfRxBuffer(s_pstSerial);
	if (size) {
		size = read(s_fd, rxData, size);
		if (size) {
			pifComm_ReceiveDatas(s_pstSerial, rxData, size);
		}
	}
}

static void _TimerHandler()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer1ms);
}

int main(int argc, char **argv)
{
	int i;
    struct termios newtio;
    char port[16];

    if (start_timer(1, &_TimerHandler)) {     // 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    for (i = 1; i < 10; i++) {
    	sprintf(port, "/dev/ttyACM%d", i);
		s_fd = open( port, O_RDWR | O_NOCTTY );
		if (s_fd >= 0) break;
    }
	if (s_fd < 0) {
		fprintf(stderr, "All port open failed.\n");
		exit(-1);
	}
	else {
		printf("%s port opened.\n", port);
	}

    memset( &newtio, 0, sizeof(newtio) );

    newtio.c_cflag = B115200;
    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;
    newtio.c_cflag |= CREAD;
    newtio.c_iflag = IGNPAR;
 //   newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(s_fd, TCIFLUSH);
    tcsetattr(s_fd, TCSANOW, &newtio);

    pif_Init();

    pifLog_Init();
    pifLog_AttachActPrint(_actLogPrint);

    if (!pifComm_Init(COMM_COUNT)) goto fail;

    if (!pifPulse_Init(PULSE_COUNT)) goto fail;
    s_pstTimer1ms = pifPulse_Add(PIF_ID_AUTO, PULSE_ITEM_COUNT, 1000);		// 1000us
    if (!s_pstTimer1ms) goto fail;

    s_pstSerial = pifComm_Add(PIF_ID_AUTO);
	if (!s_pstSerial) goto fail;

    if (!pifProtocol_Init(s_pstTimer1ms, PROTOCOL_COUNT)) goto fail;
    s_pstProtocol = pifProtocol_Add(PIF_ID_AUTO, PT_enSmall, stProtocolQuestions);
    if (!s_pstProtocol) goto fail;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    pifProtocol_AttachEvent(s_pstProtocol, _evtProtocolError);

    for (int i = 0; i < 2; i++) {
    	s_stProtocolTest[i].pstDelay = pifPulse_AddItem(s_pstTimer1ms, PT_enOnce);
		if (!s_stProtocolTest[i].pstDelay) goto fail;
		pifPulse_AttachEvtFinish(s_stProtocolTest[i].pstDelay, _evtDelay, (void *)&stProtocolRequestTable[i]);
    }

    if (!pifTask_Init(TASK_COUNT)) goto fail;
    if (!pifTask_AddRatio(100, pifPulse_taskAll, NULL)) goto fail;		// 100%
    if (!pifTask_AddRatio(3, pifComm_taskAll, NULL)) goto fail;			// 3%

    if (!pifTask_AddRatio(3, _taskProtocolTest, NULL)) goto fail;		// 3%

    while (1) {
        pif_Loop();

        pifTask_Loop();
    }

fail:
    pifLog_Exit();

    stop_timer();

	return pif_enError;
}
