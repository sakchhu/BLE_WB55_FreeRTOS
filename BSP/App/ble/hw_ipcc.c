/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Target/hw_ipcc.c
  * @author  MCD Application Team
  * @brief   Hardware IPCC source file for STM32WPAN Middleware.
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
#include "app_common.h"
#include "mbox_def.h"
#include "utilities_conf.h"

/* Global variables ---------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define HW_IPCC_TX_PENDING( channel ) ( !(LL_C1_IPCC_IsActiveFlag_CHx( IPCC, channel )) ) &&  (((~(IPCC->C1MR)) & (channel << 16U)))
#define HW_IPCC_RX_PENDING( channel )  (LL_C2_IPCC_IsActiveFlag_CHx( IPCC, channel )) && (((~(IPCC->C1MR)) & (channel << 0U)))

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void (*FreeBufCb)( void );

/* Private function prototypes -----------------------------------------------*/
static void HW_IPCC_BLE_EvtHandler( void );
static void HW_IPCC_BLE_AclDataEvtHandler( void );
static void HW_IPCC_MM_FreeBufHandler( void );
static void HW_IPCC_SYS_CmdEvtHandler( void );
static void HW_IPCC_SYS_EvtHandler( void );
static void HW_IPCC_TRACES_EvtHandler( void );

/* Public function definition -----------------------------------------------*/

/******************************************************************************
 * INTERRUPT HANDLER
 ******************************************************************************/
void HW_IPCC_Rx_Handler( void )
{
  if (HW_IPCC_RX_PENDING( HW_IPCC_SYSTEM_EVENT_CHANNEL ))
  {
      HW_IPCC_SYS_EvtHandler();
  }
  else if (HW_IPCC_RX_PENDING( HW_IPCC_BLE_EVENT_CHANNEL ))
  {
    HW_IPCC_BLE_EvtHandler();
  }
  else if (HW_IPCC_RX_PENDING( HW_IPCC_TRACES_CHANNEL ))
  {
    HW_IPCC_TRACES_EvtHandler();
  }

  return;
}

void HW_IPCC_Tx_Handler( void )
{
  if (HW_IPCC_TX_PENDING( HW_IPCC_SYSTEM_CMD_RSP_CHANNEL ))
  {
    HW_IPCC_SYS_CmdEvtHandler();
  }
  else if (HW_IPCC_TX_PENDING( HW_IPCC_MM_RELEASE_BUFFER_CHANNEL ))
  {
    HW_IPCC_MM_FreeBufHandler();
  }
  else if (HW_IPCC_TX_PENDING( HW_IPCC_HCI_ACL_DATA_CHANNEL ))
  {
    HW_IPCC_BLE_AclDataEvtHandler();
  }

  return;
}
/******************************************************************************
 * GENERAL
 ******************************************************************************/
void HW_IPCC_Enable( void )
{
  /**
  * Such as IPCC IP available to the CPU2, it is required to keep the IPCC clock running
  * when FUS is running on CPU2 and CPU1 enters deep sleep mode
  */
  LL_C2_AHB3_GRP1_EnableClock(LL_C2_AHB3_GRP1_PERIPH_IPCC);

  /**
  * When the device is out of standby, it is required to use the EXTI mechanism to wakeup CPU2
  */
  LL_EXTI_EnableRisingTrig_32_63( LL_EXTI_LINE_41 );
  /* It is required to have at least a system clock cycle before a SEV after LL_EXTI_EnableRisingTrig_32_63() */
  LL_C2_EXTI_EnableEvent_32_63( LL_EXTI_LINE_41 );

  /**
   * In case the SBSFU is implemented, it may have already set the C2BOOT bit to startup the CPU2.
   * In that case, to keep the mechanism transparent to the user application, it shall call the system command
   * SHCI_C2_Reinit( ) before jumping to the application.
   * When the CPU2 receives that command, it waits for its event input to be set to restart the CPU2 firmware.
   * This is required because once C2BOOT has been set once, a clear/set on C2BOOT has no effect.
   * When SHCI_C2_Reinit( ) is not called, generating an event to the CPU2 does not have any effect
   * So, by default, the application shall both set the event flag and set the C2BOOT bit.
   */
  __SEV( );       /* Set the internal event flag and send an event to the CPU2 */
  __WFE( );       /* Clear the internal event flag */
  LL_PWR_EnableBootC2( );

  return;
}

void HW_IPCC_Init( void )
{
  LL_AHB3_GRP1_EnableClock( LL_AHB3_GRP1_PERIPH_IPCC );

  LL_C1_IPCC_EnableIT_RXO( IPCC );
  LL_C1_IPCC_EnableIT_TXF( IPCC );

  HAL_NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
  HAL_NVIC_EnableIRQ(IPCC_C1_TX_IRQn);

  return;
}

/******************************************************************************
 * BLE
 ******************************************************************************/
void HW_IPCC_BLE_Init( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_EnableReceiveChannel( IPCC, HW_IPCC_BLE_EVENT_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  return;
}

void HW_IPCC_BLE_SendCmd( void )
{
  LL_C1_IPCC_SetFlag_CHx( IPCC, HW_IPCC_BLE_CMD_CHANNEL );

  return;
}

static void HW_IPCC_BLE_EvtHandler( void )
{
  HW_IPCC_BLE_RxEvtNot();

  LL_C1_IPCC_ClearFlag_CHx( IPCC, HW_IPCC_BLE_EVENT_CHANNEL );

  return;
}

void HW_IPCC_BLE_SendAclData( void )
{
  LL_C1_IPCC_SetFlag_CHx( IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL );
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_EnableTransmitChannel( IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  return;
}

static void HW_IPCC_BLE_AclDataEvtHandler( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_DisableTransmitChannel( IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  HW_IPCC_BLE_AclDataAckNot();

  return;
}

__weak void HW_IPCC_BLE_AclDataAckNot( void ){};
__weak void HW_IPCC_BLE_RxEvtNot( void ){};

/******************************************************************************
 * SYSTEM
 ******************************************************************************/
void HW_IPCC_SYS_Init( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_EnableReceiveChannel( IPCC, HW_IPCC_SYSTEM_EVENT_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  return;
}

void HW_IPCC_SYS_SendCmd( void )
{
  LL_C1_IPCC_SetFlag_CHx( IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL );
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_EnableTransmitChannel( IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  return;
}

static void HW_IPCC_SYS_CmdEvtHandler( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_DisableTransmitChannel( IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  HW_IPCC_SYS_CmdEvtNot();

  return;
}

static void HW_IPCC_SYS_EvtHandler( void )
{
  HW_IPCC_SYS_EvtNot();

  LL_C1_IPCC_ClearFlag_CHx( IPCC, HW_IPCC_SYSTEM_EVENT_CHANNEL );

  return;
}

__weak void HW_IPCC_SYS_CmdEvtNot( void ){};
__weak void HW_IPCC_SYS_EvtNot( void ){};


/******************************************************************************
 * MEMORY MANAGER
 ******************************************************************************/
void HW_IPCC_MM_SendFreeBuf( void (*cb)( void ) )
{
  if ( LL_C1_IPCC_IsActiveFlag_CHx( IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL ) )
  {
    FreeBufCb = cb;
    UTILS_ENTER_CRITICAL_SECTION();
    LL_C1_IPCC_EnableTransmitChannel( IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL );
    UTILS_EXIT_CRITICAL_SECTION();
  }
  else
  {
    cb();

    LL_C1_IPCC_SetFlag_CHx( IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL );
  }

  return;
}

static void HW_IPCC_MM_FreeBufHandler( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_DisableTransmitChannel( IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  FreeBufCb();

  LL_C1_IPCC_SetFlag_CHx( IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL );

  return;
}

/******************************************************************************
 * TRACES
 ******************************************************************************/
void HW_IPCC_TRACES_Init( void )
{
  UTILS_ENTER_CRITICAL_SECTION();
  LL_C1_IPCC_EnableReceiveChannel( IPCC, HW_IPCC_TRACES_CHANNEL );
  UTILS_EXIT_CRITICAL_SECTION();

  return;
}

static void HW_IPCC_TRACES_EvtHandler( void )
{
  HW_IPCC_TRACES_EvtNot();

  LL_C1_IPCC_ClearFlag_CHx( IPCC, HW_IPCC_TRACES_CHANNEL );

  return;
}

__weak void HW_IPCC_TRACES_EvtNot( void ){};
