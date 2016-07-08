/**
 * @file       configuration.c
 * @brief      Module that contains the configuration functionality.
 */

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F103xB)
  #include "stm32f1xx_hal.h"
#elif defined(STM32L151xB)
  #include "stm32l1xx_hal.h"
#else
  #error Device not specified.
#endif
#include "configuration.h"
#include "debounce.h"
#include "swrtc.h"
#include "irmp.h"
#include "irsnd.h"
#include "stm32_hal_msp.h"
#include "application.h"
#include "main.h"
#include "global_variables.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : GPIO_ConfigAsAnalog
* Description    : Configures the different GPIO ports as analog to reduce
*                  current consumption.
*******************************************************************************/
void GPIO_ConfigAsAnalog(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Configure all GPIO as analog to reduce current consumption on non used IOs */
   /* Enable GPIOs clock */
#if defined(STM32F103xB)
   RCC->AHBENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN |
                   RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN);
#elif defined(STM32L151xB)
   RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
                   RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOHEN);
#else
#error Device not specified.
#endif

   GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
   GPIO_InitStructure.Speed = GPIO_SPEED;
   GPIO_InitStructure.Pull = GPIO_NOPULL;
   GPIO_InitStructure.Pin = GPIO_PIN_All;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
   HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
   HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
#if defined(STM32L151xB)
//   GPIO_Init(GPIOF, &GPIO_InitStructure);
//   GPIO_Init(GPIOG, &GPIO_InitStructure);
   HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
#endif

   // be careful to not disable SWD-pins (hangs microcontroller)
   // also do not disable USB-pins (breaks USB communication)
   GPIO_InitStructure.Pin = GPIO_PIN_All & ~GPIO_PIN_11 & ~GPIO_PIN_12 &
                                           ~GPIO_PIN_13 & ~GPIO_PIN_14;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

   GPIO_InitStructure.Pin = GPIO_PIN_All & ~GPIO_PIN_14 & ~GPIO_PIN_15;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

   /* Disable GPIOs clock */
#if defined(STM32F103xB)
   RCC->AHBENR &= ~(/*RCC_APB2ENR_IOPAEN |*/ RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN |
                    RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN);
#elif defined(STM32L151xB)
   RCC->AHBENR &= ~(/*RCC_AHBENR_GPIOAEN |*/ RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
                    RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOHEN);
#else
#error Device not specified.
#endif
}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
*******************************************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* init common characteristics for outputs */
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  /* init pin for power switch output and set default state */
  POWER_PORT_CLK_EN();
  GPIO_InitStructure.Pin = POWER_BIT;
  HAL_GPIO_Init(POWER_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(POWER_PORT, POWER_BIT, POWER_RELEASED);

  /* init pin for reset switch output and set default state */
  RESET_PORT_CLK_EN();
  GPIO_InitStructure.Pin = RESET_BIT;
  HAL_GPIO_Init(RESET_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(RESET_PORT, RESET_BIT, RESET_RELEASED);

  /* init pin for ir enable output and set default state */
  IR_ENABLE_PORT_CLK_EN();
  GPIO_InitStructure.Pin = IR_ENABLE_BIT;
  HAL_GPIO_Init(IR_ENABLE_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(IR_ENABLE_PORT, IR_ENABLE_BIT, IR_ENABLED);

  /* init common characteristics for inputs */
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;

  /* init pin for USB sense input */
  USB_SENSE_PORT_CLK_EN();
  GPIO_InitStructure.Pin = USB_SENSE_BIT;
#ifdef DEBUG
  GPIO_InitStructure.Pull = GPIO_PULLUP; // xxx: must only be active while debugging
#endif
  HAL_GPIO_Init(USB_SENSE_PORT, &GPIO_InitStructure);

  /* init pin for PSU sense input */
  PSU_SENSE_PORT_CLK_EN();
  GPIO_InitStructure.Pin = PSU_SENSE_BIT;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PSU_SENSE_PORT, &GPIO_InitStructure);
}

#if defined(USE_BACKUP_SUPPLY)
/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SystemClockConfig_STOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while(__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

  /* After wake-up from STOP reconfigure the system clock: Enable HSE and PLL */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL8;
  RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLL_DIV3;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/*******************************************************************************
* Function Name  : PrepareStopMode.
* Description    : Prepares peripherals before entering the stop mode..
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void PrepareStopMode(void)
{
   /* Disable IR receiver (set high) */
   HAL_GPIO_WritePin(IR_ENABLE_PORT, IR_ENABLE_BIT, IR_DISABLED);

   /* Disable USB & TIM7 clock */
   // todo: disable more clocks (e.g. USB/PCD)
   IRMP_IRSND_TIMER_CLK_DIS();

   /* Disable GPIO clocks */
   RCC->AHBENR &= ~(RCC_AHBENR_GPIOCEN);

   /* Clear the WAKEUPTIMER interrupt pending flag and the according EXTI flag,
    * otherwise program execution continues */
   __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle, RTC_FLAG_WUTF);
   __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
}

/*******************************************************************************
* Function Name  : LeaveStopMode.
* Description    : Enables HSE, PLL and peripherals after leaving the stop mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void LeaveStopMode(void)
{
   /* Configures system clock after wake-up from STOP: enable HSE, PLL and
      select PLL as system clock source (HSE and PLL are disabled automatically
      in STOP mode) */
   SystemClockConfig_STOP();

   /* Enable GPIO clocks */
   RCC->AHBENR |= (RCC_AHBENR_GPIOCEN);

   /* Enable USB & TIM7 clock */
   // todo: enable clocks that were disabled above (e.g. USB/PCD)
   IRMP_IRSND_TIMER_CLK_EN();

   /* Enable IR receiver (set low) */
   HAL_GPIO_WritePin(IR_ENABLE_PORT, IR_ENABLE_BIT, IR_ENABLED);
}
#endif

/**
  * @brief  This function handles TIMx global interrupt request.
  */
void IRMP_IRSND_TIMER_IRQ_HANDLER(void)
{
   if( !irsnd_ISR() )   // call irsnd ISR
   {                    // if not busy...
      irmp_ISR();       // call irmp ISR
   }

   __HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
}

/**
  * @brief  This function handles RTC Wakeup global interrupt request.
  */
#if defined(STM32F103xB)
void RTC_IRQHandler(void)
#elif defined(STM32L151xB)
void RTC_WKUP_IRQHandler(void)
#else
#error Device not specified.
#endif
{
#if defined(STM32F103xB)
   if(__HAL_RTC_SECOND_GET_IT_SOURCE(&RtcHandle, RTC_IT_SEC) != RESET)
#elif defined(STM32L151xB)
   if(__HAL_RTC_WAKEUPTIMER_GET_IT_SOURCE(&RtcHandle, RTC_IT_WUT) != RESET)
#else
#error Device not specified.
#endif
   {
      SWRTC_Service();  // service software RTC
      DEB_Service();    // debounce signals (MUST happen inside ISR)
      flags.wakeup_occurred = 1;
   }

#if defined(STM32F103xB)
   /* Clear the SECOND interrupt pending flag */
   __HAL_RTC_SECOND_CLEAR_FLAG(&RtcHandle, RTC_FLAG_SEC);
#elif defined(STM32L151xB)
   /* Clear the WAKEUPTIMER interrupt pending flag and the according EXTI flag */
   __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle, RTC_FLAG_WUTF);
   __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
#else
#error Device not specified.
#endif
}
