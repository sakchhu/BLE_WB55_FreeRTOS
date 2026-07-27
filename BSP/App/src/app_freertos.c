#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    printf("A Stack Overflow has occured in %s", pcTaskName);
}
