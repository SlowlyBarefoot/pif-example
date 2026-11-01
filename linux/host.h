#ifndef HOST_H
#define HOST_H


#include <stdint.h>


/**
 * Linux stands in for the board of the examples here.
 *
 * A thread calls the 1 ms tick as the SysTick interrupt of a board would, and holds hostIrqDisable() while it
 * runs, so that pif_IrqDisable() keeps it out as it keeps an interrupt out. The console stands in for the UART
 * of the log. The header does not include pif, so that it keeps clear of the types of the platform.
 */


typedef void (*HostEvtTick1ms)(void);


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts the tick thread and puts the console in raw mode. Ctrl+C ends hostIsRunning().
 * @param evt_tick1ms Called every millisecond from the tick thread, as the body of a SysTick handler.
 * @return 1 on success, 0 otherwise.
 */
int hostInit(HostEvtTick1ms evt_tick1ms);

/**
 * @brief Stops the tick thread and gives the console its mode back.
 */
void hostExit();

/**
 * @brief Free-running 1 us counter, for pif_Init().
 */
uint32_t hostMicros();

/**
 * @brief Interrupt mask of pif, for pif_act_irq_disable and pif_act_irq_restore.
 */
uint32_t hostIrqDisable();
void hostIrqRestore(uint32_t state);

/**
 * @brief 0 once Ctrl+C or SIGTERM has come.
 */
int hostIsRunning();

/**
 * @brief Idle callback for pifTaskManager_SetIdle(). It only marks the pass as idle, so that hostYield() gives the
 *        CPU up outside the task manager and the sleep does not count as load.
 */
void hostEvtIdle();

/**
 * @brief Call after every pifTaskManager_Loop(). It sleeps a little after a pass that ran nothing.
 */
void hostYield();

/**
 * @brief Writes to stdout. For PifUart::act_send_data.
 */
uint16_t hostConsoleWrite(const uint8_t *p_data, uint16_t size);

/**
 * @brief Reads what has come in on stdin without waiting. For PifUart::act_receive_data.
 */
uint16_t hostConsoleRead(uint8_t *p_data, uint16_t size);

/**
 * @brief Draws the LED of the board on the current line of the console.
 */
void hostLed(int state);

#ifdef __cplusplus
}
#endif


#endif	// HOST_H
