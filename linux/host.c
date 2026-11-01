#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "host.h"


#define IDLE_SLEEP_NS			100000L		// 100us


static HostEvtTick1ms s_evt_tick1ms;
static pthread_t s_tick_thread;
static int s_tick_started = 0;
static pthread_mutex_t s_irq_mutex;
static volatile sig_atomic_t s_running = 0;
static volatile int s_idle = 0;
static struct timespec s_start;
static int s_termios_saved = 0;
static struct termios s_termios;


static uint64_t _elapsedNs()
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (uint64_t)(now.tv_sec - s_start.tv_sec) * 1000000000ULL + now.tv_nsec - s_start.tv_nsec;
}

static void _evtSignal(int signum)
{
	(void)signum;

	s_running = 0;
}

// The tick n is due n ms after the start. A late wake-up runs every tick it missed, so the time base keeps up
// with the clock even when the thread was held back.
static void *_tickThread(void *p_arg)
{
	struct timespec next;
	uint64_t ticks = 0, due_ns;

	(void)p_arg;

	while (s_running) {
		due_ns = (ticks + 1) * 1000000ULL;
		next.tv_sec = s_start.tv_sec + (time_t)(due_ns / 1000000000ULL);
		next.tv_nsec = s_start.tv_nsec + (long)(due_ns % 1000000000ULL);
		if (next.tv_nsec >= 1000000000L) {
			next.tv_sec++;
			next.tv_nsec -= 1000000000L;
		}
		while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL) == EINTR && s_running);

		while (s_running && ticks < _elapsedNs() / 1000000ULL) {
			pthread_mutex_lock(&s_irq_mutex);
			(*s_evt_tick1ms)();
			pthread_mutex_unlock(&s_irq_mutex);
			ticks++;
		}
	}
	return NULL;
}

static void _restoreConsole()
{
	if (s_termios_saved) {
		tcsetattr(STDIN_FILENO, TCSANOW, &s_termios);
		s_termios_saved = 0;
	}
}

// Keys go to the log as they are typed, without the echo of the terminal: the log echoes them itself, as it
// does on the UART of a board. Ctrl+C still raises SIGINT.
static void _rawConsole()
{
	struct termios raw;

	if (!isatty(STDIN_FILENO) || tcgetattr(STDIN_FILENO, &s_termios)) return;
	s_termios_saved = 1;
	atexit(_restoreConsole);

	raw = s_termios;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int hostInit(HostEvtTick1ms evt_tick1ms)
{
	pthread_mutexattr_t attr;
	struct sigaction action;

	if (!evt_tick1ms) return 0;
	s_evt_tick1ms = evt_tick1ms;

	// pif_IrqDisable() may be called again inside a section it began.
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&s_irq_mutex, &attr);
	pthread_mutexattr_destroy(&attr);

	action.sa_handler = _evtSignal;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);

	_rawConsole();

	clock_gettime(CLOCK_MONOTONIC, &s_start);
	s_running = 1;
	if (pthread_create(&s_tick_thread, NULL, _tickThread, NULL)) {
		s_running = 0;
		_restoreConsole();
		return 0;
	}
	s_tick_started = 1;
	return 1;
}

void hostExit()
{
	s_running = 0;
	if (s_tick_started) {
		pthread_join(s_tick_thread, NULL);
		s_tick_started = 0;
	}
	_restoreConsole();
	printf("\n");
}

uint32_t hostMicros()
{
	return (uint32_t)(_elapsedNs() / 1000ULL);
}

uint32_t hostIrqDisable()
{
	pthread_mutex_lock(&s_irq_mutex);
	return 0;
}

void hostIrqRestore(uint32_t state)
{
	(void)state;

	pthread_mutex_unlock(&s_irq_mutex);
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
	struct timespec delay = { 0, IDLE_SLEEP_NS };

	if (!s_idle) return;
	s_idle = 0;
	nanosleep(&delay, NULL);
}

uint16_t hostConsoleWrite(const uint8_t *p_data, uint16_t size)
{
	size_t written;

	written = fwrite(p_data, 1, size, stdout);
	fflush(stdout);
	return (uint16_t)written;
}

// poll() keeps a pipe from blocking the main loop. stdin is not made O_NONBLOCK, which would also make the writes
// to a terminal that shares its file description fail with EAGAIN.
uint16_t hostConsoleRead(uint8_t *p_data, uint16_t size)
{
	struct pollfd fds = { STDIN_FILENO, POLLIN, 0 };
	ssize_t length;

	if (poll(&fds, 1, 0) <= 0 || !(fds.revents & POLLIN)) return 0;
	length = read(STDIN_FILENO, p_data, size);
	return length > 0 ? (uint16_t)length : 0;
}

void hostLed(int state)
{
	printf("\rLED [%c]", state ? '#' : ' ');
	fflush(stdout);
}
