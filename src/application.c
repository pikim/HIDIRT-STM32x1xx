/**
 * @file       application.c
 * @brief      Module that contains the application functionality.
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>   // for memcpy
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
  #include "eeprom.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
#else
  #error Device not specified.
#endif
#include "application.h"
#include "debounce.h"
#include "swrtc.h"
#include "irmp.h"
#include "irsnd.h"
#include "usbd_customhid.h"
#include "usbd_customhid_if.h"
#include "stm32_hal_msp.h"
#include "configuration.h"
#include "main.h"
#include "global_variables.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static hidirt_data_t hidirt_data;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Reads bytes from the (emulated) EEPROM.
  * @param  Address:
  * @param  *Data:
  * @param  Length:
  * @return FLASH Status: The returned value can be:
  *   FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
HAL_StatusTypeDef EEPROM_ReadBytes(uint32_t address, void *data, uint8_t length)
{
   HAL_StatusTypeDef status = HAL_OK;

#if defined(STM32F103xB)
   uint16_t dat;

   while(length && (status == HAL_OK))
   {
      status = EE_ReadVariable(address, &dat);

      // copy data if read was successful
      if(status == HAL_OK)
      {
         *(uint8_t*)data = dat;
         data++;
         length--;
         if(length > 0)
         {
            *(uint8_t*)data = dat >> 8;
            data++;
            length--;
         }
         address++;
      }
   }
#elif defined(STM32L151xB)
   // add EEPROM address offset
   address += DATA_EEPROM_START_ADDR;
   memcpy(data, (void *) address, length);
#else
#error Device not specified.
#endif
   return status;
}

/**
  * @brief  Writes bytes into the (emulated) EEPROM.
  * @param  Address:
  * @param  *Data:
  * @param  Length:
  * @return FLASH Status: The returned value can be:
  *   FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
HAL_StatusTypeDef EEPROM_WriteBytes(uint32_t address, void *data, uint8_t length)
{
   HAL_StatusTypeDef status = HAL_OK;

#if defined(STM32F103xB)
   uint16_t dat;

   HAL_FLASH_Unlock();

   while(length && (status == HAL_OK))
   {
      dat = *(uint8_t*)data;
      data++;
      length--;
      if(length > 0)
      {
         dat |= (*(uint8_t*)data << 8);
         data++;
         length--;
      }

      status = EE_WriteVariable(address, dat);
      address++;
   }

   HAL_FLASH_Lock();
#elif defined(STM32L151xB)
   // add EEPROM address offset
   address += DATA_EEPROM_START_ADDR;

   if((address < DATA_EEPROM_START_ADDR) || (address > DATA_EEPROM_END_ADDR))
   {
      return HAL_ERROR;
   }

   HAL_FLASHEx_DATAEEPROM_Unlock();

   while(length && (status == HAL_OK))
   {
      status = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address, *(uint8_t*)data);
      address++;
      data++;
      length--;
   }

   HAL_FLASHEx_DATAEEPROM_Lock();
#else
#error Device not specified.
#endif
   return status;
}

/**
  * @brief  Toggles a pin (power or reset) if the corresponding flag is set. The
  *         pin will be active between the first and second call of this
  *         function, after the flag has been set.
  */
void ProcessButtons(void)
{
   static flags_t last_flags = {0, 0};

   // if supply voltage is present
   if(DEB_GetKeyState(DEB_USB_SENSE))
   {
      // handle power button
      if(flags.press_power_button != last_flags.press_power_button)
      {
         // if PC buttons shall be controlled
         if(hidirt_data.control_pc_enable)
         {
            HAL_GPIO_WritePin(POWER_PORT, POWER_BIT, POWER_PRESSED);
         }
         else // start PC via USB
         {
            HAL_PCD_ActivateRemoteWakeup(&hpcd);
            HAL_Delay(2); // must be active between 1ms and 15ms, so wait 2ms
            HAL_PCD_DeActivateRemoteWakeup(&hpcd);
//todo test instead of           Resume(RESUME_START);
         }
      }
      else
      {
         HAL_GPIO_WritePin(POWER_PORT, POWER_BIT, POWER_RELEASED);
         flags.press_power_button = 0;
      }
      last_flags.press_power_button = flags.press_power_button;

      // handle reset button
      if(flags.press_reset_button != last_flags.press_reset_button)
      {
         // if PC buttons shall be controlled
         if(hidirt_data.control_pc_enable)
         {
            HAL_GPIO_WritePin(RESET_PORT, RESET_BIT, RESET_PRESSED);
         }
      }
      else
      {
         HAL_GPIO_WritePin(RESET_PORT, RESET_BIT, RESET_RELEASED);
         flags.press_reset_button = 0;
      }
      last_flags.press_reset_button = flags.press_reset_button;
   }
}

/**
  * @brief  Compares the contents of two IRMP_DATA structures.
  * @param  *irmp_data_1 is the first struct to compare.
  * @param  *irmp_data_2 is the second struct to compare.
  * @return false (0) if contents differ.
  *         true (!0) if contents are equal.
  */
bool IRMP_DataIsEqual(IRMP_DATA* irmp_data_1, IRMP_DATA* irmp_data_2)
{
   if( (irmp_data_1->command == irmp_data_2->command) &&
       (irmp_data_1->address == irmp_data_2->address) &&
       (irmp_data_1->protocol == irmp_data_2->protocol) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

  /**
  * @brief  Forwards received IR data over USB (and IR diode if enabled).
  * @param  *irmp_data is the received IR data.
  */
void IRMP_ForwardData(IRMP_DATA* irmp_data)
{
   static uint8_t    repeat_ctr = 0;
   uint8_t tx_buffer[USBD_CUSTOMHID_INREPORT_BUF_SIZE];

   if( !(irmp_data->flags & IRMP_FLAG_REPETITION) )
   {
      // new command received, reset counter
      repeat_ctr = 0;
   }
   else
   {
      // still the same command, increase counter
      repeat_ctr++;
   }

   // only send interrupt if first command, or counter >= MinRepeats
   if( (repeat_ctr == 0) || (repeat_ctr >= hidirt_data.min_ir_repeats) )
   {
      // check if last transfer was completed
//      if(PrevXferComplete) // todo check if necessary!
      {
         // copy data and ID to buffer
         memcpy((void*)&tx_buffer[1], irmp_data, sizeof(*irmp_data));
         tx_buffer[0] = 1;

         // write the descriptor through the endpoint
         USBD_CUSTOM_HID_SendReport(&USBD_Device, tx_buffer, sizeof(*irmp_data)+1);
         PrevXferComplete = 0;
      }

      if(repeat_ctr >= hidirt_data.min_ir_repeats)
      {
         // fix overflow for very long key push
         repeat_ctr = hidirt_data.min_ir_repeats;
      }
   }

   // if usage of IRSND is enabled to forward IR codes
   if(hidirt_data.forward_ir_enable)
   {
      // write command to FIFO from where it will be sent later
      FIFO_Write(&irsnd_fifo, (fifo_entry_t*)&irmp_data);
   }
}

/**
  * @brief  Processes received IR data.
  *         Initiates pin toggling (power/ reset) and stores
  *         power-on- and power-off IR codes when necessary.
  * @param  *irmp_data is the received IR data to process.
  */
void IRMP_ProcessData(IRMP_DATA* irmp_data)
{
   static uint8_t reset_ctr = 0;

   // if code is not yet trained
   if( (hidirt_data.irmp_power_on.protocol == 0x00) || (hidirt_data.irmp_power_on.protocol == 0xFF) )
   {
      // update trained code
      memcpy(&hidirt_data.irmp_power_on, irmp_data, sizeof(hidirt_data.irmp_power_on));
      EEPROM_WriteBytes(ADDRESS_irmp_power_on, irmp_data, sizeof(*irmp_data));

      if( (hidirt_data.irmp_power_off.protocol == 0x00) || (hidirt_data.irmp_power_off.protocol == 0xFF) )
      {
         // update trained code
         memcpy(&hidirt_data.irmp_power_off, irmp_data, sizeof(hidirt_data.irmp_power_off));
         EEPROM_WriteBytes(ADDRESS_irmp_power_off, irmp_data, sizeof(*irmp_data));
      }
   }
   else // code is already trained
   {
      IRMP_ForwardData(irmp_data);

      // compare trained codes with last received code
      // if PC is not running and irmp_data is equal to irmp_power_on
      // or PC is running and irmp_data is equal to irmp_power_off
      if( ( IRMP_DataIsEqual(irmp_data, &hidirt_data.irmp_power_on) &&
            !DEB_GetKeyState(DEB_PSU_SENSE) ) ||
          ( IRMP_DataIsEqual(irmp_data, &hidirt_data.irmp_power_off) &&
            DEB_GetKeyState(DEB_PSU_SENSE) ) )
      {
         flags.press_power_button = 1;
      }
   }

   // if reset key was pressed
   if( IRMP_DataIsEqual(irmp_data, &hidirt_data.irmp_reset) )
   {
      // don't count repeated codes
      if( !(irmp_data->flags & IRMP_FLAG_REPETITION) )
      {
         // increase counter
         reset_ctr++;
         // if reset was pressed 3 times with not being interrupted by another key
         if( reset_ctr >= 3 )
         {
            flags.press_reset_button = 1;
            // set to 0 for next reset use
            reset_ctr = 0;
         }
      }
   }
   else // another code than reset code was received
   {
      // reset counter
      reset_ctr = 0;
   }
}

/**
  * @brief  Processes IRSND data stored in the FIFO.
  */
void IRSND_ProcessData(void)
{
   IRMP_DATA irmp_data;

   // if IRSND is ready to transmit a command
   if( !irsnd_is_busy() )
   {
      // if there's a command to be sent
      if( FIFO_Read(&irsnd_fifo, (fifo_entry_t*)&irmp_data) )
      {
         irsnd_send_data(&irmp_data, false);
      }
   }
}

/**
  * @brief  Callback from SWRTC that is called every second and stores the
  *         current time in the backup registers.
  */
void FullSecond(void)
{
   HAL_RTCEx_BKUPWrite(&RtcHandle, BACKUP_REG_SECOND, SWRTC_GetSeconds());
}

/**
  * @brief  Sets a flag that is managed inside RTC_ManageInterrupts().
  */
void Alarm1(uint8_t idx)
{
   flags.alarm_a_occurred = SET;
}

/**
  * @brief  Sets a flag that is managed inside RTC_ManageInterrupts().
  */
void Alarm2(uint8_t idx)
{
   flags.alarm_b_occurred = SET;
}

/**
  * @brief  Handles RTC interrupt flags.
  */
void RTC_HandleInterruptFlags(void)
{
   // if a wakeup interrupt occurred the buttons must be processed
   if(flags.wakeup_occurred != RESET)
   {
      ProcessButtons();
      flags.wakeup_occurred = RESET;
   }

   // if an alarm A interrupt occurred the PC must be started if the power
   // supply allows it (no black-out)
   if(flags.alarm_a_occurred != RESET)
   {
      // if supply is fine and PC is not running
      if(DEB_GetKeyState(DEB_USB_SENSE) && !DEB_GetKeyState(DEB_PSU_SENSE))
      {
          flags.press_power_button = 1;
      }

      // if supply is fine and PC is already running
      if(DEB_GetKeyState(DEB_USB_SENSE) && DEB_GetKeyState(DEB_PSU_SENSE))
      {
         flags.alarm_a_occurred = RESET;
      }
   }

   // if an alarm B interrupt occurred it's too late to start the PC, so delete
   // the flags (wakeup timespan has passed)
   if(flags.alarm_b_occurred != RESET)
   {
      flags.alarm_a_occurred = RESET;
      flags.alarm_b_occurred = RESET;
   }
}

/**
  * @brief  Recovers settings after startup (power-up or reset).
  */
void InitHidirtConfig(void)
{
   // recover last stored time if reset occurred
   if(HAL_RTCEx_BKUPRead(&RtcHandle, BACKUP_REG_RESET) == BACKUP_INIT_PATTERN)
   {
      SWRTC_SetSeconds(HAL_RTCEx_BKUPRead(&RtcHandle, BACKUP_REG_SECOND));
   }

   EEPROM_ReadBytes(ADDRESS_clock_correction,
                    &hidirt_data.clock_correction,
                    sizeof(hidirt_data.clock_correction));
   SWRTC_SetDeviation(hidirt_data.clock_correction);

   EEPROM_ReadBytes(ADDRESS_control_pc_enable,
                    &hidirt_data.control_pc_enable,
                    sizeof(hidirt_data.control_pc_enable));

   EEPROM_ReadBytes(ADDRESS_forward_ir_enable,
                    &hidirt_data.forward_ir_enable,
                    sizeof(hidirt_data.forward_ir_enable));

   EEPROM_ReadBytes(ADDRESS_irmp_power_on,
                    &hidirt_data.irmp_power_on,
                    sizeof(hidirt_data.irmp_power_on));

   EEPROM_ReadBytes(ADDRESS_irmp_power_off,
                    &hidirt_data.irmp_power_off,
                    sizeof(hidirt_data.irmp_power_off));

   EEPROM_ReadBytes(ADDRESS_irmp_reset,
                    &hidirt_data.irmp_reset,
                    sizeof(hidirt_data.irmp_reset));

   EEPROM_ReadBytes(ADDRESS_min_ir_repeats,
                    &hidirt_data.min_ir_repeats,
                    sizeof(hidirt_data.min_ir_repeats));

   EEPROM_ReadBytes(ADDRESS_wakeup_time_span,
                    &hidirt_data.wakeup_time_span,
                    sizeof(hidirt_data.wakeup_time_span));

   // either restore the old wakeup time or wake the PC (in 3 seconds) to
   // retrieve a new one
   if(HAL_RTCEx_BKUPRead(&RtcHandle, BACKUP_REG_RESET) == BACKUP_INIT_PATTERN)
   {
      SWRTC_SetAlarmTime(0, HAL_RTCEx_BKUPRead(&RtcHandle, BACKUP_REG_ALARM));
   }
   else
   {
      SWRTC_SetAlarmTime(0, 3);
   }
   // set the time-span accordingly
   SWRTC_SetAlarmTime(1, SWRTC_GetAlarmTime(0) + hidirt_data.wakeup_time_span*60);
}

/**
  * @brief  Allows other modules to read the main data struct holding the
  *         configuration.
  * @param  *config holds the configuration afterwards.
  */
void GetHidirtConfig(hidirt_data_t* config)
{
   __disable_irq();
   memcpy(config, &hidirt_data, sizeof(*config));
   __enable_irq();
}

void hidirt_init(void)
{
   /* Initialize HAL peripherals */
   HAL_MspInitCustom();

#if defined(STM32F103xB)
   /* Unlock the Flash Program Erase controller */
   HAL_FLASH_Unlock();

   /* EEPROM Init */
   EE_Init();

   /* Lock the Flash Program Erase controller */
   HAL_FLASH_Lock();
#endif

   /* Initialize config data */
   InitHidirtConfig();

   /* Configure the unused and used GPIOs */
   GPIO_ConfigAsAnalog();
   GPIO_Configuration();

   /* Init default state for debouncing */
   DEB_Init(DEB_USB_SENSE);

   /* Configure RTC alarms and wakeup for calling debounce function */
   SWRTC_RegisterAlarmCallback(0, Alarm1);
   SWRTC_RegisterAlarmCallback(1, Alarm2);
   SWRTC_RegisterFullSecondCallback(FullSecond);

   /* Initialize infrared interface */
   irmp_init();
   irsnd_init();

   /* Enable interrupts */
   __enable_irq();

#if defined(USE_BACKUP_SUPPLY)
   /* Enable ultra low power mode */
   HAL_PWREx_EnableUltraLowPower();

   /* Enable the fast wake up from Ultra low power mode */
   HAL_PWREx_EnableFastWakeUp();
#endif

   /* Stop IWDG when CPU is halted */
   __HAL_DBGMCU_FREEZE_IWDG();
   /* Enable Independent WatchDoG */
   if(HAL_IWDG_Start(&IwdgHandle) != HAL_OK)
   {
     Error_Handler();
   }
}

void hidirt(void)
{
   IRMP_DATA irmp_data;

   /* Determine whether host is running and watchdog is enabled */
   if(DEB_GetKeyState(DEB_PSU_SENSE) && hidirt_data.watchdog_enable == TRUE)
   {
      /* Check if watchdog reset flag was sent from host */
      if(hidirt_data.watchdog_reset == TRUE)
      {
         /* Reload IWDG counter */
         HAL_IWDG_Refresh(&IwdgHandle);
         /* Reset flag */
         hidirt_data.watchdog_reset = FALSE;
      }
   }
   /* Host is NOT running or watchdog (keepalive) is disabled */
   else
   {
      /* Reload IWDG counter */
      HAL_IWDG_Refresh(&IwdgHandle);
   }

   /* Check if new IR code was received */
   if(irmp_get_data(&irmp_data))
   {
      /* IR signal decoded, process it */
      IRMP_ProcessData(&irmp_data);
   }

   /* Process IRSND data */
   IRSND_ProcessData();

   /* Handle periodic tasks (when a RTC wakeup interrupt or alarm occurs) */
   RTC_HandleInterruptFlags();

   /* Update values retrieved via USB to work with it */
   GetHidirtShadowConfig(&hidirt_data);

#if defined(USE_BACKUP_SUPPLY)
   /* Enter and stay in standby mode as long as USB voltage is not present */
   while(!DEB_GetKeyState(DEB_USB_SENSE))
   {
      /* Prepare clocks and peripherals to enter stop mode */
      PrepareStopMode();

      /* Clocks and peripherals are prepared now, so only execute the
       * absolutely necessary steps. That are:
       * - IWDG refresh
       * - SWRTC service (in ISR)
       * - debounce service (in ISR) */
      while(!DEB_GetKeyState(DEB_USB_SENSE))
      {
         /* Reload IWDG counter */
         HAL_IWDG_Refresh(&IwdgHandle);

#  ifndef DEBUG   // for debugging
         /* Clear wake-up flag */
         __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

         /* Enter Stop Mode. Wake up through RTC wakeup */
         HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
#  endif
      }

      /* Restore clocks and peripherals after leaving stop mode */
      LeaveStopMode();
   }
#endif
}
