#include "linker.h"

#include "interpreter/pif_basic.h"


PifLed g_led_l;
PifTimerManager g_timer_1ms;

static int _cmdBasicExecute(int argc, char* argv[]);

const PifLogCmdEntry c_psCmdTable[] = {
	{ "help", pifLog_CmdHelp, "This command", NULL },
	{ "version", pifLog_CmdPrintVersion, "Print version", NULL },
	{ "task", pifLog_CmdPrintTask, "Print task", NULL },
	{ "status", pifLog_CmdSetStatus, "Set and print status", NULL },
	{ "exec", _cmdBasicExecute, "Run Basic Program", NULL },

	{ NULL, NULL, NULL, NULL }
};

const char c_basic1[] = "\
SUB SUBR X,Y,Z\n\
	LOCAL	N\n\
	N=	10\n\
	FORMAT	\"SUBR: %,%,%\", X,Y,Z\n\
	FORMAT	\"SUBR: OK!\"\n\
	RETURN\n\
	FORMAT	\"S: RETURN DID NOT RETURN!\"\n\
END SUB\n\
\n\
SUB F	X,Y\n\
	RETURN	X+Y\n\
	FORMAT	\"F: RETURN DID NOT RETURN!\"\n\
END SUB\n\
\n\
N=	400\n\
X=	100\n\
Y=	200\n\
Z=	300\n\
SUBR	1,2,3\n\
FORMAT	\"X,Y,Z,N: %,%,%,%\", X,Y,Z,N\n\
FORMAT	\"F(100,200): %\", F(100,F(120,80))\n\
\n\
I=	0\n\
WHILE	I<5\n\
	FORMAT	\"WHILE I:%\",I\n\
	I=I+1\n\
END WHILE\n\
\n\
SUB TESTLOOP\n\
	LOCAL I\n\
	FOR I=0 TO 3\n\
	END FOR\n\
END SUB\n\
FOR	I=0 TO 5\n\
	TESTLOOP\n\
END FOR\n\
\n\
FOR	I=0 TO 3\n\
FOR	J=10 TO 12\n\
	FORMAT	\"% %\", I,J\n\
END FOR\n\
END FOR\n\
\n\
SUB	TESTESCAPE\n\
	FOR	I=0 TO 10\n\
		RETURN\n\
	END FOR\n\
END SUB\n\
TESTESCAPE\n\
\n\
IF	0\n\
	FORMAT	\"NO\"\n\
ELSE IF 1<2\n\
	FORMAT	\"YES!\"\n\
ELSE\n\
	FORMAT \"WTF\"\n\
END IF\n\
\n\
IF	1 THEN FORMAT \"IN-LINE OK\"\n\
IF	0 THEN FORMAT \"IN-LINE BAD\"\n\
\n\
DIM	A(10)\n\
FOR	I=1 TO UBOUND(A)+1\n\
	A(I)=	I*10\n\
END FOR\n\
FORMAT	\"% % %\",A(1),A(10),UBOUND(A)\n\
\n\
#FORMAT	\"RESUMED BOUND: %\", A(0)\n\
#FORMAT \"RESUMED DIV: %\", 1/0\n\
#BREAK\n\
FORMAT	\"DONE.\"";

const char c_basic2[] = "\
FOR	I=1 TO 100\r\n\
	FORMAT	\"% % %\", I,I*I,I/5\r\n\
END FOR";

const char c_basic3[] = "\
FOR	I=1 TO 10\n\r\
	IF I>5\n\r\
		FORMAT	\"% : Big\",I\n\r\
	ELSE IF I = 5\n\r\
		FORMAT	\"% : Same\",I\n\r\
	ELSE\n\r\
		FORMAT	\"% : Little\",I\n\r\
	END IF\n\r\
END FOR";

const char c_basic4[] = "\
EXEC 1,2\r\
EXEC 3,4\r\
EXEC 2,3,4";

const char c_basic5[] = "\
N = EXEC 1,4\n\
FORMAT \"N=%\", N\n\
N = EXEC 2,8,9\n\
FORMAT \"N=%\", N";

const char* c_basics[] = {
		c_basic1,
		c_basic2,
		c_basic3,
		c_basic4,
		c_basic5,
};

static int _basicProcess1(int count, int* p_params);
static int _basicProcess2(int count, int* p_params);

static PifBasicProcess p_process[] = {
		_basicProcess1,
		_basicProcess2,
		NULL,
};


static int _basicProcess1(int count, int* p_params)
{
	pifLog_Printf(LT_INFO, "P1 = %d :", count);
	for (int i = 0; i < count; i++) {
		pifLog_Printf(LT_NONE, " %d", p_params[i]);
	}
	return p_params[0] + 1;
}

static int _basicProcess2(int count, int* p_params)
{
	pifLog_Printf(LT_INFO, "P2 = %d :", count);
	for (int i = 0; i < count; i++) {
		pifLog_Printf(LT_NONE, " %d", p_params[i]);
	}
	return p_params[0] + 2;
}

static int _cmdBasicExecute(int argc, char* argv[])
{
	int n;

	if (argc == 1) {
		n = atoi(argv[0]) - 1;
		if (n < sizeof(c_basics) / sizeof(c_basics[0])) {
			pifBasic_Execute((char*)c_basics[n], 32);		// 32 :  Count of opcodes processed at one time
		}
		else {
			return PIF_LOG_CMD_INVALID_ARG;
		}
		return PIF_LOG_CMD_NO_ERROR;
	}
	return PIF_LOG_CMD_TOO_FEW_ARGS;
}

static void _evtComplete(PifBasic* p_owner)
{
	char *sym[2] = { "<=", ">" };

	pifLog_Print(LT_INFO, "Complete:");
	pifLog_Printf(LT_NONE, "\n\tProgram Size = %d %s %d", p_owner->_program_size, sym[p_owner->_program_size > PIF_BASIC_PROGRAM], PIF_BASIC_PROGRAM);
	pifLog_Printf(LT_NONE, "\n\tVariable Count = %d %s %d", p_owner->_varable_count, sym[p_owner->_varable_count > PIF_BASIC_VARIABLE], PIF_BASIC_VARIABLE);
	pifLog_Printf(LT_NONE, "\n\tString Count = %d %s %d", p_owner->_string_count, sym[p_owner->_string_count > PIF_BASIC_STRING], PIF_BASIC_STRING);
	pifLog_Printf(LT_NONE, "\n\tStack Count = %d %s %d", p_owner->_stack_count, sym[p_owner->_stack_count > PIF_BASIC_STACK], PIF_BASIC_STACK);
	pifLog_Printf(LT_NONE, "\n\tParsing Time = %ldms", p_owner->_parsing_time);
	pifLog_Printf(LT_NONE, "\n\tProcess Time = %ldms\n", p_owner->_process_time);
}

BOOL appSetup()
{
    if (!pifLog_UseCommand(c_psCmdTable, "\nDebug> ")) return FALSE;

    if (!pifLed_AttachSBlink(&g_led_l, 500)) return FALSE;		// 500ms
    pifLed_SBlinkOn(&g_led_l, 1 << 0);

	pifBasic_Init(p_process, _evtComplete);
	return TRUE;
}
