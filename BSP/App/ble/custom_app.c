/**
 ******************************************************************************
 * @file    App/custom_app.c
 * @author  MCD Application Team
 * @brief   Custom Example Application (Server)
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "custom_app.h"
#include "cmsis_gcc.h"
#include "custom_stm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    /* Nordic_UART_Service */
    uint8_t Tx_Notification_Status;
    struct
    {
        uint8_t data_len;
        uint8_t data_buf[512];
    } rx_write_data;

    uint16_t ConnectionHandle;
} gatt_app_context_t;

/* Private defines ------------------------------------------------------------*/

/* Private macros -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static gatt_app_context_t gs_app_ctx;

/**
 * END of Section BLE_APP_CONTEXT
 */

TaskHandle_t gs_tx_task = NULL;

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
    switch (pNotification->Custom_Evt_Opcode) {

    /* Nordic_UART_Service */
    case CUSTOM_STM_RX_WRITE_NO_RESP_EVT:
    {
        if ( 1 == gs_app_ctx.Tx_Notification_Status )
        {
            gs_app_ctx.rx_write_data.data_len = pNotification->DataTransfered.Length;
            memcpy(gs_app_ctx.rx_write_data.data_buf, pNotification->DataTransfered.pPayload, gs_app_ctx.rx_write_data.data_len);
            xTaskNotify(gs_tx_task, 1, eSetBits);
        }
    }
    break;

    case CUSTOM_STM_TX_NOTIFY_ENABLED_EVT:
        gs_app_ctx.Tx_Notification_Status = 1;
    break;

    case CUSTOM_STM_TX_NOTIFY_DISABLED_EVT:
        gs_app_ctx.Tx_Notification_Status = 0;
    break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
    break;

    case CUSTOM_STM_BLOB_RX_WRITE_NO_RESP_EVT:
    {
        printf("BLOB_RX evt\n");
        for ( uint8_t i = 0; i < pNotification->DataTransfered.Length; ++i )
        {
            printf("%02X ", pNotification->DataTransfered.pPayload[i]);
        }
        printf("\n");
    }
    break;

    default:
    break;
    }
    return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
    switch (pNotification->Custom_Evt_Opcode)
    {
    case CUSTOM_CONN_HANDLE_EVT:
        gs_app_ctx.ConnectionHandle = pNotification->ConnectionHandle;
    break;

    case CUSTOM_DISCON_HANDLE_EVT:
        gs_app_ctx.Tx_Notification_Status = 0;
        gs_app_ctx.ConnectionHandle = 0xFFFF;
        gs_app_ctx.rx_write_data.data_len = 0;
    break;

    default:
    break;
    }

    return;
}

static void nus_tx_task(void* vp_arg)
{
    while ( 1 )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if ( gs_app_ctx.ConnectionHandle != 0xFFFF && 1 == gs_app_ctx.Tx_Notification_Status && gs_app_ctx.rx_write_data.data_len != 0  )
        {
            Custom_STM_App_Update_Char_Variable_Length(CUSTOM_STM_TX, gs_app_ctx.rx_write_data.data_buf, gs_app_ctx.rx_write_data.data_len);
        }
        else
        {
            printf("Echo Fail!!\n");
        }
    }
}

void Custom_APP_Init(void) {
    xTaskCreate(nus_tx_task, "nusNotifyTx", 0x1000, NULL, configMAX_PRIORITIES - 4, &gs_tx_task);

    gs_app_ctx.Tx_Notification_Status = 0;
    gs_app_ctx.ConnectionHandle = 0xFFFF;
    gs_app_ctx.rx_write_data.data_len = 0;
    return;
}
