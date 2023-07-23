#include "app_main.h"


PifI2cPort g_i2c_port;
PifTimerManager g_timer_1ms;


BOOL appSetup()
{
    pifI2cPort_ScanAddress(&g_i2c_port);
    return TRUE;
}
