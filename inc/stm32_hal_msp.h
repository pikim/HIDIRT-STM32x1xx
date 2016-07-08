/**
 * @file       stm32_hal_msp.h
 * @brief      Module that configures the used peripherals..
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_HAL_MSP_H
#define STM32_HAL_MSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "application.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Defines related to RTC configuration */
/* Uncomment to enable the desired Clock Source */
/* #define RTC_CLOCK_SOURCE_LSI */
#define RTC_CLOCK_SOURCE_LSE

#define BACKUP_INIT_PATTERN   0x32F2
#define BACKUP_INIT_REGISTER  BACKUP_REG_RESET

#if defined(STM32F103xB)
  #define RTC_ASYNCH_PREDIV   (16384-1) // leads to a 0.5 second interval
#elif defined(STM32L151xB)
  #ifdef RTC_CLOCK_SOURCE_LSI
    #define RTC_ASYNCH_PREDIV 0x7F
    #define RTC_SYNCH_PREDIV  0x0120
  #endif

  #ifdef RTC_CLOCK_SOURCE_LSE
    #define RTC_ASYNCH_PREDIV 0x7F
    #define RTC_SYNCH_PREDIV  0x00FF
  #endif
#else
  #error Device not specified.
#endif

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
extern void HAL_MspInitCustom(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32_HAL_MSP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
