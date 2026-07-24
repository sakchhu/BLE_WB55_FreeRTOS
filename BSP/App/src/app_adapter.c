#include "stm32wb55xx.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "stm32wbxx_hal.h"
#include "task.h"

int __io_putchar ( int ch )
{
    ITM_SendChar(ch);
    return (ch);
}

void Error_Handler ( void )
{
    __disable_irq();
    while(1)
    {
    }
}

void assert_failed(uint8_t *file, uint32_t line)
{
    printf("%s:assert_failed %lu\n", (char*)file, line);
    Error_Handler();
}
