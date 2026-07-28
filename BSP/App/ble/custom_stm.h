/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_stm.h
  * @author  MCD Application Team
  * @brief   Header for custom_stm.c module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CUSTOM_STM_H
#define CUSTOM_STM_H

#include "ble_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* Nordic_UART_Service */
  CUSTOM_STM_RX,
  CUSTOM_STM_TX,
  CUSTOM_STM_BLOB_RX,
  CUSTOM_STM_BLOB_TX,
} Custom_STM_Char_Opcode_t;

typedef enum
{
  /* RX_Characteristic */
  CUSTOM_STM_RX_WRITE_NO_RESP_EVT,
  CUSTOM_STM_RX_WRITE_EVT,
  /* TX_Characteristic */
  CUSTOM_STM_TX_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_TX_NOTIFY_DISABLED_EVT,
  CUSTOM_STM_NOTIFICATION_COMPLETE_EVT,

  /* BLOB_RX_Characteristic */
  CUSTOM_STM_BLOB_RX_WRITE_NO_RESP_EVT,
  CUSTOM_STM_BLOB_RX_WRITE_EVT,
  /* BLOB_TX_Characteristic */
  CUSTOM_STM_BLOB_TX_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_BLOB_TX_NOTIFY_DISABLED_EVT,
  CUSTOM_STM_BLOB_NOTIFICATION_COMPLETE_EVT,

  CUSTOM_STM_BOOT_REQUEST_EVT
} Custom_STM_Opcode_evt_t;

typedef struct
{
  uint8_t * pPayload;
  uint8_t   Length;
} Custom_STM_Data_t;

typedef struct
{
  Custom_STM_Opcode_evt_t       Custom_Evt_Opcode;
  Custom_STM_Data_t             DataTransfered;
  uint16_t                      ConnectionHandle;
  uint8_t                       ServiceInstance;
  uint16_t                      AttrHandle;
} Custom_STM_App_Notification_evt_t;

/* Exported constants --------------------------------------------------------*/
extern uint16_t SizeRx;
extern uint16_t SizeTx;

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void SVCCTL_InitCustomSvc(void);
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification);
tBleStatus Custom_STM_App_Update_Char(Custom_STM_Char_Opcode_t CharOpcode,  uint8_t *pPayload);
tBleStatus Custom_STM_App_Update_Char_Variable_Length(Custom_STM_Char_Opcode_t CharOpcode, uint8_t *pPayload, uint8_t size);
tBleStatus Custom_STM_App_Update_Char_Ext(uint16_t Connection_Handle, Custom_STM_Char_Opcode_t CharOpcode, uint8_t *pPayload);

#ifdef __cplusplus
}
#endif

#endif /*CUSTOM_STM_H */
