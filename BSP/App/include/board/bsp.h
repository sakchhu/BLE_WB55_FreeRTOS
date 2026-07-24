#ifndef RTS_RLS23L_BOARD_BOARD_H
#define RTS_RLS23L_BOARD_BOARD_H

#include "stm32wbxx_hal.h"

enum rts_rls23l_board_err_e
{
    RTS_RLS23L_BOARD_ERR_OK,
    RTS_RLS23L_BOARD_ERR_HAL,
    RTS_RLS23L_BOARD_ERR_SYSCLK,
    RTS_RLS23L_BOARD_ERR_PRPHCLK,
    RTS_RLS23L_BOARD_ERR_RTC,
    RTS_RLS23L_BOARD_ERR_IWDG,
    RTS_RLS23L_BOARD_ERR_GPIO,
    RTS_RLS23L_BOARD_ERR_IMU,
    RTS_RLS23L_BOARD_ERR_EXT_FLASH,
    RTS_RLS23L_BOARD_ERR_CHARGER,
    RTS_RLS23L_BOARD_ERR_RLS,
    RTS_RLS23L_BOARD_ERR_GSM,
    RTS_RLS23L_BOARD_ERR_LORA_MODULE,
};

typedef struct rts_rls23l_peripherals_s
{
    I2C_HandleTypeDef imu_handle;
    TIM_HandleTypeDef tim2_handle;
    struct 
    {
        SPI_HandleTypeDef spi_handle;
        DMA_HandleTypeDef spi_tx_dma_handle;
        DMA_HandleTypeDef spi_rx_dma_handle;
    } a121;
} peripherals_t;

enum rts_rls23l_board_err_e bsp_board_init(void);
peripherals_t *const        bsp_get_peripheral_handles(void);
TIM_HandleTypeDef          *bsp_get_tim2_handle(void);
I2C_HandleTypeDef          *bsp_get_imu_handle(void);
SPI_HandleTypeDef          *bsp_get_a121_spi_handle(void);
DMA_HandleTypeDef          *bsp_get_a121_dma_tx_handle(void);
DMA_HandleTypeDef          *bsp_get_a121_dma_rx_handle(void);

void Error_Handler();

#endif

/* EOF */