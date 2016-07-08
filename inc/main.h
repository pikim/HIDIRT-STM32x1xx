/**
  ******************************************************************************
  * @file    USB_Device/CustomHID_Standalone/Inc/main.h
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    09-October-2015
  * @brief   Header for main.c module
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
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
//  #include "stm3210e_eval.h"
//  #include "stm32f1xx_hal_pcd.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
//  #include "stm32l152d_eval.h"
#else
  #error Device not specified.
#endif
//#include "usbd_core.h"
//#include "usbd_desc.h"
//#include "usbd_customhid.h"
//#include "usbd_customhid_if.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void Error_Handler(void);
extern void SystemClock_Config(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
