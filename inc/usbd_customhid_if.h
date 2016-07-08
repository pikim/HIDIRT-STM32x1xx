/**
  ******************************************************************************
  * @file    USB_Device/CustomHID_Standalone/Inc/usbd_customhid_if.h
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    09-October-2015
  * @brief   Header for usbd_customhid_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CUSTOMHID_IF_H
#define __USBD_CUSTOMHID_IF_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
#include "application.h"

/* Exported types ------------------------------------------------------------*/
typedef enum _CUSTOMHID_REPORT_ID
{
  REP_ID_IR_CODE_INTERRUPT       = 1,
  REP_ID_GET_FIRMWARE_VERSION    = 0x10,
  REP_ID_CONTROL_PC_ENABLE       = 0x11,
  REP_ID_FORWARD_IR_ENABLE       = 0x12,
  REP_ID_POWER_ON_IR_CODE        = 0x13,
  REP_ID_POWER_OFF_IR_CODE       = 0x14,
  REP_ID_RESET_IR_CODE           = 0x15,
  REP_ID_MINIMUM_REPEATS         = 0x16,
  REP_ID_CURRENT_TIME            = 0x17,
  REP_ID_CLOCK_CORRECTION        = 0x18,
  REP_ID_WAKEUP_TIME             = 0x19,
  REP_ID_WAKEUP_TIME_SPAN        = 0x1A,
  REP_ID_REQUEST_BOOTLOADER      = 0x50,
  REP_ID_WATCHDOG_ENABLE         = 0x51,
  REP_ID_WATCHDOG_RESET          = 0x52
} CUSTOMHID_REPORT_ID;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops;
extern void GetHidirtShadowConfig(hidirt_data_t *config);

#endif /* __USBD_CUSTOMHID_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
