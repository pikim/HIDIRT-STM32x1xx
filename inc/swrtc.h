/**
 * @file       swrtc.h
 * @brief      Module for a software based RTC.
 *
 * @details    Depending on the settings, up to 1ms resolution can be achieved.
 *             By adjusting a deviation value, eventual clock tolerances can be
 *             compensated. The compensation that can be achieved depends on the
 *             value of \c SWRTC_TICKS_PER_SECOND.
 *
 *             If, for example, \c SWRTC_TICKS_PER_SECOND
 *             - is set to 1e6: up to 1ppm can be compensated,
 *             - is set to 20e6: up to 0.05ppm can be compensated.
 *
 *             Compensation example:<br/>
 *             with \c SWRTC_TICKS_PER_SECOND = 4 and after period of 60 real
 *             seconds the SWRTC shows
 *             - 48 seconds with deviation = -1
 *             - 72 seconds with deviation = +1
 *
 *             The short time accuracy may be poor if
 *             \c SWRTC_INTERVALS_PER_SECOND is set to a relatively low value
 *             and a compensation cycle occurs. Nevertheless the long time
 *             accuracy will be quite good. If a better short time accuracy is
 *             needed, \c SWRTC_INTERVALS_PER_SECOND must be set to a higher
 *             value.
 *
 *             The module also offers the possibility to register callback
 *             functions that will be executed every half or every full second.
 *
 *             Moreover a number of alarms can be set up, each with a dedicated
 *             time (only full seconds are supported) and callback function.
 *
 * @remark     The code works exactly as Roman Black explains on his webpage,
 *             plus the part around the error compensation.
 *
 * @author     Roman Black (2001-2011)
 * @author     Peter Dannegger (2003)
 * @author     Michael K. (2016)
 * @see        http://www.romanblack.com/one_sec.htm
 * @see        http://www.mikrocontroller.net/articles/AVR_-_Die_genaue_Sekunde_/_RTC#Bresenham_f.C3.BCr_RTCs
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SWRTC_H
#define SWRTC_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/**
 * @brief      Enable a callback function to be called every second.
 */
#define SWRTC_ENABLE_FULL_SECOND_CALLBACK    1

/**
 * @brief      Enable a callback function to be called every half second.
 */
#define SWRTC_ENABLE_HALF_SECOND_CALLBACK    0

/**
 * @brief      Enable alarms and callbacks at a given time.
 */
#define SWRTC_ENABLE_ALARMS                  1

/* Exported types ------------------------------------------------------------*/
/**
 * @brief      A type that contains a time in seconds and ticks, where one tick
 *             is 100us and possibly rounded.
 */
typedef struct SWRTC_TIME
{
   uint32_t    seconds;
   int16_t     ticks;
} swrtc_time_t;

/* Exported functions ------------------------------------------------------- */
void SWRTC_Service (void);
int32_t SWRTC_GetDeviation (void);
bool SWRTC_SetDeviation (int32_t deviation);
uint32_t SWRTC_GetSeconds (void);
void SWRTC_SetSeconds (uint32_t seconds);
int16_t SWRTC_GetTicks (void);
void SWRTC_SetTicks (uint16_t ticks);
swrtc_time_t SWRTC_GetTime (void);
void SWRTC_SetTime (swrtc_time_t time);

#if SWRTC_ENABLE_FULL_SECOND_CALLBACK || DOXYGEN
void SWRTC_RegisterFullSecondCallback (void (*cb)(void));
#endif

#if SWRTC_ENABLE_HALF_SECOND_CALLBACK || DOXYGEN
void SWRTC_RegisterHalfSecondCallback (void (*cb)(void));
#endif

#if SWRTC_ENABLE_ALARMS || DOXYGEN
uint32_t SWRTC_GetAlarmTime (uint8_t idx);
bool SWRTC_SetAlarmTime (uint8_t idx, uint32_t alarm_time);
bool SWRTC_RegisterAlarmCallback (uint8_t idx, void (*cb)(uint8_t));
#endif

#endif /* SWRTC_H */
