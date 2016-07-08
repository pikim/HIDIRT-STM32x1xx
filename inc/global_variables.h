/**
 * @file       global_variables.h
 * @brief      Module that holds peripheral instances and global flags and
 *             variables.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GLOB_VAR_H
#define GLOB_VAR_H

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
#else
  #error Device not specified.
#endif
#include "usbd_def.h"
#include "fifo.h"
#include "application.h"

/* Global variables ----------------------------------------------------------*/
extern IWDG_HandleTypeDef   IwdgHandle;
extern PCD_HandleTypeDef    hpcd;       // in usbd_conf.c
extern RTC_HandleTypeDef    RtcHandle;
extern TIM_HandleTypeDef    TimHandle;
extern USBD_HandleTypeDef   USBD_Device;// in main.c

extern volatile uint8_t     PrevXferComplete; // todo check if necessary!
extern flags_t              flags;
extern fifo_t               irsnd_fifo;

#endif
