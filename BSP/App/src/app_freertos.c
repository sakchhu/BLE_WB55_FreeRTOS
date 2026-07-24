#include "FreeRTOS.h"
#include "cmsis_gcc.h"
#include "stm32_lpm.h"
#include "task.h"

#include <stdio.h>

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    printf("A Stack Overflow has occured in %s", pcTaskName);
}

// void vApplicationIdleHook( void )
// {
//     // UTIL_LPM_EnterLowPower();
//     __WFI();
// }
