/**
  ******************************************************************************
  * @file    App/app_ble.c
  * @author  MCD Application Team
  * @brief   BLE Application
  *****************************************************************************
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
#include "board/pin_defs.h"
#include "main.h"

#include "app_common.h"

#include "dbg_trace.h"
#include "ble.h"
#include "tl.h"
#include "app_ble.h"

#include "shci.h"
#include "stm32_lpm.h"
#include "otp.h"

#include "custom_app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Private includes ----------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/

/**
 * security parameters structure
 */
typedef struct _tSecurityParams
{
  /**
   * IO capability of the device
   */
  uint8_t ioCapability;

  /**
   * Authentication requirement of the device
   * Man In the Middle protection required?
   */
  uint8_t mitm_mode;

  /**
   * bonding mode of the device
   */
  uint8_t bonding_mode;

  /**
   * minimum encryption key size requirement
   */
  uint8_t encryptionKeySizeMin;

  /**
   * maximum encryption key size requirement
   */
  uint8_t encryptionKeySizeMax;

  /**
   * this flag indicates whether the host has to initiate
   * the security, wait for pairing or does not have any security
   * requirements.
   * 0x00 : no security required
   * 0x01 : host should initiate security by sending the slave security
   *        request command
   * 0x02 : host need not send the clave security request but it
   * has to wait for paiirng to complete before doing any other
   * processing
   */
  uint8_t initiateSecurity;

}tSecurityParams;

/**
 * global context
 * contains the variables common to all
 * services
 */
typedef struct _tBLEProfileGlobalContext
{
  /**
   * security requirements of the host
   */
  tSecurityParams bleSecurityParam;

  /**
   * gap service handle
   */
  uint16_t gapServiceHandle;

  /**
   * device name characteristic handle
   */
  uint16_t devNameCharHandle;

  /**
   * appearance characteristic handle
   */
  uint16_t appearanceCharHandle;

  /**
   * connection handle of the current active connection
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandle;

  /**
   * length of the UUID list to be used while advertising
   */
  uint8_t advtServUUIDlen;

  /**
   * the UUID list to be used while advertising
   */
  uint8_t advtServUUID[100];

}BleGlobalContext_t;

typedef struct
{
  BleGlobalContext_t BleApplicationContext_legacy;
  APP_BLE_ConnStatus_t Device_Connection_Status;

}BleApplicationContext_t;


/* Private defines -----------------------------------------------------------*/

#define BD_ADDR_SIZE_LOCAL    6
#define BLE_DEFAULT_PIN                     (111111)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

static const uint8_t a_MBdAddr[BD_ADDR_SIZE_LOCAL] =
{
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF)),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32),
  (uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40)
};

static uint8_t a_BdAddrUdn[BD_ADDR_SIZE_LOCAL];

/**
 *   Identity root key used to derive IRK and DHK(Legacy)
 */
static const uint8_t a_BLE_CfgIrValue[16] = CFG_BLE_IR;

/**
 * Encryption root key used to derive LTK(Legacy) and CSRK
 */
static const uint8_t a_BLE_CfgErValue[16] = CFG_BLE_ER;

/**
 * These are the two tags used to manage a power failure during OTA
 * The MagicKeywordAdress shall be mapped @0x140 from start of the binary image
 * The MagicKeywordvalue is checked in the ble_ota application
 */
PLACE_IN_SECTION("TAG_OTA_END") const uint32_t MagicKeywordValue = 0x94448A29 ;
PLACE_IN_SECTION("TAG_OTA_START") const uint32_t MagicKeywordAddress = (uint32_t)&MagicKeywordValue;

static BleApplicationContext_t BleApplicationContext;

Custom_App_ConnHandle_Not_evt_t HandleNotification;

/**
 * Advertising Data
 */
uint8_t a_AdvData[] =
{
  2, AD_TYPE_TX_POWER_LEVEL, 0 /* 0dBm */, /* Transmission Power */
  10, AD_TYPE_COMPLETE_LOCAL_NAME, 'R', 'T', 'S', '_', 'R', 'L', 'S', '0', '1',  /* Complete name */

};

SemaphoreHandle_t g_x_mtx_hci = NULL;
SemaphoreHandle_t g_x_sem_hci = NULL;
TaskHandle_t g_x_hci_user_evt_task = NULL;
TaskHandle_t g_x_hci_adv_cancel_task = NULL;

/* Private function prototypes -----------------------------------------------*/
static void BLE_UserEvtRx(void *p_Payload);
static void BLE_StatusNot(HCI_TL_CmdStatus_t Status);
static void Ble_Tl_Init(void);
static void Ble_Hci_Gap_Gatt_Init(void);
static const uint8_t* BleGetBdAddress(void);
static void Adv_Request(APP_BLE_ConnStatus_t NewStatus);
static void Adv_Cancel(void);

/* External variables --------------------------------------------------------*/
extern RNG_HandleTypeDef hrng;

/* Functions Definition ------------------------------------------------------*/

void hci_user_evt_task ( void* vp_arg )
{
  while(1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    hci_user_evt_proc();
  }
}

void adv_cancel_task ( void* vp_arg )
{
  while(1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Adv_Cancel();
  }
}

void APP_BLE_Init(void)
{
  SHCI_CmdStatus_t status;
#if (RADIO_ACTIVITY_EVENT != 0)
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
#endif /* RADIO_ACTIVITY_EVENT != 0 */

  SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet =
  {
    {{0,0,0}},                          /**< Header unused */
    {0,                                 /** pBleBufferAddress not used */
     0,                                 /** BleBufferSize not used */
     CFG_BLE_NUM_GATT_ATTRIBUTES,
     CFG_BLE_NUM_GATT_SERVICES,
     CFG_BLE_ATT_VALUE_ARRAY_SIZE,
     CFG_BLE_NUM_LINK,
     CFG_BLE_DATA_LENGTH_EXTENSION,
     CFG_BLE_PREPARE_WRITE_LIST_SIZE,
     CFG_BLE_MBLOCK_COUNT,
     CFG_BLE_MAX_ATT_MTU,
     CFG_BLE_PERIPHERAL_SCA,
     CFG_BLE_CENTRAL_SCA,
     CFG_BLE_LS_SOURCE,
     CFG_BLE_MAX_CONN_EVENT_LENGTH,
     CFG_BLE_HSE_STARTUP_TIME,
     CFG_BLE_VITERBI_MODE,
     CFG_BLE_OPTIONS,
     0,
     CFG_BLE_MAX_COC_INITIATOR_NBR,
     CFG_BLE_MIN_TX_POWER,
     CFG_BLE_MAX_TX_POWER,
     CFG_BLE_RX_MODEL_CONFIG,
     CFG_BLE_MAX_ADV_SET_NBR,
     CFG_BLE_MAX_ADV_DATA_LEN,
     CFG_BLE_TX_PATH_COMPENS,
     CFG_BLE_RX_PATH_COMPENS,
     CFG_BLE_CORE_VERSION,
     CFG_BLE_OPTIONS_EXT,
	 CFG_BLE_MAX_ADD_EATT_BEARERS
    }
  };

  /**
   * Initialize Ble Transport Layer
   */
  Ble_Tl_Init();

  /**
   * Starts the BLE Stack on CPU2
   */
  status = SHCI_C2_BLE_Init(&ble_init_cmd_packet);
  if (status != SHCI_Success)
  {
    printf("  Fail   : SHCI_C2_BLE_Init command, result: 0x%02x\n\r", status);
    /* if you are here, maybe CPU2 doesn't contain STM32WB_Copro_Wireless_Binaries, see Release_Notes.html */
    Error_Handler();
  }
  else
  {
    printf("  Success: SHCI_C2_BLE_Init command\n\r");
  }

  /**
   * Initialization of HCI & GATT & GAP layer
   */
  Ble_Hci_Gap_Gatt_Init();

  /**
   * Initialization of the BLE Services
   */
  SVCCTL_Init();

  /**
   * Initialization of the BLE App Context
   */
  BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
  BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0xFFFF;

  /**
   * From here, all initialization are BLE application specific
   */

  /**
   * Initialization of ADV - Ad Manufacturer Element - Support OTA Bit Mask
   */
#if (RADIO_ACTIVITY_EVENT != 0)
  ret = aci_hal_set_radio_activity_mask(0x0006);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_hal_set_radio_activity_mask command, result: 0x%x \n\r", ret);
  }
  else
  {
    printf("  Success: aci_hal_set_radio_activity_mask command\n\r");
  }
#endif /* RADIO_ACTIVITY_EVENT != 0 */

  /**
   * Initialize Custom Template Application
   */
  Custom_APP_Init();

  /**
   * Make device discoverable
   */
  BleApplicationContext.BleApplicationContext_legacy.advtServUUID[0] = NULL;
  BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen = 0;

  /**
   * Start to Advertise to be connected by a Client
   */
  Adv_Request(APP_BLE_LP_ADV);

  return;
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification(void *p_Pckt)
{
  hci_event_pckt    *p_event_pckt;
  evt_le_meta_event *p_meta_evt;
  evt_blecore_aci   *p_blecore_evt;
  tBleStatus        ret = BLE_STATUS_INVALID_PARAMS;
  hci_le_connection_complete_event_rp0        *p_connection_complete_event;
  hci_disconnection_complete_event_rp0        *p_disconnection_complete_event;
  hci_le_connection_update_complete_event_rp0 *p_connection_update_complete_event;

  /* PAIRING */
  aci_gap_pairing_complete_event_rp0          *p_pairing_complete;
  /* PAIRING */

  p_event_pckt = (hci_event_pckt*) ((hci_uart_pckt *) p_Pckt)->data;

  switch (p_event_pckt->evt)
  {
    case HCI_DISCONNECTION_COMPLETE_EVT_CODE:
    {      
      p_disconnection_complete_event = (hci_disconnection_complete_event_rp0 *) p_event_pckt->data;

      if (p_disconnection_complete_event->Connection_Handle == BleApplicationContext.BleApplicationContext_legacy.connectionHandle)
      {
        BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0;
        BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
        printf(">>== HCI_DISCONNECTION_COMPLETE_EVT_CODE\n");
        printf("     - Connection Handle:   0x%x\n     - Reason:    0x%x\n\r",
                    p_disconnection_complete_event->Connection_Handle,
                    p_disconnection_complete_event->Reason);
      }

      /* restart advertising */
      Adv_Request(APP_BLE_LP_ADV);

      /**
       * SPECIFIC to Custom Template APP
       */
      HandleNotification.Custom_Evt_Opcode = CUSTOM_DISCON_HANDLE_EVT;
      HandleNotification.ConnectionHandle = BleApplicationContext.BleApplicationContext_legacy.connectionHandle;
      Custom_APP_Notification(&HandleNotification);

      break; /* HCI_DISCONNECTION_COMPLETE_EVT_CODE */
    }

    case HCI_HARDWARE_ERROR_EVT_CODE:
    {
      hci_hardware_error_event_rp0 *p_hardware_error_event;

      p_hardware_error_event = (hci_hardware_error_event_rp0 *)p_event_pckt->data;
	  UNUSED(p_hardware_error_event);
      printf(">>== HCI_HARDWARE_ERROR_EVT_CODE\n");
      printf("Hardware Code = 0x%02X\n",p_hardware_error_event->Hardware_Code);
      break; /* HCI_HARDWARE_ERROR_EVT_CODE */
    }

    case HCI_LE_META_EVT_CODE:
    {
      p_meta_evt = (evt_le_meta_event*) p_event_pckt->data;

      switch (p_meta_evt->subevent)
      {
        case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE:
          p_connection_update_complete_event = (hci_le_connection_update_complete_event_rp0 *) p_meta_evt->data;
          printf(">>== HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE\n");
          printf("     - Connection Interval:   %.2f ms\n     - Connection latency:    %d\n     - Supervision Timeout: %d ms\n\r",
                       p_connection_update_complete_event->Conn_Interval*1.25,
                       p_connection_update_complete_event->Conn_Latency,
                       p_connection_update_complete_event->Supervision_Timeout*10);
          break;

        case HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE:
        {
          p_connection_complete_event = (hci_le_connection_complete_event_rp0 *) p_meta_evt->data;
          /**
           * The connection is done, there is no need anymore to schedule the LP ADV
           */

          printf(">>== HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE - Connection handle: 0x%x\n", p_connection_complete_event->Connection_Handle);
          printf("     - Connection established with Central: @:%02x:%02x:%02x:%02x:%02x:%02x\n",
                      p_connection_complete_event->Peer_Address[5],
                      p_connection_complete_event->Peer_Address[4],
                      p_connection_complete_event->Peer_Address[3],
                      p_connection_complete_event->Peer_Address[2],
                      p_connection_complete_event->Peer_Address[1],
                      p_connection_complete_event->Peer_Address[0]);
          printf("     - Connection Interval:   %.2f ms\n     - Connection latency:    %d\n     - Supervision Timeout: %d ms\n\r",
                      p_connection_complete_event->Conn_Interval*1.25,
                      p_connection_complete_event->Conn_Latency,
                      p_connection_complete_event->Supervision_Timeout*10
                     );

          if (BleApplicationContext.Device_Connection_Status == APP_BLE_LP_CONNECTING)
          {
            /* Connection as client */
            BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_CLIENT;
          }
          else
          {
            /* Connection as server */
            BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_SERVER;
          }
          BleApplicationContext.BleApplicationContext_legacy.connectionHandle = p_connection_complete_event->Connection_Handle;
          /**
           * SPECIFIC to Custom Template APP
           */
          HandleNotification.Custom_Evt_Opcode = CUSTOM_CONN_HANDLE_EVT;
          HandleNotification.ConnectionHandle = BleApplicationContext.BleApplicationContext_legacy.connectionHandle;
          Custom_APP_Notification(&HandleNotification);
          break; /* HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE */
        }

        case HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE:
          printf("HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_READ_REMOTE_FEATURES_PAGE_0_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_READ_REMOTE_FEATURES_PAGE_0_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_LONG_TERM_KEY_REQUEST_SUBEVT_CODE:
          printf("HCI_LE_LONG_TERM_KEY_REQUEST_SUBEVT_CODE\n");
        break;
        case HCI_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_SUBEVT_CODE:
          printf("HCI_LE_REMOTE_CONNECTION_PARAMETER_REQUEST_SUBEVT_CODE\n");
        break;
        case HCI_LE_DATA_LENGTH_CHANGE_SUBEVT_CODE:
          printf("HCI_LE_DATA_LENGTH_CHANGE_SUBEVT_CODE\n");
        break;
        case HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_GENERATE_DHKEY_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_GENERATE_DHKEY_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_DIRECTED_ADVERTISING_REPORT_SUBEVT_CODE:
          printf("HCI_LE_DIRECTED_ADVERTISING_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_EXTENDED_ADVERTISING_REPORT_SUBEVT_CODE:
          printf("HCI_LE_EXTENDED_ADVERTISING_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_REPORT_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SYNC_LOST_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SYNC_LOST_SUBEVT_CODE\n");
        break;
        case HCI_LE_SCAN_TIMEOUT_SUBEVT_CODE:
          printf("HCI_LE_SCAN_TIMEOUT_SUBEVT_CODE\n");
        break;
        case HCI_LE_ADVERTISING_SET_TERMINATED_SUBEVT_CODE:
          printf("HCI_LE_ADVERTISING_SET_TERMINATED_SUBEVT_CODE\n");
        break;
        case HCI_LE_SCAN_REQUEST_RECEIVED_SUBEVT_CODE:
          printf("HCI_LE_SCAN_REQUEST_RECEIVED_SUBEVT_CODE\n");
        break;
        case HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVT_CODE:
          printf("HCI_LE_CHANNEL_SELECTION_ALGORITHM_SUBEVT_CODE\n");
        break;
        case HCI_LE_CONNECTIONLESS_IQ_REPORT_SUBEVT_CODE:
          printf("HCI_LE_CONNECTIONLESS_IQ_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_CONNECTION_IQ_REPORT_SUBEVT_CODE:
          printf("HCI_LE_CONNECTION_IQ_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_CTE_REQUEST_FAILED_SUBEVT_CODE:
          printf("HCI_LE_CTE_REQUEST_FAILED_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED_SUBEVT_CODE\n");
        break;
        case HCI_LE_CIS_ESTABLISHED_SUBEVT_CODE:
          printf("HCI_LE_CIS_ESTABLISHED_SUBEVT_CODE\n");
        break;
        case HCI_LE_CIS_REQUEST_SUBEVT_CODE:
          printf("HCI_LE_CIS_REQUEST_SUBEVT_CODE\n");
        break;
        case HCI_LE_CREATE_BIG_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CREATE_BIG_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_TERMINATE_BIG_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_TERMINATE_BIG_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_BIG_SYNC_ESTABLISHED_SUBEVT_CODE:
          printf("HCI_LE_BIG_SYNC_ESTABLISHED_SUBEVT_CODE\n");
        break;
        case HCI_LE_BIG_SYNC_LOST_SUBEVT_CODE:
          printf("HCI_LE_BIG_SYNC_LOST_SUBEVT_CODE\n");
        break;
        case HCI_LE_REQUEST_PEER_SCA_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_REQUEST_PEER_SCA_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_PATH_LOSS_THRESHOLD_SUBEVT_CODE:
          printf("HCI_LE_PATH_LOSS_THRESHOLD_SUBEVT_CODE\n");
        break;
        case HCI_LE_TRANSMIT_POWER_REPORTING_SUBEVT_CODE:
          printf("HCI_LE_TRANSMIT_POWER_REPORTING_SUBEVT_CODE\n");
        break;
        case HCI_LE_BIGINFO_ADVERTISING_REPORT_SUBEVT_CODE:
          printf("HCI_LE_BIGINFO_ADVERTISING_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_SUBRATE_CHANGE_SUBEVT_CODE:
          printf("HCI_LE_SUBRATE_CHANGE_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_V2_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED_V2_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_REPORT_V2_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_REPORT_V2_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED_V2_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED_V2_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_SUBEVENT_DATA_REQUEST_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_SUBEVENT_DATA_REQUEST_SUBEVT_CODE\n");
        break;
        case HCI_LE_PERIODIC_ADVERTISING_RESPONSE_REPORT_SUBEVT_CODE:
          printf("HCI_LE_PERIODIC_ADVERTISING_RESPONSE_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_ENHANCED_CONNECTION_COMPLETE_V2_SUBEVT_CODE:
          printf("HCI_LE_ENHANCED_CONNECTION_COMPLETE_V2_SUBEVT_CODE\n");
        break;
        case HCI_LE_CIS_ESTABLISHED_V2_SUBEVT_CODE:
          printf("HCI_LE_CIS_ESTABLISHED_V2_SUBEVT_CODE\n");
        break;
        case HCI_LE_READ_ALL_REMOTE_FEATURES_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_READ_ALL_REMOTE_FEATURES_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_READ_REMOTE_SUPPORTED_CAPABILITIES_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_READ_REMOTE_SUPPORTED_CAPABILITIES_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_READ_REMOTE_FAE_TABLE_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_READ_REMOTE_FAE_TABLE_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_SECURITY_ENABLE_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_SECURITY_ENABLE_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_CONFIG_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_CONFIG_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_PROCEDURE_ENABLE_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_PROCEDURE_ENABLE_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_SUBEVENT_RESULT_SUBEVT_CODE:
          printf("HCI_LE_CS_SUBEVENT_RESULT_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_SUBEVENT_RESULT_CONTINUE_SUBEVT_CODE:
          printf("HCI_LE_CS_SUBEVENT_RESULT_CONTINUE_SUBEVT_CODE\n");
        break;
        case HCI_LE_CS_TEST_END_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_CS_TEST_END_COMPLETE_SUBEVT_CODE\n");
        break;
        case HCI_LE_MONITORED_ADVERTISERS_REPORT_SUBEVT_CODE:
          printf("HCI_LE_MONITORED_ADVERTISERS_REPORT_SUBEVT_CODE\n");
        break;
        case HCI_LE_FRAME_SPACE_UPDATE_COMPLETE_SUBEVT_CODE:
          printf("HCI_LE_FRAME_SPACE_UPDATE_COMPLETE_SUBEVT_CODE\n");
        break;

        default:

          break;
      }


      break; /* HCI_LE_META_EVT_CODE */
    }

    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:
      p_blecore_evt = (evt_blecore_aci*) p_event_pckt->data;

      switch (p_blecore_evt->ecode)
      {
        /**
         * SPECIFIC to Custom Template APP
         */

        case ACI_GAP_PROC_COMPLETE_VSEVT_CODE:
          printf(">>== ACI_GAP_PROC_COMPLETE_VSEVT_CODE \r");

          break; /* ACI_GAP_PROC_COMPLETE_VSEVT_CODE */

#if (RADIO_ACTIVITY_EVENT != 0)
        case ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE:
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_BLUE_GPIO_PIN);
        break; /* ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE */
#endif /* RADIO_ACTIVITY_EVENT != 0 */

        /* PAIRING */
        case (ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE):
            printf(">>== ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE\n");

          break;

        case ACI_GAP_PASS_KEY_REQ_VSEVT_CODE:
		{
	      uint32_t pin;
          printf(">>== ACI_GAP_PASS_KEY_REQ_VSEVT_CODE \n");
          pin = BLE_DEFAULT_PIN;

          ret = aci_gap_pass_key_resp(BleApplicationContext.BleApplicationContext_legacy.connectionHandle, pin);
          if (ret != BLE_STATUS_SUCCESS)
          {
            printf("==>> aci_gap_pass_key_resp : Fail, reason: 0x%x\n", ret);
          }
          else
          {
            printf("==>> aci_gap_pass_key_resp : Success \n");
          }

          break;
		}

        case ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE:
          printf(">>== ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE\n");
          printf("     - numeric_value = %ld\n",
                      ((aci_gap_numeric_comparison_value_event_rp0 *)(p_blecore_evt->data))->Numeric_Value);
          printf("     - Hex_value = %lx\n",
                      ((aci_gap_numeric_comparison_value_event_rp0 *)(p_blecore_evt->data))->Numeric_Value);
          ret = aci_gap_numeric_comparison_value_confirm_yesno(BleApplicationContext.BleApplicationContext_legacy.connectionHandle, YES);
          if (ret != BLE_STATUS_SUCCESS)
          {
            printf("==>> aci_gap_numeric_comparison_value_confirm_yesno-->YES : Fail, reason: 0x%x\n", ret);
          }
          else
          {
            printf("==>> aci_gap_numeric_comparison_value_confirm_yesno-->YES : Success \n");
          }

          break;

        case ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE:
          p_pairing_complete = (aci_gap_pairing_complete_event_rp0*)p_blecore_evt->data;

          printf(">>== ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE\n");
          if (p_pairing_complete->Status != 0)
          {
            printf("     - Pairing KO \n     - Status: 0x%x\n     - Reason: 0x%x\n", p_pairing_complete->Status, p_pairing_complete->Reason);
          }
          else
          {
            printf("     - Pairing Success\n");
          }
          printf("\n");


          break;
        /* PAIRING */
        case ACI_GATT_INDICATION_VSEVT_CODE:
        {
          printf(">>== ACI_GATT_INDICATION_VSEVT_CODE \r");
          aci_gatt_confirm_indication(BleApplicationContext.BleApplicationContext_legacy.connectionHandle);
        }
        break;

        case ACI_HAL_WARNING_VSEVT_CODE:
        {
          aci_hal_warning_event_rp0 *p_warning_event;

	      p_warning_event = (aci_hal_warning_event_rp0 *)p_blecore_evt->data;
		  UNUSED(p_warning_event);
          printf(">>== ACI_HAL_WARNING_VSEVT_CODE\n");
          printf("Warning Type = 0x%02X\n", p_warning_event->Warning_Type);

          break;
        }

      }
      break; /* HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE */

    default:
      break;
  }

  return (SVCCTL_UserEvtFlowEnable);
}

APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void)
{
  return BleApplicationContext.Device_Connection_Status;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Ble_Tl_Init(void)
{
  HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;

  g_x_mtx_hci = xSemaphoreCreateMutex();
  g_x_sem_hci = xSemaphoreCreateBinary();
  /**
   * Register the hci transport layer to handle BLE User Asynchronous Events
   */
  xTaskCreate(hci_user_evt_task, "hciAsyncTask", 0x1000, NULL, configMAX_PRIORITIES - 2, &g_x_hci_user_evt_task);

  Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
  Hci_Tl_Init_Conf.StatusNotCallBack = BLE_StatusNot;
  hci_init(BLE_UserEvtRx, (void*) &Hci_Tl_Init_Conf);

  return;
}

static void Ble_Hci_Gap_Gatt_Init(void)
{
  uint8_t role;
  uint16_t gap_service_handle = 0, gap_dev_name_char_handle, gap_appearance_char_handle = 0;
  const uint8_t *p_bd_addr;
  uint16_t a_appearance[1] = {BLE_CFG_GAP_APPEARANCE};
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  printf("==>> Start Ble_Hci_Gap_Gatt_Init function\n");

  /**
   * Initialize HCI layer
   */
  /*HCI Reset to synchronise BLE Stack*/
  ret = hci_reset();
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : hci_reset command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: hci_reset command\n");
  }

  /**
   * Write the BD Address
   */
  p_bd_addr = BleGetBdAddress();
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBLIC_ADDRESS_OFFSET, CONFIG_DATA_PUBLIC_ADDRESS_LEN, (uint8_t*) p_bd_addr);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_PUBLIC_ADDRESS_OFFSET, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_hal_write_config_data command - CONFIG_DATA_PUBLIC_ADDRESS_OFFSET\n");
    printf("  Public Bluetooth Address: %02x:%02x:%02x:%02x:%02x:%02x\n",p_bd_addr[5],p_bd_addr[4],p_bd_addr[3],p_bd_addr[2],p_bd_addr[1],p_bd_addr[0]);
  }

  /**
   * Write Identity root key used to derive IRK and DHK(Legacy)
   */
  ret = aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN, (uint8_t*)a_BLE_CfgIrValue);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET\n");
  }

  /**
   * Write Encryption root key used to derive LTK and CSRK
   */
  ret = aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN, (uint8_t*)a_BLE_CfgErValue);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET\n");
  }

  /**
   * Set TX Power.
   */
  ret = aci_hal_set_tx_power_level(1, CFG_TX_POWER);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_hal_set_tx_power_level command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_hal_set_tx_power_level command\n");
  }

  /**
   * Initialize GATT interface
   */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_gatt_init command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_gatt_init command\n");
  }

  /**
   * Initialize GAP interface
   */
  role = GAP_PERIPHERAL_ROLE;

  if (role > 0)
  {
    const char *name = CFG_GAP_DEVICE_NAME;
    ret = aci_gap_init(role,
                       CFG_PRIVACY,
                       CFG_GAP_DEVICE_NAME_LENGTH,
                       &gap_service_handle,
                       &gap_dev_name_char_handle,
                       &gap_appearance_char_handle);

    if (ret != BLE_STATUS_SUCCESS)
    {
      printf("  Fail   : aci_gap_init command, result: 0x%x \n", ret);
    }
    else
    {
      printf("  Success: aci_gap_init command\n");
    }

    ret = aci_gatt_update_char_value(gap_service_handle, gap_dev_name_char_handle, 0, strlen(name), (uint8_t *) name);
    if (ret != BLE_STATUS_SUCCESS)
    {
      BLE_DBG_SVCCTL_MSG("  Fail   : aci_gatt_update_char_value - Device Name\n");
    }
    else
    {
      BLE_DBG_SVCCTL_MSG("  Success: aci_gatt_update_char_value - Device Name\n");
    }
  }

  ret = aci_gatt_update_char_value(gap_service_handle,
                                   gap_appearance_char_handle,
                                   0,
                                   2,
                                   (uint8_t *)&a_appearance);
  if (ret != BLE_STATUS_SUCCESS)
  {
    BLE_DBG_SVCCTL_MSG("  Fail   : aci_gatt_update_char_value - Appearance\n");
  }
  else
  {
    BLE_DBG_SVCCTL_MSG("  Success: aci_gatt_update_char_value - Appearance\n");
  }

  /**
   * Initialize Default PHY
   */
  ret = hci_le_set_default_phy(ALL_PHYS_PREFERENCE,TX_2M_PREFERRED,RX_2M_PREFERRED);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : hci_le_set_default_phy command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: hci_le_set_default_phy command\n");
  }

  /**
   * Initialize IO capability
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability = CFG_IO_CAPABILITY;
  ret = aci_gap_set_io_capability(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_gap_set_io_capability command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_gap_set_io_capability command\n");
  }

  /**
   * Initialize authentication
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin = CFG_ENCRYPTION_KEY_SIZE_MIN;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax = CFG_ENCRYPTION_KEY_SIZE_MAX;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode = CFG_BONDING_MODE;

  ret = aci_gap_set_authentication_requirement(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
                                               BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
                                               CFG_SC_SUPPORT,
                                               CFG_KEYPRESS_NOTIFICATION_SUPPORT,
                                               BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
                                               BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
                                               USE_FIXED_PIN_FOR_PAIRING_FORBIDDEN, /* deprecated feature */
                                               0,                                   /* deprecated feature */
                                               CFG_IDENTITY_ADDRESS);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("  Fail   : aci_gap_set_authentication_requirement command, result: 0x%x \n", ret);
  }
  else
  {
    printf("  Success: aci_gap_set_authentication_requirement command\n");
  }

  /**
   * Initialize whitelist
   */
  if (BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode)
  {
    ret = aci_gap_configure_whitelist();
    if (ret != BLE_STATUS_SUCCESS)
    {
      printf("  Fail   : aci_gap_configure_whitelist command, result: 0x%x \n", ret);
    }
    else
    {
      printf("  Success: aci_gap_configure_whitelist command\n");
    }
  }
  printf("==>> End Ble_Hci_Gap_Gatt_Init function\n\r");
}

static void Adv_Request(APP_BLE_ConnStatus_t NewStatus)
{
  tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

  BleApplicationContext.Device_Connection_Status = NewStatus;
  /* Start Fast or Low Power Advertising */
  ret = aci_gap_set_discoverable(ADV_TYPE,
                                 CFG_LP_CONN_ADV_INTERVAL_MIN,
                                 CFG_LP_CONN_ADV_INTERVAL_MAX,
                                 CFG_BLE_ADDRESS_TYPE,
                                 ADV_FILTER,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("==>> aci_gap_set_discoverable - fail, result: 0x%x \n", ret);
  }
  else
  {
    printf("==>> aci_gap_set_discoverable - Success\n");
  }

  /* Update Advertising data */
  ret = aci_gap_update_adv_data(sizeof(a_AdvData), (uint8_t*) a_AdvData);
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("==>> Start LP Advertising Failed , result: %d \n\r", ret);
  }
  else
  {
    printf("==>> Success: Start LP Advertising \n\r");
  }

  return;
}

const uint8_t* BleGetBdAddress(void)
{
  uint8_t *p_otp_addr;
  const uint8_t *p_bd_addr;
  uint32_t udn;
  uint32_t company_id;
  uint32_t device_id;

  udn = LL_FLASH_GetUDN();

  if (udn != 0xFFFFFFFF)
  {
    company_id = LL_FLASH_GetSTCompanyID();
    device_id = LL_FLASH_GetDeviceID();

    /**
     * Public Address with the ST company ID
     * bit[47:24] : 24bits (OUI) equal to the company ID
     * bit[23:16] : Device ID.
     * bit[15:0] : The last 16bits from the UDN
     * Note: In order to use the Public Address in a final product, a dedicated
     * 24bits company ID (OUI) shall be bought.
     */
    a_BdAddrUdn[0] = (uint8_t)(udn & 0x000000FF);
    a_BdAddrUdn[1] = (uint8_t)((udn & 0x0000FF00) >> 8);
    a_BdAddrUdn[2] = (uint8_t)device_id;
    a_BdAddrUdn[3] = (uint8_t)(company_id & 0x000000FF);
    a_BdAddrUdn[4] = (uint8_t)((company_id & 0x0000FF00) >> 8);
    a_BdAddrUdn[5] = (uint8_t)((company_id & 0x00FF0000) >> 16);

    p_bd_addr = (const uint8_t *)a_BdAddrUdn;
  }
  else
  {
    p_otp_addr = OTP_Read(0);
    if (p_otp_addr)
    {
      p_bd_addr = ((OTP_ID0_t*)p_otp_addr)->bd_address;
    }
    else
    {
      p_bd_addr = a_MBdAddr;
    }
  }

  return p_bd_addr;
}



/*************************************************************
 *
 * SPECIFIC FUNCTIONS FOR CUSTOM
 *
 *************************************************************/
static void Adv_Cancel(void)
{
  if (BleApplicationContext.Device_Connection_Status != APP_BLE_CONNECTED_SERVER)
  {
    tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

    ret = aci_gap_set_non_discoverable();

    BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
    if (ret != BLE_STATUS_SUCCESS)
    {
      printf("** STOP ADVERTISING **  Failed \r\n\r");
    }
    else
    {
      printf("  \r\n\r");
      printf("** STOP ADVERTISING **  \r\n\r");
    }
  }

  return;
}

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void hci_notify_asynch_evt(void* p_Data)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;      
  xTaskNotifyFromISR(g_x_hci_user_evt_task, 1, eSetBits, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  return;
}

void hci_cmd_resp_release(uint32_t Flag)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(g_x_sem_hci, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  return;
}

void hci_cmd_resp_wait(uint32_t Timeout)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreTakeFromISR(g_x_sem_hci, &xHigherPriorityTaskWoken);

  return;
}

static void BLE_UserEvtRx(void *p_Payload)
{
  SVCCTL_UserEvtFlowStatus_t svctl_return_status;
  tHCI_UserEvtRxParam *p_param;

  p_param = (tHCI_UserEvtRxParam *)p_Payload;

  svctl_return_status = SVCCTL_UserEvtRx((void *)&(p_param->pckt->evtserial));
  if (svctl_return_status != SVCCTL_UserEvtFlowDisable)
  {
    p_param->status = HCI_TL_UserEventFlow_Enable;
  }
  else
  {
    p_param->status = HCI_TL_UserEventFlow_Disable;
  }

  return;
}

static void BLE_StatusNot(HCI_TL_CmdStatus_t Status)
{
  switch (Status)
  {
    case HCI_TL_CmdBusy:
    {
      xSemaphoreTake(g_x_mtx_hci, portMAX_DELAY);
      break;
    }

    case HCI_TL_CmdAvailable:
    {
      xSemaphoreGive(g_x_mtx_hci);
      break;
    }

    default:
      break;
  }

  return;
}

void SVCCTL_ResumeUserEventFlow(void)
{
  hci_resume_flow();

  return;
}

