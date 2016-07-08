/**
 * @file       configuration.h
 * @brief      Module that contains the configuration functionality.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

#define POWER_PORT_LETTER        B     /* port of power switch output */
#define POWER_BIT_NUMBER         14    /* bit where OK1 will be connected */
#define POWER_PRESSED            SET
#define POWER_RELEASED           RESET

#define RESET_PORT_LETTER        B     /* port of reset switch output */
#define RESET_BIT_NUMBER         13    /* bit where optocoupler will be connected */
#define RESET_PRESSED            SET
#define RESET_RELEASED           RESET

#define PSU_SENSE_PORT_LETTER    A     /* port of PSU sense input */
#define PSU_SENSE_BIT_NUMBER     8     /* bit where optocoupler will be connected */

#define USB_SENSE_PORT_LETTER    B     /* port of USB sense input */
#define USB_SENSE_BIT_NUMBER     8     /* bit where USB sense will be connected */

#define IR_ENABLE_PORT_LETTER    A     /* register for enabling IR receiver output */
#define IR_ENABLE_BIT_NUMBER     1     /* bit where enable MOSFET will be connected */
#define IR_ENABLED               RESET
#define IR_DISABLED              SET

#define IRMP_IRSND_TIMER_NUMBER  3

/* independent watchdog configuration */
#define IWDG_TIMEOUT_IN_SECONDS  2

/* select GPIO speed */
#if defined(STM32F103xB)
  #define GPIO_SPEED             GPIO_SPEED_LOW
#elif defined(STM32L151xB)
  #define GPIO_SPEED             GPIO_SPEED_VERY_LOW
#else
  #error Device not specified.
#endif

/* uncomment when external backup supply is used and current consumption shall
 * be minimized */
//#define USE_BACKUP_SUPPLY

/* do not change the following lines unless you know what you're doing */
#define _CONCAT(a,b)             a##b
#define CONCAT(a,b)              _CONCAT(a,b)
#define _CONCAT3(a,b,c)          a##b##c
#define CONCAT3(a,b,c)           _CONCAT3(a,b,c)

#define POWER_BIT                CONCAT(GPIO_PIN_, POWER_BIT_NUMBER)
#define POWER_PORT               CONCAT(GPIO, POWER_PORT_LETTER)
#define POWER_PORT_CLK_EN        CONCAT3(__HAL_RCC_GPIO, POWER_PORT_LETTER, _CLK_ENABLE)

#define RESET_BIT                CONCAT(GPIO_PIN_, RESET_BIT_NUMBER)
#define RESET_PORT               CONCAT(GPIO, RESET_PORT_LETTER)
#define RESET_PORT_CLK_EN        CONCAT3(__HAL_RCC_GPIO, RESET_PORT_LETTER, _CLK_ENABLE)

#define PSU_SENSE_BIT            CONCAT(GPIO_PIN_, PSU_SENSE_BIT_NUMBER)
#define PSU_SENSE_PORT           CONCAT(GPIO, PSU_SENSE_PORT_LETTER)
#define PSU_SENSE_PORT_CLK_EN    CONCAT3(__HAL_RCC_GPIO, PSU_SENSE_PORT_LETTER, _CLK_ENABLE)

#define USB_SENSE_BIT            CONCAT(GPIO_PIN_, USB_SENSE_BIT_NUMBER)
#define USB_SENSE_PORT           CONCAT(GPIO, USB_SENSE_PORT_LETTER)
#define USB_SENSE_PORT_CLK_EN    CONCAT3(__HAL_RCC_GPIO, USB_SENSE_PORT_LETTER, _CLK_ENABLE)

#define IR_ENABLE_BIT            CONCAT(GPIO_PIN_, IR_ENABLE_BIT_NUMBER)
#define IR_ENABLE_PORT           CONCAT(GPIO, IR_ENABLE_PORT_LETTER)
#define IR_ENABLE_PORT_CLK_EN    CONCAT3(__HAL_RCC_GPIO, IR_ENABLE_PORT_LETTER, _CLK_ENABLE)

#define IRMP_IRSND_TIMER         CONCAT(TIM, IRMP_IRSND_TIMER_NUMBER)
#define IRMP_IRSND_TIMER_IRQ     CONCAT3(TIM, IRMP_IRSND_TIMER_NUMBER, _IRQn)
#define IRMP_IRSND_TIMER_CLK_EN  CONCAT3(__HAL_RCC_TIM, IRMP_IRSND_TIMER_NUMBER, _CLK_ENABLE)
#define IRMP_IRSND_TIMER_CLK_DIS CONCAT3(__HAL_RCC_TIM, IRMP_IRSND_TIMER_NUMBER, _CLK_DISABLE)
#define IRMP_IRSND_TIMER_IRQ_HANDLER CONCAT3(TIM, IRMP_IRSND_TIMER_NUMBER, _IRQHandler)

/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void GPIO_ConfigAsAnalog(void);
extern void GPIO_Configuration(void);
extern void SystemClockConfig_STOP(void);
extern void PrepareStopMode(void);
extern void LeaveStopMode(void);

#endif /* CONFIGURATION_H */
