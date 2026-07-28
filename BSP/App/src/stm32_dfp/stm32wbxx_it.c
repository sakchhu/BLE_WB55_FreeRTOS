/**
  ******************************************************************************
  * @file    stm32wbxx_it.c
  * @brief   Interrupt Service Routines.
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
#include <stdio.h>
#include "stm32wbxx_hal.h"
#include "app_conf.h"
#include "app_entry.h"
#include "app_common.h"
#include "stm32wbxx_it.h"

/* External variables --------------------------------------------------------*/
extern IPCC_HandleTypeDef hipcc;
extern RTC_HandleTypeDef hrtc;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  HAL_RCC_NMI_IRQHandler();
   while (1)
  {
  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/* STM32WBxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wbxx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles IPCC RX occupied interrupt.
  */
void IPCC_C1_RX_IRQHandler(void)
{
  HAL_IPCC_RX_IRQHandler(&hipcc);
}

/**
  * @brief This function handles IPCC TX free interrupt.
  */
void IPCC_C1_TX_IRQHandler(void)
{
  HAL_IPCC_TX_IRQHandler(&hipcc);
}

/**
  * @brief This function handles HSEM global interrupt.
  */
void HSEM_IRQHandler(void)
{
  HAL_HSEM_IRQHandler();
}


