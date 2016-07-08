/**
 ******************************************************************************
 * @file    USB_Device/CustomHID_Standalone/Src/usbd_customhid_if.c
 * @author  MCD Application Team
 * @version V1.4.0
 * @date    09-October-2015
 * @brief   USB Device Custom HID interface file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "swrtc.h"
#include "fifo.h"
#include "irmp.h"
#include "usbd_customhid_if.h"
#include "application.h"
#include "global_variables.h"
#include "stm32_hal_msp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int8_t CustomHID_Init        (void);
static int8_t CustomHID_DeInit      (void);
static int8_t CustomHID_OutEvent    (uint8_t event_idx, uint8_t* buffer);
static int8_t CustomHID_SetFeature  (uint8_t event_idx, uint8_t* buffer);
static int8_t CustomHID_GetFeature  (uint8_t event_idx, uint8_t* buffer, uint16_t* length);
static uint16_t CustomHID_FeatureReportLength(uint8_t event_idx);

/* Private variables ---------------------------------------------------------*/
static char FirmwareVersion[USBD_CUSTOMHID_INREPORT_BUF_SIZE-1] = "v0.31";
static hidirt_data_t hidirt_data_shadow = {0};

__ALIGN_BEGIN static uint8_t CustomHID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
   0x06, 0x00, 0xff,                   // USAGE_PAGE (Vendor Defined Page 1)
   0x09, 0x01,                         // USAGE (Vendor Usage 1)
   0xa1, 0x01,                         // COLLECTION (Application)
   0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
   0x26, 0xff, 0x00,                   //   LOGICAL_MAXIMUM (255)
   0x75, 0x08,                         //   REPORT_SIZE (8)

   0x95, 0x01,                         //   REPORT_COUNT (1)
   0x85, REP_ID_CONTROL_PC_ENABLE,     //   REPORT_ID (0x11)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_FORWARD_IR_ENABLE,     //   REPORT_ID (0x12)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_MINIMUM_REPEATS,       //   REPORT_ID (0x16)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_WAKEUP_TIME_SPAN,      //   REPORT_ID (0x1A)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_REQUEST_BOOTLOADER,    //   REPORT_ID (0x50)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_WATCHDOG_ENABLE,       //   REPORT_ID (0x51)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_WATCHDOG_RESET,        //   REPORT_ID (0x52)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)

   0x95, 0x04,                         //   REPORT_COUNT (4)
   0x85, REP_ID_CLOCK_CORRECTION,      //   REPORT_ID (0x18)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_WAKEUP_TIME,           //   REPORT_ID (0x19)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)

   0x95, 0x06,                         //   REPORT_COUNT (6)
   0x85, REP_ID_IR_CODE_INTERRUPT,     //   REPORT_ID (1)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0x81, 0x02,                         //   INPUT (Data,Var,Abs)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0x91, 0x02,                         //   OUTPUT (Data,Var,Abs)
   0x85, REP_ID_POWER_ON_IR_CODE,      //   REPORT_ID (0x13)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_POWER_OFF_IR_CODE,     //   REPORT_ID (0x14)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_RESET_IR_CODE,         //   REPORT_ID (0x15)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0x85, REP_ID_CURRENT_TIME,          //   REPORT_ID (0x17)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)

   0x95, 0x0f,                         //   REPORT_COUNT (15)
   0x85, REP_ID_GET_FIRMWARE_VERSION,  //   REPORT_ID (0x10)
   0x09, 0x01,                         //   USAGE (Vendor Usage 1)
   0xb1, 0x02,                         //   FEATURE (Data,Var,Abs)
   0xc0                                // END_COLLECTION
};

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops =
{
   CustomHID_ReportDesc,
   CustomHID_Init,
   CustomHID_DeInit,
   CustomHID_OutEvent,
   CustomHID_SetFeature,
   CustomHID_GetFeature
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  CustomHID_Init
 *         Initializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CustomHID_Init(void)
{
   /*
   Add your initialization code here
   */
   return (0);
}

/**
 * @brief  CustomHID_DeInit
 *         DeInitializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CustomHID_DeInit(void)
{
   /*
   Add your deinitialization code here
   */
   return (0);
}

/**
 * @brief  CustomHID_OutEvent
 *         Manage the CUSTOM HID class Out Event
 *         Host -> Device
 * @param  event_idx: Report Number
 * @param  buffer: Received Data
 * @retval USBD_OK
 */
static int8_t CustomHID_OutEvent(uint8_t event_idx, uint8_t* buffer)
{
   switch(event_idx)
   {
   case REP_ID_CONTROL_PC_ENABLE:
      // write command to FIFO from where it will be sent later
      FIFO_Write(&irsnd_fifo, (fifo_entry_t*)buffer);
      break;

   default: /* Report does not exist */
      break;
   }

   return (USBD_OK);
}

/**
 * @brief  CustomHID_SetFeature
 *         Manage the CUSTOM HID SetFeature request.
 *         Host -> Device
 * @param  event_idx: Report Number
 * @param  buffer: Received Data
 * @retval USBD_OK
 */
static int8_t CustomHID_SetFeature(uint8_t event_idx, uint8_t* buffer)
{
   swrtc_time_t   time;
   uint32_t       alarm;

   switch(event_idx)
   {
   case REP_ID_CONTROL_PC_ENABLE:
      hidirt_data_shadow.control_pc_enable = buffer[0];
      hidirt_data_shadow.data_update_pending = REP_ID_CONTROL_PC_ENABLE;
      break;

   case REP_ID_FORWARD_IR_ENABLE:
      hidirt_data_shadow.forward_ir_enable = buffer[0];
      hidirt_data_shadow.data_update_pending = REP_ID_FORWARD_IR_ENABLE;
      break;

   case REP_ID_POWER_ON_IR_CODE:
      memcpy(&hidirt_data_shadow.irmp_power_on,
             &buffer[0],
             sizeof(hidirt_data_shadow.irmp_power_on));
      hidirt_data_shadow.data_update_pending = REP_ID_POWER_ON_IR_CODE;
      break;

   case REP_ID_POWER_OFF_IR_CODE:
      memcpy(&hidirt_data_shadow.irmp_power_off,
             &buffer[0],
             sizeof(hidirt_data_shadow.irmp_power_off));
      hidirt_data_shadow.data_update_pending = REP_ID_POWER_OFF_IR_CODE;
      break;

   case REP_ID_RESET_IR_CODE:
      memcpy(&hidirt_data_shadow.irmp_reset,
             &buffer[0],
             sizeof(hidirt_data_shadow.irmp_reset));
      hidirt_data_shadow.data_update_pending = REP_ID_RESET_IR_CODE;
      break;

   case REP_ID_MINIMUM_REPEATS:
      memcpy(&hidirt_data_shadow.min_ir_repeats,
             &buffer[0],
             sizeof(hidirt_data_shadow.min_ir_repeats));
      hidirt_data_shadow.data_update_pending = REP_ID_MINIMUM_REPEATS;
      break;

   case REP_ID_CURRENT_TIME:
      memcpy(&time,
             &buffer[0],
             sizeof(time));
      SWRTC_SetTime(time);
      HAL_RTCEx_BKUPWrite(&RtcHandle, BACKUP_REG_RESET, BACKUP_INIT_PATTERN);
      break;

   case REP_ID_CLOCK_CORRECTION:
      memcpy(&hidirt_data_shadow.clock_correction,
             &buffer[0],
             sizeof(hidirt_data_shadow.clock_correction));
      hidirt_data_shadow.data_update_pending = REP_ID_CLOCK_CORRECTION;
      break;

   case REP_ID_WAKEUP_TIME:
      memcpy(&alarm,
             &buffer[0],
             sizeof(alarm));
      SWRTC_SetAlarmTime(0, alarm);
      HAL_RTCEx_BKUPWrite(&RtcHandle, BACKUP_REG_ALARM, alarm);
      hidirt_data_shadow.data_update_pending = REP_ID_WAKEUP_TIME;
      break;

   case REP_ID_WAKEUP_TIME_SPAN:
      memcpy(&hidirt_data_shadow.wakeup_time_span,
             &buffer[0],
             sizeof(hidirt_data_shadow.wakeup_time_span));
      hidirt_data_shadow.data_update_pending = REP_ID_WAKEUP_TIME_SPAN;
      break;

   case REP_ID_REQUEST_BOOTLOADER:
      if(buffer[0] == 0x5a)
         hidirt_data_shadow.data_update_pending = REP_ID_REQUEST_BOOTLOADER;
      break;

   case REP_ID_WATCHDOG_ENABLE:
      hidirt_data_shadow.watchdog_enable = buffer[0];
      hidirt_data_shadow.data_update_pending = REP_ID_WATCHDOG_ENABLE;
      break;

   case REP_ID_WATCHDOG_RESET:
      hidirt_data_shadow.watchdog_reset = buffer[0];
      hidirt_data_shadow.data_update_pending = REP_ID_WATCHDOG_RESET;
      break;

   default: /* Report does not exist */
      break;
   }

   return (USBD_OK);
}

/**
 * @brief  CustomHID_GetFeature
 *         Manage the CUSTOM HID GetFeature request.
 *         Device -> Host
 * @param  event_idx: Requested Report Number
 * @param  buffer: Data to transmit including ReportID
 * @param  length: Length of the buffer
 * @retval length: Number of bytes to send
 * @retval USBD_OK
 */
static int8_t CustomHID_GetFeature(uint8_t event_idx, uint8_t* buffer, uint16_t* length)
{
   swrtc_time_t   time;
   uint32_t       alarm;

   // clear transmission data array
   memset(buffer, 0x00, *length);

   // retrieve currently valid config
   GetHidirtConfig(&hidirt_data_shadow);

   switch(event_idx)
   {
   case REP_ID_GET_FIRMWARE_VERSION:
      memcpy(&buffer[0],
             &FirmwareVersion[0],
             sizeof(FirmwareVersion));
      break;

   case REP_ID_CONTROL_PC_ENABLE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.control_pc_enable,
             sizeof(hidirt_data_shadow.control_pc_enable));
      break;

   case REP_ID_FORWARD_IR_ENABLE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.forward_ir_enable,
             sizeof(hidirt_data_shadow.forward_ir_enable));
      break;

   case REP_ID_POWER_ON_IR_CODE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.irmp_power_on,
             sizeof(hidirt_data_shadow.irmp_power_on));
      break;

   case REP_ID_POWER_OFF_IR_CODE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.irmp_power_off,
             sizeof(hidirt_data_shadow.irmp_power_off));
      break;

   case REP_ID_RESET_IR_CODE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.irmp_reset,
             sizeof(hidirt_data_shadow.irmp_reset));
      break;

   case REP_ID_MINIMUM_REPEATS:
      memcpy(&buffer[0],
             &hidirt_data_shadow.min_ir_repeats,
             sizeof(hidirt_data_shadow.min_ir_repeats));
      break;

   case REP_ID_CURRENT_TIME:
      time = SWRTC_GetTime();
      memcpy(&buffer[0],
             &time,
             sizeof(time));
      break;

   case REP_ID_CLOCK_CORRECTION:
      memcpy(&buffer[0],
             &hidirt_data_shadow.clock_correction,
             sizeof(hidirt_data_shadow.clock_correction));
      break;

   case REP_ID_WAKEUP_TIME:
      alarm = SWRTC_GetAlarmTime(0);
      memcpy(&buffer[0],
             &alarm,
             sizeof(alarm));
      break;

   case REP_ID_WAKEUP_TIME_SPAN:
      memcpy(&buffer[0],
             &hidirt_data_shadow.wakeup_time_span,
             sizeof(hidirt_data_shadow.wakeup_time_span));
      break;

   case REP_ID_WATCHDOG_ENABLE:
      memcpy(&buffer[0],
             &hidirt_data_shadow.watchdog_enable,
             sizeof(hidirt_data_shadow.watchdog_enable));
      break;

   default: /* Report does not exist */
      return (USBD_FAIL);
      break;
   }

   *length = CustomHID_FeatureReportLength(event_idx);
   return (USBD_OK);
}

/**
 * @brief  CustomHID_FeatureReportLength
 *         Get length of a specific feature report (data without ReportID).
 * @param  event_idx: Requested Report Number
 * @retval length: Number of bytes of the according report.
 */
uint16_t CustomHID_FeatureReportLength(uint8_t event_idx)
{
   uint16_t length = 0;

   // Get length of corresponding data
   switch (event_idx)
   {
   case REP_ID_GET_FIRMWARE_VERSION:
      length = sizeof(FirmwareVersion);
      break;

   case REP_ID_CONTROL_PC_ENABLE:
   case REP_ID_FORWARD_IR_ENABLE:
   case REP_ID_WATCHDOG_ENABLE:
   case REP_ID_WATCHDOG_RESET:
      length = sizeof(bool);
      break;

   case REP_ID_POWER_ON_IR_CODE:
   case REP_ID_POWER_OFF_IR_CODE:
   case REP_ID_RESET_IR_CODE:
      length = sizeof(IRMP_DATA);
      break;

   case REP_ID_MINIMUM_REPEATS:
   case REP_ID_WAKEUP_TIME_SPAN:
   case REP_ID_REQUEST_BOOTLOADER:
      length = sizeof(uint8_t);
      break;

   case REP_ID_CURRENT_TIME:
      length = sizeof(swrtc_time_t);
      break;

   case REP_ID_CLOCK_CORRECTION:
   case REP_ID_WAKEUP_TIME:
      length = sizeof(hidirt_data_shadow.clock_correction);
      break;

   default:
      break;
   }

   return length;
}

/**
  * @brief  Allows main to read the last configuration parameters received via
  *         USB and also stores them into the EEPROM.
  * @param  *config holds the updated configuration afterwards.
  */
void GetHidirtShadowConfig(hidirt_data_t *config)
{
   uint32_t wut;

   __disable_irq();
   switch(hidirt_data_shadow.data_update_pending)    // switch data to update
   {
   case REP_ID_CONTROL_PC_ENABLE:
      memcpy(&config->control_pc_enable,
            &hidirt_data_shadow.control_pc_enable,
            sizeof(config->control_pc_enable));
      EEPROM_WriteBytes(ADDRESS_control_pc_enable,
            &hidirt_data_shadow.control_pc_enable,
            sizeof(hidirt_data_shadow.control_pc_enable));
      break;

   case REP_ID_FORWARD_IR_ENABLE:
      memcpy(&config->forward_ir_enable,
            &hidirt_data_shadow.forward_ir_enable,
            sizeof(config->forward_ir_enable));
      EEPROM_WriteBytes(ADDRESS_forward_ir_enable,
            &hidirt_data_shadow.forward_ir_enable,
            sizeof(hidirt_data_shadow.forward_ir_enable));
      break;

   case REP_ID_POWER_ON_IR_CODE:
      memcpy(&config->irmp_power_on,
            &hidirt_data_shadow.irmp_power_on,
            sizeof(config->irmp_power_on));
      EEPROM_WriteBytes(ADDRESS_irmp_power_on,
            &hidirt_data_shadow.irmp_power_on,
            sizeof(hidirt_data_shadow.irmp_power_on));
      break;

   case REP_ID_POWER_OFF_IR_CODE:
      memcpy(&config->irmp_power_off,
            &hidirt_data_shadow.irmp_power_off,
            sizeof(config->irmp_power_off));
      EEPROM_WriteBytes(ADDRESS_irmp_power_off,
            &hidirt_data_shadow.irmp_power_off,
            sizeof(hidirt_data_shadow.irmp_power_off));
      break;

   case REP_ID_RESET_IR_CODE:
      memcpy(&config->irmp_reset,
            &hidirt_data_shadow.irmp_reset,
            sizeof(config->irmp_reset));
      EEPROM_WriteBytes(ADDRESS_irmp_reset,
            &hidirt_data_shadow.irmp_reset,
            sizeof(hidirt_data_shadow.irmp_reset));
      break;

   case REP_ID_MINIMUM_REPEATS:
      memcpy(&config->min_ir_repeats,
            &hidirt_data_shadow.min_ir_repeats,
            sizeof(config->min_ir_repeats));
      EEPROM_WriteBytes(ADDRESS_min_ir_repeats,
            &hidirt_data_shadow.min_ir_repeats,
            sizeof(hidirt_data_shadow.min_ir_repeats));
      break;

   case REP_ID_CLOCK_CORRECTION:
      memcpy(&config->clock_correction,
            &hidirt_data_shadow.clock_correction,
            sizeof(config->clock_correction));
      EEPROM_WriteBytes(ADDRESS_clock_correction,
            &hidirt_data_shadow.clock_correction,
            sizeof(hidirt_data_shadow.clock_correction));
      SWRTC_SetDeviation(config->clock_correction);
      break;

   case REP_ID_WAKEUP_TIME:
      wut = SWRTC_GetAlarmTime(0);
      SWRTC_SetAlarmTime(1, wut+config->wakeup_time_span*60);
      break;

   case REP_ID_WAKEUP_TIME_SPAN:
      memcpy(&config->wakeup_time_span,
            &hidirt_data_shadow.wakeup_time_span,
            sizeof(config->wakeup_time_span));
      EEPROM_WriteBytes(ADDRESS_wakeup_time_span,
            &hidirt_data_shadow.wakeup_time_span,
            sizeof(hidirt_data_shadow.wakeup_time_span));
      wut = SWRTC_GetAlarmTime(0);
      SWRTC_SetAlarmTime(1, wut+config->wakeup_time_span*60);
      break;

   case REP_ID_REQUEST_BOOTLOADER:
      HAL_RTCEx_BKUPWrite(&RtcHandle, BACKUP_REG_BOOTLOADER, 0xABADC0DE);
      while(1);
      break;

   case REP_ID_WATCHDOG_ENABLE:
      memcpy(&config->watchdog_enable,
            &hidirt_data_shadow.watchdog_enable,
            sizeof(config->watchdog_enable));
      break;

   case REP_ID_WATCHDOG_RESET:
      memcpy(&config->watchdog_reset,
            &hidirt_data_shadow.watchdog_reset,
            sizeof(config->watchdog_reset));
      break;

   default:
      break;
   }
   hidirt_data_shadow.data_update_pending = 0;
   __enable_irq();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
