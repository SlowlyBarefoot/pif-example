#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include <stdio.h>

#include "host.h"


#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION	0x00000002
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING		0x0004
#endif

#define IDLE_SLEEP_US			100


static HostEvtTick1ms s_evt_tick1ms;
static HANDLE s_tick_thread = NULL;
static HANDLE s_tick_timer = NULL;
static HANDLE s_idle_timer = NULL;
static BOOL s_period_set = FALSE;
static CRITICAL_SECTION s_irq_lock;
static volatile LONG s_running = 0;
static volatile LONG s_idle = 0;
static LARGE_INTEGER s_frequency;
static LARGE_INTEGER s_start;
static HANDLE s_stdin;
static BOOL s_stdin_console = FALSE;
static DWORD s_stdin_type;
static HANDLE s_stdout;
static DWORD s_stdout_mode;
static BOOL s_stdout_mode_saved = FALSE;


static uint64_t _elapsedUs()
{
	LARGE_INTEGER now;
	uint64_t count;

	QueryPerformanceCounter(&now);
	count = (uint64_t)(now.QuadPart - s_start.QuadPart);
	// Split so that the product does not overflow after a few days.
	return count / s_frequency.QuadPart * 1000000ULL + count % s_frequency.QuadPart * 1000000ULL / s_frequency.QuadPart;
}

// A high resolution waitable timer sleeps for well under a millisecond on Windows 10 1803 and later. Without one,
// Sleep() rounds up to the period set with timeBeginPeriod().
static HANDLE _createTimer()
{
	return CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
}

static void _sleepUs(HANDLE timer, uint32_t us)
{
	LARGE_INTEGER due;

	if (timer) {
		due.QuadPart = -(LONGLONG)us * 10;		// Relative, in 100ns
		if (SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE)) {
			WaitForSingleObject(timer, INFINITE);
			return;
		}
	}
	Sleep((us + 999) / 1000);
}

static BOOL WINAPI _evtConsoleCtrl(DWORD type)
{
	InterlockedExchange(&s_running, 0);
	// Windows ends the process once this returns from the close of the console. The wait gives the main loop the
	// time to exit on its own.
	if (type == CTRL_CLOSE_EVENT) Sleep(1000);
	return TRUE;
}

// The tick n is due n ms after the start. A late wake-up runs every tick it missed, so the time base keeps up
// with the clock even when the thread was held back.
static DWORD WINAPI _tickThread(LPVOID p_arg)
{
	uint64_t ticks = 0, elapsed;

	(void)p_arg;

	while (s_running) {
		elapsed = _elapsedUs();
		if (elapsed < (ticks + 1) * 1000ULL) _sleepUs(s_tick_timer, (uint32_t)((ticks + 1) * 1000ULL - elapsed));

		while (s_running && ticks < _elapsedUs() / 1000ULL) {
			EnterCriticalSection(&s_irq_lock);
			(*s_evt_tick1ms)();
			LeaveCriticalSection(&s_irq_lock);
			ticks++;
		}
	}
	return 0;
}

// _getch() already hands the keys over as they are typed, without an echo: the log echoes them itself, as it does
// on the UART of a board. The escape sequences of the log need the virtual terminal of the console.
static void _rawConsole()
{
	DWORD mode;

	s_stdin = GetStdHandle(STD_INPUT_HANDLE);
	s_stdin_console = GetConsoleMode(s_stdin, &mode);
	s_stdin_type = GetFileType(s_stdin);

	s_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleMode(s_stdout, &s_stdout_mode)) {
		s_stdout_mode_saved = TRUE;
		SetConsoleMode(s_stdout, s_stdout_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
}

static void _restoreConsole()
{
	if (s_stdout_mode_saved) {
		SetConsoleMode(s_stdout, s_stdout_mode);
		s_stdout_mode_saved = FALSE;
	}
}

int hostInit(HostEvtTick1ms evt_tick1ms)
{
	if (!evt_tick1ms) return 0;
	s_evt_tick1ms = evt_tick1ms;

	// pif_IrqDisable() may be called again inside a section it began, which a critical section allows.
	InitializeCriticalSection(&s_irq_lock);

	s_tick_timer = _createTimer();
	s_idle_timer = _createTimer();
	if (!s_tick_timer || !s_idle_timer) s_period_set = timeBeginPeriod(1) == TIMERR_NOERROR;

	SetConsoleCtrlHandler(_evtConsoleCtrl, TRUE);
	_rawConsole();

	QueryPerformanceFrequency(&s_frequency);
	QueryPerformanceCounter(&s_start);
	s_running = 1;
	s_tick_thread = CreateThread(NULL, 0, _tickThread, NULL, 0, NULL);
	if (!s_tick_thread) {
		hostExit();
		return 0;
	}
	SetThreadPriority(s_tick_thread, THREAD_PRIORITY_TIME_CRITICAL);
	return 1;
}

void hostExit()
{
	InterlockedExchange(&s_running, 0);
	if (s_tick_thread) {
		WaitForSingleObject(s_tick_thread, INFINITE);
		CloseHandle(s_tick_thread);
		s_tick_thread = NULL;
	}
	if (s_tick_timer) {
		CloseHandle(s_tick_timer);
		s_tick_timer = NULL;
	}
	if (s_idle_timer) {
		CloseHandle(s_idle_timer);
		s_idle_timer = NULL;
	}
	if (s_period_set) {
		timeEndPeriod(1);
		s_period_set = FALSE;
	}
	_restoreConsole();
	printf("\n");
}

uint32_t hostMicros()
{
	return (uint32_t)_elapsedUs();
}

uint32_t hostIrqDisable()
{
	EnterCriticalSection(&s_irq_lock);
	return 0;
}

void hostIrqRestore(uint32_t state)
{
	(void)state;

	LeaveCriticalSection(&s_irq_lock);
}

int hostIsRunning()
{
	return s_running;
}

void hostEvtIdle()
{
	s_idle = 1;
}

void hostYield()
{
	if (!s_idle) return;
	s_idle = 0;
	_sleepUs(s_idle_timer, IDLE_SLEEP_US);
}

uint16_t hostConsoleWrite(const uint8_t *p_data, uint16_t size)
{
	size_t written;

	written = fwrite(p_data, 1, size, stdout);
	fflush(stdout);
	return (uint16_t)written;
}

// A console is read with _getch(), a pipe only as far as PeekNamedPipe() says it has data, so that neither blocks
// the main loop. A file is always ready. Any other device, such as NUL, gives nothing: _kbhit() on one that is
// not a console takes milliseconds a call.
uint16_t hostConsoleRead(uint8_t *p_data, uint16_t size)
{
	DWORD available, length = 0;
	uint16_t i;

	if (s_stdin_console) {
		for (i = 0; i < size && _kbhit(); i++) p_data[i] = (uint8_t)_getch();
		return i;
	}

	switch (s_stdin_type) {
	case FILE_TYPE_PIPE:
		if (!PeekNamedPipe(s_stdin, NULL, 0, NULL, &available, NULL) || !available) return 0;
		if (available < size) size = (uint16_t)available;
		break;

	case FILE_TYPE_DISK:
		break;

	default:
		return 0;
	}
	if (!ReadFile(s_stdin, p_data, size, &length, NULL)) return 0;
	return (uint16_t)length;
}

void hostLed(int state)
{
	printf("\rLED [%c]", state ? '#' : ' ');
	fflush(stdout);
}
