/**
 * @file       swrtc.c
 * @brief      Module for a software based RTC.
 * @see        swrtc.h for informations about how to use this module and how it
 *             works.
 */

/* Includes ------------------------------------------------------------------*/
#include "swrtc.h"

/* Includes and private defines for MCU customization and portability --------*/
#ifndef DOXYGEN
  #if defined(USE_STDPERIPH_DRIVER) || defined(USE_HAL_DRIVER)
    #include "cm_atomic.h"
  #endif
  #if defined(USE_STDPERIPH_DRIVER)
    #if defined(STM32L1XX_MD) || defined(STM32L1XX_MDP) || defined(STM32L1XX_HD)
      #include <stm32l1xx.h>
    #else
      #error Device not specified.
    #endif
  #elif defined(USE_HAL_DRIVER)
    #if defined(STM32F103xB)
      #include "stm32f1xx_hal.h"
    #elif defined(STM32L151xB)
      #include "stm32l1xx_hal.h"
    #else
      #error Device not specified.
    #endif
  #else
    #include <avr/io.h>          // for PORT/ PIN access
    #include <avr/interrupt.h>   // for cli() and sei()
    #include <util/atomic.h>     // for ATOMIC_BLOCK(x)
  #endif
#endif

/* Private define ------------------------------------------------------------*/
#if SWRTC_ENABLE_ALARMS || DOXYGEN
/**
 * @brief      The number of available alarm times and callbacks.
 */
#define SWRTC_NUMBER_OF_ALARMS      2
#endif

/**
 * @brief      That often the RTC service routine will be called every second.
 */
#define SWRTC_INTERVALS_PER_SECOND  2

/**
 * @brief      The number of ticks every second.
 * @details    May be the oscillator frequency and determines the deviation
 *             that can be compensated, e.g.:
 *             - when set to 1e6: up to 1ppm can be compensated,
 *             - when set to 20e6: up to 0.05ppm can be compensated.
 */
#define SWRTC_TICKS_PER_SECOND      32000000UL

/**
 * @brief      Allows avoidance of consecutive time error compensation.
 * @details    When set to 0 it may happen that the time is compensated in two
 *             consecutive calls of \c SWRTC_Service(). This leads to different
 *             negative/ positive compensation results, although the absolute
 *             deviation value is the same. For example:
 *             - +80 deviation leads to +5 minutes after a given time while
 *             - -80 deviation leads to -3 minutes after the same time.
 *
 *             This behavior can be avoided by setting the macro to any other
 *             value than 0.
 */
#define SWRTC_AVOID_CONSECUTIVE_COMPENSATION 1

/**
 * @brief      The number of ticks every interval.
 */
#define SWRTC_TICKS_PER_INTERVAL (SWRTC_TICKS_PER_SECOND/SWRTC_INTERVALS_PER_SECOND)

#ifndef NULL
/**
 * @brief      Define NULL pointer in case none is existing, yet.
 */
#define NULL   ((void *)0)
#endif

/* Private macro -------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
#if SWRTC_ENABLE_ALARMS || DOXYGEN
/**
 * @brief      Structure that holds the alarm members.
 */
typedef struct SWRTC_ALARM {
   void (*callback)(uint8_t); /**< Pointer to the alarm callback function. */
   uint32_t time;             /**< The alarm time. */
} swrtc_alarm_t;
#endif

/* Private variables ---------------------------------------------------------*/
/**
 * @brief      Holds the current time in seconds.
 */
static volatile uint32_t      clk_secs = 0;

/**
 * @brief      Holds the current number of ticks per second.
 */
static volatile uint32_t      clk_ticks = 0;

/**
 * @brief      Holds the deviation of the clock source.
 */
static int32_t                clk_deviation = 0;

#if SWRTC_ENABLE_FULL_SECOND_CALLBACK || DOXYGEN
/**
 * @brief      Pointer to the full-second callback function.
 */
static void (*full_second_callback_ptr)(void) = NULL;
#endif

#if SWRTC_ENABLE_HALF_SECOND_CALLBACK || DOXYGEN
/**
 * @brief      Pointer to the half-second callback function.
 */
static void (*half_second_callback_ptr)(void) = NULL;
#endif

#if SWRTC_ENABLE_ALARMS || DOXYGEN
/**
 * @brief      Holds the alarm time(s) and callback(s).
 */
static swrtc_alarm_t alarms[SWRTC_NUMBER_OF_ALARMS];
#endif

/* Extern variables ----------------------------------------------------------*/
/* Error handling ------------------------------------------------------------*/
#if SWRTC_TICKS_PER_SECOND > __UINT32_MAX__
#undef SWRTC_TICKS_PER_SECOND
#error Number is out of bounds.
#endif

#if SWRTC_INTERVALS_PER_SECOND > SWRTC_TICKS_PER_SECOND
#undef SWRTC_INTERVALS_PER_SECOND
#error Number of intervals must not be bigger than number of ticks per second.
#endif

#if SWRTC_TICKS_PER_SECOND % SWRTC_INTERVALS_PER_SECOND
#warning SWRTC_TICKS_PER_SECOND/SWRTC_INTERVALS_PER_SECOND is not an integer \
         and therefore timing accuracy will suffer.
#endif

/* Private functions ---------------------------------------------------------*/
/* Extern functions ----------------------------------------------------------*/
/**
 * @brief      Service routine to process the RTC and its callbacks.
 * @note       Must be called regularly in a constant interval.
 */
void SWRTC_Service(void)
{
#if SWRTC_AVOID_CONSECUTIVE_COMPENSATION
   static uint_fast8_t compensated = false;
   uint_fast8_t temp_flag = false;
#endif

   // add ticks for one interval
   clk_ticks += SWRTC_TICKS_PER_INTERVAL;

#if SWRTC_ENABLE_HALF_SECOND_CALLBACK
   // call corresponding function every half and every full second
   if( ( clk_ticks >= SWRTC_TICKS_PER_SECOND / 2 &&
         clk_ticks <  SWRTC_TICKS_PER_SECOND / 2 + SWRTC_TICKS_PER_INTERVAL )
      || clk_ticks >= SWRTC_TICKS_PER_SECOND )
   {
      if( *half_second_callback_ptr != NULL )
      {
         (*half_second_callback_ptr)();
      }
   }
#endif

   // compute time when one second has passed
   if( clk_ticks >= SWRTC_TICKS_PER_SECOND )
   {
      clk_secs++;
      clk_ticks -= SWRTC_TICKS_PER_SECOND;

#if SWRTC_AVOID_CONSECUTIVE_COMPENSATION
      if( compensated == false )
      {
         // add deviation only when not compensated in the last cycle
         clk_ticks += clk_deviation;

         // set flag to remember compensation state
         temp_flag = true;
      }
#else
      // add deviation
      clk_ticks += clk_deviation;
#endif

#if SWRTC_ENABLE_FULL_SECOND_CALLBACK
      // call corresponding function every full second
      if( *full_second_callback_ptr != NULL )
      {
         (*full_second_callback_ptr)();
      }
#endif

#if SWRTC_ENABLE_ALARMS
      for(uint8_t idx=0; idx<SWRTC_NUMBER_OF_ALARMS; idx++)
      {
         if( alarms[idx].time == clk_secs && alarms[idx].callback != NULL )
         {
            // call corresponding function if alarm time is met and a callback
            // is registered
            alarms[idx].callback(idx);
         }
      }
#endif
   }

#if SWRTC_AVOID_CONSECUTIVE_COMPENSATION
   // remember compensation state for the next call of this function
   compensated = temp_flag;
#endif
}

/**
 * @brief      Get the clock deviation.
 * @return     The clock deviation in digits, that means the difference to
 *             \c SWRTC_TICKS_PER_SECOND. It is
 *             - positive if \c SWRTC_TICKS_PER_SECOND is too low
 *             - negative if \c SWRTC_TICKS_PER_SECOND is too high
 */
int32_t SWRTC_GetDeviation(void)
{
   int32_t        deviation;

   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      deviation = clk_deviation;
   }
   return deviation;
}

/**
 * @brief      Set the clock deviation.
 * @param      deviation is the clock deviation in digits, that means the
 *             difference to \c SWRTC_TICKS_PER_SECOND. It is
 *             - positive if \c SWRTC_TICKS_PER_SECOND is too low
 *             - negative if \c SWRTC_TICKS_PER_SECOND is too high
 * @return     The execution state:
 *             - \c false if value was out of bounds
 *             - \c true if it was set successfully
 */
bool SWRTC_SetDeviation(int32_t deviation)
{
   if( deviation < (int32_t)SWRTC_TICKS_PER_SECOND
    && deviation > (int32_t)-SWRTC_TICKS_PER_SECOND )
   {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
         clk_deviation = deviation;
      }
      return true;
   }
   return false;
}

/**
 * @brief      Get the current seconds.
 * @return     Current seconds.
 */
uint32_t SWRTC_GetSeconds(void)
{
   uint32_t       seconds;

   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      seconds = clk_secs;
   }
   return seconds;
}

/**
 * @brief      Set the current seconds.
 * @param      seconds to be set.
 */
void SWRTC_SetSeconds(uint32_t seconds)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      clk_secs = seconds;
   }
}

/**
 * @brief      Get the current ticks.
 * @return     Current ticks in a multiple of 100us (e.g. 950 for 95ms).
 *             There are two special cases because of the compensation feature:
 *             - if value is negative it must be subtracted from current time
 *             - if value is positive above 10000, one more second has passed
 *               and 10000 must be subtracted from the return value
 * @see        \c SWRTC_GetTime() for an example how to use it.
 */
int16_t SWRTC_GetTicks(void)
{
   int32_t        ticks;

   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      ticks = clk_ticks;
   }
#if SWRTC_TICKS_PER_SECOND >= 10000
   ticks /= ((SWRTC_TICKS_PER_SECOND + clk_deviation) / 10000);
#else
   ticks *= (10000 / (SWRTC_TICKS_PER_SECOND + clk_deviation));
#endif
   return (int16_t)ticks;
}

/**
 * @brief      Set the current ticks.
 * @param      ticks is a multiple of 100us to be set.
 */
void SWRTC_SetTicks(uint16_t ticks)
{
#if SWRTC_TICKS_PER_SECOND >= 10000
   ticks *= ((SWRTC_TICKS_PER_SECOND + clk_deviation) / 10000);
#else
   ticks /= (10000 / (SWRTC_TICKS_PER_SECOND + clk_deviation));
#endif
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      clk_ticks = ticks;
   }
}

/**
 * @brief      Get the current time (seconds and ticks).
 * @return     Current time.
 */
swrtc_time_t SWRTC_GetTime(void)
{
   swrtc_time_t   time;

   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      time.seconds = clk_secs;
      time.ticks = SWRTC_GetTicks();
   }

   if( time.ticks < 0 )
   {
      time.seconds--;
      time.ticks += 10000;
   }

   if( time.ticks > 10000 )
   {
      time.seconds++;
      time.ticks -= 10000;
   }
   return time;
}

/**
 * @brief      Set the current time (seconds and ticks).
 * @param      time to be set.
 */
void SWRTC_SetTime(swrtc_time_t time)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      clk_secs = time.seconds;
      SWRTC_SetTicks(time.ticks);
   }
}

#if SWRTC_ENABLE_FULL_SECOND_CALLBACK || DOXYGEN
/**
 * @brief      Set a pointer to a callback function that will be called every
 *             second.
 * @param      *cb is a pointer to the callback function. The function being
 *             called takes and returns nothing.
 *             Pass a NULL pointer ((void *)0) to unregister callback function.
 */
void SWRTC_RegisterFullSecondCallback(void (*cb)(void))
{
   full_second_callback_ptr = cb;
}
#endif

#if SWRTC_ENABLE_HALF_SECOND_CALLBACK || DOXYGEN
/**
 * @brief      Set a pointer to a callback function that will be called every
 *             half second.
 * @param      *cb is a pointer to the callback function. The function being
 *             called takes and returns nothing.
 *             Pass a NULL pointer ((void *)0) to unregister callback function.
 */
void SWRTC_RegisterHalfSecondCallback(void (*cb)(void))
{
   half_second_callback_ptr = cb;
}
#endif

#if SWRTC_ENABLE_ALARMS || DOXYGEN
/**
 * @brief      Set a pointer to a callback function that will be called when an
 *             alarm time is met.
 * @param      idx is the number of the desired alarm.
 * @param      *cb is a pointer to the corresponding alarm function. The
 *             function being called takes a \c uint8_t holding the calling
 *             alarm index and returns nothing.
 *             Pass a NULL pointer ((void *)0) to unregister callback function.
 * @return     Execution state
 *             - \c true if callback was successfully registered
 *             - \c false if idx was invalid
 */
bool SWRTC_RegisterAlarmCallback(uint8_t idx, void (*cb)(uint8_t))
{
   if( idx < SWRTC_NUMBER_OF_ALARMS )
   {
      alarms[idx].callback = cb;
      return true;
   }
   return false;
}

/**
 * @brief      Get an alarm time.
 * @param      idx is the number of the desired alarm.
 * @return     The alarm time or 0 if idx was invalid.
 */
uint32_t SWRTC_GetAlarmTime(uint8_t idx)
{
   if( idx < SWRTC_NUMBER_OF_ALARMS )
   {
      return alarms[idx].time;
   }
   return 0;
}

/**
 * @brief      Set an alarm time.
 * @param      idx is the number of the desired alarm.
 * @param      alarm_time to be set.
 * @return     Execution state
 *             - \c true if the alarm time was successfully set
 *             - \c false if idx was invalid
 */
bool SWRTC_SetAlarmTime(uint8_t idx, uint32_t alarm_time)
{
   if( idx < SWRTC_NUMBER_OF_ALARMS )
   {
      alarms[idx].time = alarm_time;
      return true;
   }
   return false;
}
#endif
