/**
 * @file       application.h
 * @brief      Module that contains the application functionality.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APPLICATION_H
#define APPLICATION_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "irmp.h"

/* Exported macro ------------------------------------------------------------*/
#define BACKUP_REG_BOOTLOADER       RTC_BKP_DR1
#define BACKUP_REG_RESET            RTC_BKP_DR2
#define BACKUP_REG_SECOND           RTC_BKP_DR3
#define BACKUP_REG_ALARM            RTC_BKP_DR4

#if defined(STM32L151xB)
#define DATA_EEPROM_START_ADDR      0x08080000
#define DATA_EEPROM_END_ADDR        0x080803FF
#define DATA_EEPROM_PAGE_SIZE       0x8
#endif

enum ADDRESSES
{
   ADDRESS_irmp_power_on      = 0,
   ADDRESS_irmp_power_off     = 6,
   ADDRESS_irmp_reset         = 12,
   ADDRESS_clock_correction   = 18,
   ADDRESS_wakeup_time        = 22,
   ADDRESS_wakeup_time_span   = 26,
   ADDRESS_min_ir_repeats     = 27,
   ADDRESS_control_pc_enable  = 28,
   ADDRESS_forward_ir_enable  = 29,
   ADDRESS_LENGTH /* Used to adjust NUMBER_OF_VARIABLES in eeprom.h when
                     adding variables/addresses and using STM32F1xx. Be
                     careful to consider the length of the last element. */
};

/* Exported types ------------------------------------------------------------*/
typedef struct HIDIRT_DATA
{
   int32_t     clock_correction;
   uint8_t     data_update_pending;
   uint8_t     min_ir_repeats;
   uint8_t     wakeup_time_span;
   IRMP_DATA   irmp_power_on;
   IRMP_DATA   irmp_power_off;
   IRMP_DATA   irmp_reset;
   bool        control_pc_enable;
   bool        forward_ir_enable;
   bool        watchdog_enable;
   bool        watchdog_reset;
} hidirt_data_t;

typedef struct FLAGS
{
   uint8_t     alarm_a_occurred:1;
   uint8_t     alarm_b_occurred:1;
   uint8_t     press_power_button:1;
   uint8_t     press_reset_button:1;
   uint8_t     wakeup_occurred:1;
} flags_t;

/* Exported functions --------------------------------------------------------*/
HAL_StatusTypeDef EEPROM_WriteBytes(uint32_t address, void *data, uint8_t length);
extern void GetHidirtConfig(hidirt_data_t* config);
extern void hidirt_init(void);
extern void hidirt(void);

#endif /* APPLICATION_H */
