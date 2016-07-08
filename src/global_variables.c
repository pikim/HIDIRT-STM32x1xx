/**
 * @file       global_variables.c
 * @brief      Module that holds peripheral instances and global flags and
 *             variables.
 */

/* Includes ------------------------------------------------------------------*/
#include "global_variables.h"
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
#else
  #error Device not specified.
#endif
#include "fifo.h"
#include "application.h"

/* Global variables ----------------------------------------------------------*/
IWDG_HandleTypeDef   IwdgHandle;
//PCD_HandleTypeDef    hpcd;       // in usbd_conf.c
RTC_HandleTypeDef    RtcHandle;
TIM_HandleTypeDef    TimHandle;
//USBD_HandleTypeDef   USBD_Device;// in main.c

volatile uint8_t     PrevXferComplete = 1; // todo check if necessary!
flags_t              flags;
fifo_t               irsnd_fifo;
