/**
  ******************************************************************************
  * @file    USB_Device/CustomHID_Standalone/Src/stm32x1xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    09-October-2015
  * @brief   HAL MSP module.
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
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
#else
  #error Device not specified.
#endif
#include "debounce.h"
#include "swrtc.h"
#include "irmp.h"
#include "irsnd.h"
#include "stm32_hal_msp.h"
#include "configuration.h"
#include "application.h"
#include "main.h"
#include "global_variables.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  HAL MSP Initialization
  *         This function configures the hardware resources used in this
  *         application.
  * @param  None
  * @retval None
  */
void HAL_MspInitCustom(void)
{
   IwdgHandle.Instance = IWDG;
   if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
   {
      /* Initialization Error */
      Error_Handler();
   }

   // Don't initialize RTC before having called USBD_Start(), otherwise device
   // won't start properly and hang itself up. No idea what causes this.
   RtcHandle.Instance = RTC;
   if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
   {
      /* Initialization Error */
      Error_Handler();
   }

   TimHandle.Instance = IRMP_IRSND_TIMER;
   if(HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
   {
      /* Initialization Error */
      Error_Handler();
   }
}

/**
  * @brief  IWDG MSP Initialization
  *         This function configures the hardware resources used in this
  *         application.
  * @param  hiwdg: IWDG handle pointer
  * @retval None
  */
void HAL_IWDG_MspInit(IWDG_HandleTypeDef *hiwdg)
{
   /* Initialize IWDG */
   hiwdg->Init.Prescaler = IWDG_PRESCALER_32;
   hiwdg->Init.Reload    = 37000/32*IWDG_TIMEOUT_IN_SECONDS; // Xsec/(1/(37000Hz/32presc))

   /* Start IWDG */
//   HAL_IWDG_Start(hiwdg); // IWDG is started at the end of hidirt_init

   /* Refresh IWDG: reload counter */
   HAL_IWDG_Refresh(hiwdg);
}

/**
  * @brief  RTC MSP Initialization
  *         This function configures the hardware resources used in this example
  * @param  hrtc: RTC handle pointer
  *
  * @note   Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select
  *         the RTC clock source; in this case the Backup domain will be reset in
  *         order to modify the RTC Clock source, as consequence RTC registers (including
  *         the backup registers) and RCC_BDCR register are set to their reset values.
  *
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
   RCC_OscInitTypeDef        RCC_OscInitStruct;
   RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

   /* RCC peripheral clock and backup access enable */
   __HAL_RCC_PWR_CLK_ENABLE();
   HAL_PWR_EnableBkUpAccess();

#if defined(BACKUP_INIT_PATTERN) && defined(BACKUP_INIT_REGISTER)
   /* On cold start the peripheral has to be configured */
   if(HAL_RTCEx_BKUPRead(hrtc, BACKUP_INIT_REGISTER) != BACKUP_INIT_PATTERN)
#endif
   {
      /* Configure RTC prescaler and RTC data registers */
      hrtc->Init.AsynchPrediv   = RTC_ASYNCH_PREDIV;
#if defined(STM32L151xB)
      hrtc->Init.SynchPrediv    = RTC_SYNCH_PREDIV;
      hrtc->Init.HourFormat     = RTC_HOURFORMAT_24;
      hrtc->Init.OutPut         = RTC_OUTPUT_DISABLE;
      hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
      hrtc->Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
#endif

   #ifdef RTC_CLOCK_SOURCE_LSE
      /* Configure LSE as RTC clock source */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
      RCC_OscInitStruct.LSEState = RCC_LSE_ON;
      RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
      if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
         Error_Handler();
      }

      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
      PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
      if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
      {
         Error_Handler();
      }
   #elif defined(RTC_CLOCK_SOURCE_LSI)
      /* Configure LSI as RTC clock source */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
      RCC_OscInitStruct.LSIState = RCC_LSI_ON;
      RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
      if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
         Error_Handler();
      }

      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
      PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
      if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
      {
         Error_Handler();
      }
   #else
   #error Please select the RTC Clock source inside the main.h file
   #endif /* RTC_CLOCK_SOURCE_LSE/RTC_CLOCK_SOURCE_LSI */

      /* Enable RTC Clock */
      __HAL_RCC_RTC_ENABLE();

      /* Special cold start actions can go here */

#if defined(STM32F103xB)
      /* Enable RTC second interrupt */
      HAL_RTCEx_SetSecond_IT(hrtc);
#elif defined(STM32L151xB)
      /* Configure RTC wakeup (~2 times each second (CK_SPRE/2)) */
      HAL_RTCEx_SetWakeUpTimer_IT(hrtc, 1024-1, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
#else
#error Device not specified.
#endif
   }
#if defined(BACKUP_INIT_PATTERN) && defined(BACKUP_INIT_REGISTER)
   /* On warm start the peripheral is already configured */
   else
   {
      /* Special warm start actions can go here */
   }
#endif

#if defined(STM32F103xB)
   /* Clear the SECOND interrupt pending flag */
   __HAL_RTC_SECOND_CLEAR_FLAG(hrtc, RTC_FLAG_SEC);

   /* Set the RTC NVIC priority */
   HAL_NVIC_SetPriority(RTC_IRQn, 2, 0);

   /* Enable the RTC global interrupt */
   HAL_NVIC_EnableIRQ(RTC_IRQn);
#elif defined(STM32L151xB)
   /* Clear the WAKEUPTIMER interrupt pending flag and the according EXTI flag */
   __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(hrtc, RTC_FLAG_WUTF);
   __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

   /* Set the RTC wakeup NVIC priority */
   HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 2, 0);

   /* Enable the RTC wakeup global interrupt */
   HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
#else
#error Device not specified.
#endif
}

/**
  * @brief  RTC MSP De-Initialization
  *         This function frees the hardware resources used in this example:
  *         - Disable the peripheral's clock
  * @param  hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
   /* Disable the RTC alarm global interrupt */
   HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);

   /* Disable RTC Clock */
   __HAL_RCC_RTC_DISABLE();
}

/**
  * @brief  TIM MSP Initialization
  *         This function configures the hardware resources used in this
  *         application.
  * @param  htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
//   if(htim->Instance == IRMP_IRSND_TIMER)
   {
      /* TIMx peripheral clock enable */
      IRMP_IRSND_TIMER_CLK_EN();

      /* Configure TIMx */
      htim->Init.Period         = (HAL_RCC_GetPCLK1Freq()/F_INTERRUPTS)-1;
      htim->Init.Prescaler      = 0;
      htim->Init.ClockDivision  = 0;
      htim->Init.CounterMode    = TIM_COUNTERMODE_UP;

      /* TIMx interrupt enable and start */
      if(HAL_TIM_Base_Start_IT(htim) != HAL_OK)
      {
        /* Starting Error */
        Error_Handler();
      }

      /* Set the TIMx NVIC priority */
      HAL_NVIC_SetPriority(IRMP_IRSND_TIMER_IRQ, 0, 1);

      /* Enable the TIMx global interrupt */
      HAL_NVIC_EnableIRQ(IRMP_IRSND_TIMER_IRQ);
   }
}

/**
  * @brief  TIM MSP De-Initialization
  *         This function frees the hardware resources used in this example:
  *         - Disable the peripheral's clock
  * @param  htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
   /* Disable the TIMx global interrupt */
   HAL_NVIC_DisableIRQ(IRMP_IRSND_TIMER_IRQ);

   /* TIMx peripheral clock disable */
   IRMP_IRSND_TIMER_CLK_DIS();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
