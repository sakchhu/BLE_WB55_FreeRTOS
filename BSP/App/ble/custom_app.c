/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "custom_app.h"
#include "app_common.h"
#include "ble.h"
#include "custom_stm.h"
#include "dbg_trace.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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
    uint8_t Tx_Send_Logs;

    uint16_t ConnectionHandle;
} Custom_App_Context_t;

/* Private defines ------------------------------------------------------------*/
#define TX_INTERVAL (1000)

/* Private macros -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[512];
uint8_t NotifyCharData[512];

TaskHandle_t gs_tx_task = NULL;

/* Private function prototypes -----------------------------------------------*/
/* Nordic_UART_Service */
static void Custom_Tx_Update_Char_Ext(void);

/* Functions Definition ------------------------------------------------------*/

void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification) {
    switch (pNotification->Custom_Evt_Opcode) {

    /* Nordic_UART_Service */
    case CUSTOM_STM_RX_WRITE_NO_RESP_EVT:
    case CUSTOM_STM_RX_WRITE_EVT:
    {
        if ( 1 == Custom_App_Context.Tx_Notification_Status )
        {
            Custom_App_Context.rx_write_data.data_len = pNotification->DataTransfered.Length;
            memcpy(Custom_App_Context.rx_write_data.data_buf, pNotification->DataTransfered.pPayload, Custom_App_Context.rx_write_data.data_len);
            xTaskNotify(gs_tx_task, 0, eSetBits);
        }
    }
    break;

    case CUSTOM_STM_TX_NOTIFY_ENABLED_EVT:
        Custom_App_Context.Tx_Notification_Status = 1;
        vTaskResume(gs_tx_task);
    break;

    case CUSTOM_STM_TX_NOTIFY_DISABLED_EVT:
        Custom_App_Context.Tx_Notification_Status = 0;
        vTaskSuspend(gs_tx_task);
    break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
    break;

    default:
    break;
    }
    return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification) {
    /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

    /* USER CODE END CUSTOM_APP_Notification_1 */

    switch (pNotification->Custom_Evt_Opcode) {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT:
        /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */
        Custom_App_Context.ConnectionHandle = pNotification->ConnectionHandle;
        /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
        break;

    case CUSTOM_DISCON_HANDLE_EVT:
        /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */
        Custom_App_Context.Tx_Notification_Status = 0;
        Custom_App_Context.ConnectionHandle = 0xFFFF;
        Custom_App_Context.rx_write_data.data_len = 0;
        /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
        break;

    default:
        /* USER CODE BEGIN CUSTOM_APP_Notification_default */

        /* USER CODE END CUSTOM_APP_Notification_default */
        break;
    }

    /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

    /* USER CODE END CUSTOM_APP_Notification_2 */

    return;
}

__USED static void nus_tx_task(void* vp_arg)
{
    while ( 1 )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if ( Custom_App_Context.ConnectionHandle != 0xFFFF && 1 == Custom_App_Context.Tx_Notification_Status && Custom_App_Context.rx_write_data.data_len != 0  )
        {
            Custom_STM_App_Update_Char_Variable_Length(CUSTOM_STM_TX, Custom_App_Context.rx_write_data.data_buf, Custom_App_Context.rx_write_data.data_len);
        }
        else
        {
            printf("Echo Fail!!\n");
        }
    }
}

void Custom_APP_Init(void) {
    xTaskCreate(nus_tx_task, "nusNotifyTx", 0x1000, NULL, configMAX_PRIORITIES - 4, &gs_tx_task);

    Custom_App_Context.Tx_Notification_Status = 0;
    Custom_App_Context.ConnectionHandle = 0xFFFF;
    Custom_App_Context.rx_write_data.data_len = 0;
    Custom_App_Context.Tx_Send_Logs = 0;
    return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* Nordic_UART_Service */
__USED void Custom_Tx_Update_Char_Ext(void) /* Property Read */
{
    uint8_t updateflag = 0;

    if (updateflag != 0) {
        Custom_STM_App_Update_Char_Ext(Custom_App_Context.ConnectionHandle, CUSTOM_STM_TX, (uint8_t *)UpdateCharData);
    }

    /* USER CODE BEGIN Tx_UC_Last*/

    /* USER CODE END Tx_UC_Last*/
    return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
