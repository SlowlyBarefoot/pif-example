/**
 * 1초마다 log를 출력한다.
 *
 * Output log every second.
 */
#include <fcntl.h>
#include <signal.h>
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
#define PULSE_ITEM_COUNT    	6
#define TASK_COUNT              4


static int s_fd;

static PIF_stPulse *s_pstTimer = NULL;
static PIF_stComm *s_pstSerial = NULL;
static PIF_stProtocol *s_pstProtocol = NULL;
static PIF_stPulseItem *s_pstDelay[2] = { NULL, NULL };

static void _fnProtocolFinish30(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolFinish31(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolFinish20(PIF_stProtocolPacket *pstPacket);
static void _fnProtocolFinish21(PIF_stProtocolPacket *pstPacket);

const PIF_stProtocolResponse stProtocolResponseTable[] = {
		{ 0x30, PF_enType_Response | PF_enLogPrint_Yes, _fnProtocolFinish30 },
		{ 0x31, PF_enType_Response | PF_enLogPrint_Yes, _fnProtocolFinish31 },
		{ 0, PF_enDefault, NULL }
};

const PIF_stProtocolRequest stProtocolRequestTable[] = {
		{ 0x20, PF_enType_Request | PF_enResponse_Yes, _fnProtocolFinish20, 3, 300 },
		{ 0x21, PF_enType_Request | PF_enResponse_Yes, _fnProtocolFinish21, 3, 300 },
		{ 0, PF_enDefault, NULL, 0, 0 }
};


static void _fnProtocolFinish30(PIF_stProtocolPacket *pstPacket)
{
	if (!pifProtocol_MakeResponse(s_pstProtocol, pstPacket->ucCommand, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "eventFinish30: PID=%d Error=%d", pstPacket->ucPacketId, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "eventFinish30: PID=%d DC=%u", pstPacket->ucPacketId, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
			pifLog_Printf(LT_enNone, "\n");
		}
		pifPulse_StartItem(s_pstDelay[0], 500);
	}
}

static void _fnProtocolFinish31(PIF_stProtocolPacket *pstPacket)
{
	if (!pifProtocol_MakeResponse(s_pstProtocol, pstPacket->ucCommand, NULL, 0)) {
		pifLog_Printf(LT_enInfo, "eventFinish31: PID=%d Error=%d", pstPacket->ucPacketId, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "eventFinish31: PID=%d DC=%u", pstPacket->ucPacketId, pstPacket->usDataCount);
		if (pstPacket->usDataCount) {
			pifLog_Printf(LT_enNone, "\nData:");
			for (int i = 0; i < pstPacket->usDataCount; i++) {
				pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
			}
			pifLog_Printf(LT_enNone, "\n");
		}
		pifPulse_StartItem(s_pstDelay[1], 500);
	}
}

static void _fnProtocolFinish20(PIF_stProtocolPacket *pstPacket)
{
	pifLog_Printf(LT_enInfo, "eventFinish20: PID=%d DC=%u", pstPacket->ucPacketId, pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
		pifLog_Printf(LT_enNone, "\n");
	}
}

static void _fnProtocolFinish21(PIF_stProtocolPacket *pstPacket)
{
	pifLog_Printf(LT_enInfo, "eventFinish21: PID=%d DC=%u", pstPacket->ucPacketId, pstPacket->usDataCount);
	if (pstPacket->usDataCount) {
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < pstPacket->usDataCount; i++) {
			pifLog_Printf(LT_enNone, " %u", pstPacket->pucData[i]);
		}
		pifLog_Printf(LT_enNone, "\n");
	}
}

static void _actLogPrint(char *cString)
{
	printf("%s", cString);
}

static void _evtProtocolError(PIF_unDeviceCode unDeviceCode)
{
	pifLog_Printf(LT_enError, "eventProtocolError DC=%d", unDeviceCode);
}

static void _evtDelay(void *pvIssuer)
{
	uint8_t ucData[8];

	if (!pvIssuer) {
		pif_enError = E_enInvalidParam;
		return;
	}

	PIF_stProtocolRequest *pstOwner = (PIF_stProtocolRequest *)pvIssuer;

	for (int i = 0; i < 8; i++) ucData[i] = rand() & 0xFF;
	if (!pifProtocol_MakeRequest(s_pstProtocol, pstOwner->ucCommand, ucData, 8)) {
		pifLog_Printf(LT_enError, "Delay: DC=%d E=%d", s_pstProtocol->unDeviceCode, pif_enError);
	}
	else {
		pifLog_Printf(LT_enInfo, "Delay: DC=%d", s_pstProtocol->unDeviceCode);
		pifLog_Printf(LT_enNone, "\nData:");
		for (int i = 0; i < 8; i++) {
			pifLog_Printf(LT_enNone, " %u", ucData[i]);
		}
	}
}

static void _taskProtocolTest(PIF_stTask *pstTask)
{
	uint8_t txData;
	int rxData;

	(void)pstTask;

    if (pifComm_SendData(s_pstSerial, &txData)) {
        write(s_fd, &txData, 1);
    }

    if (read(s_fd, &rxData, 1)) {
    	pifComm_ReceiveData(s_pstSerial, rxData);
    }
}

static void _TimerHandler()
{
    pif_sigTimer1ms();

	pifPulse_sigTick(s_pstTimer);
}

int main(int argc, char **argv)
{
	int i;
    struct termios newtio;
    const char *port[3] = { "/dev/ttyACM1", "/dev/ttyACM2", "/dev/ttyACM3" };
	PIF_unDeviceCode unDeviceCode = 1;

    if (start_timer(1, &_TimerHandler)) {     // 1ms
        printf("\nstart_timer error\n");
        return(1);
    }

    for (i = 0; i < 3; i++) {
		s_fd = open( port[i], O_RDWR | O_NOCTTY );
		if (s_fd >= 0) break;
    }
	if (s_fd < 0) {
		fprintf(stderr, "All port open failed.\n");
		exit(-1);
	}
	else {
		printf("%s port opened.\n", port[i]);
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
    s_pstTimer = pifPulse_Add(unDeviceCode++, PULSE_ITEM_COUNT);
    if (!s_pstTimer) goto fail;

    s_pstSerial = pifComm_Add(unDeviceCode++);
	if (!s_pstSerial) goto fail;

    if (!pifProtocol_Init(s_pstTimer, PROTOCOL_COUNT)) goto fail;
    s_pstProtocol = pifProtocol_Add(unDeviceCode++, PT_enMedium, stProtocolRequestTable, stProtocolResponseTable);
    if (!s_pstProtocol) goto fail;
    pifProtocol_AttachComm(s_pstProtocol, s_pstSerial);
    s_pstProtocol->evtError = _evtProtocolError;

    for (int i = 0; i < 2; i++) {
		s_pstDelay[i] = pifPulse_AddItem(s_pstTimer, PT_enOnce);
		if (!s_pstDelay[i]) goto fail;
		pifPulse_AttachEvtFinish(s_pstDelay[i], _evtDelay, &stProtocolRequestTable[i]);
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
