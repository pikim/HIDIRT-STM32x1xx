/**
 * @file       debounce.c
 * @brief      Module for key and/ or signal debouncing.
 * @see        debounce.h for informations about how to use this module and how
 *             it works.
 */

/* Includes ------------------------------------------------------------------*/
#include "debounce.h"
#include "configuration.h"

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
/**
 * @brief      Mask for the keys that shall have long or repeated key press
 *             functionality.
 */
#define  REPEAT_MASK          0

/**
 * @brief      Number of \c DEB_Service() calls until a long or repeated press
 *             is detected.
 */
#define  REPEAT_START         50

/**
 * @brief      Number of \c DEB_Service() calls until the next repeated press
 *             is detected.
 */
#define  REPEAT_NEXT          20

/* Private macro -------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
 * @brief      Holds debounced and inverted state (key is pressed if bit is
 *             high).
 */
static volatile debounce_t    key_state = -1;

/**
 * @brief      Holds key pressed state (0 => 1 occured if bit is high).
 */
static volatile debounce_t    key_press = 0;

/**
 * @brief      Holds key released state (1 => 0 occured if bit is high).
 */
static volatile debounce_t    key_release = 0;

/**
 * @brief      Long key press and repetition state.
 */
static volatile debounce_t    key_rpt = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
 * @brief      Collect the input signals that shall be debounced.
 * @note       This function must be non-static if one wants to override it
 *             using the weak attribute. See debounce.h for the according
 *             declaration.
 * @return     The signals to be debounced put together in a \c debounce_t.
 */
static debounce_t DEB_CollectKeys(void)
{
   debounce_t        keys_to_debounce = 0;

   // Copy inputs that shall be debounced into one register.
   // Be careful to not override one bit with another input.

   // key_state is 1 if PSU voltage (5V on floppy) is present and 0 if not
   // 0 -> PC is turned off
   // 1 -> PC is running
   if( !HAL_GPIO_ReadPin(PSU_SENSE_PORT, PSU_SENSE_BIT) ) {
      keys_to_debounce |= DEB_PSU_SENSE;
   }

   // key_state is 1 if USB voltage (5V) is present and 0 if not
   // 0 -> no AC power present -> power failure
   // 1 -> AC power present -> normal operation possible
   if( !HAL_GPIO_ReadPin(USB_SENSE_PORT, USB_SENSE_BIT) ) {
      keys_to_debounce |= DEB_USB_SENSE;
   }

   return keys_to_debounce;
}

/* Extern functions ----------------------------------------------------------*/
/**
 * @brief      Set the default state of the signals to be debounced.
 * @attention  Overwrites \c key_state, so call it only at start-up or if you
 *             really want to overwrite the state.
 */
void DEB_Init(debounce_t default_state)
{
   key_state = default_state;
//   key_press = 0;
//   key_release = 0;
//   key_rpt = 0;
}

/**
 * @brief      Service routine to collect the keys and debounce the given
 *             signals.
 * @note       Must be called regularly (usually every 0.1ms...20ms).
 */
void DEB_Service(void)
{
   static debounce_t ct0 = -1, ct1 = -1, rpt;
   debounce_t        i;

   i = key_state ^ DEB_CollectKeys();     // one or more signals changed?
   //i = key_state ^ ~DEB_CollectKeys();    // one or more signals changed?
   ct0 = ~( ct0 & i );                    // count in ct0 or reset
   ct1 = ct0 ^ (ct1 & i);                 // count in ct1 or reset
   i &= ct0 & ct1;                        // count until overrun
   key_state ^= i;                        // invert the debounced state
   key_press |= key_state & i;            // 0->1: key press detected
   key_release |= ~key_state & i;         // 1->0: key release detected

   if( (key_state & REPEAT_MASK) == 0 )   // if no key is pressed or no REPEAT_MASK is defined (REPEAT_MASK = 0)
   {                                      // and therefore the pressed key has no repeat-function
      rpt = REPEAT_START;                 // load REPEAT_START-value into counter (see debounce.h)
   }

   if( --rpt == 0 )                       // if counter reaches 0 (after e.g. 500ms = 10ms * 50)
   {
      rpt = REPEAT_NEXT;                  // load REPEAT_NEXT-value into counter (200ms = 10ms * 20)
      key_rpt |= key_state & REPEAT_MASK; // remember a long key press
   }
}

/**
 * @brief      Read the current state of the debounced signals.
 * @param      key_mask is a bit mask with the signals to be read.
 * @return     State of the read signals
 *             - 0 if state is 0 and
 *             - 1 if state is 1
 */
debounce_t DEB_GetKeyState(debounce_t key_mask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      key_mask &= key_state;  // read state
   }
   return key_mask;
}

/**
 * @brief      Check if a key was pressed since the last call of this function.
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if level didn't change and
 *             - 1 if level transition occurred (0 => 1)
 */
debounce_t DEB_GetKeyPress(debounce_t key_mask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      key_mask &= key_press;  // read state
      key_press ^= key_mask;  // delete read state
   }
   return key_mask;
}

/**
 * @brief      Check if a key was released since the last call of this function.
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if level didn't change and
 *             - 1 if level transition occurred (1 => 0)
 * @see        http://www.mikrocontroller.net/topic/48465#1844461
 */
debounce_t DEB_GetKeyRelease(debounce_t key_mask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      key_mask &= key_release;// read state
      key_release ^= key_mask;// delete read state
   }
   return key_mask;
}

/**
 * @brief      Check if a key is held down/ pressed repeatedly.
 * @details    When a key is held down/ pressed longer than
 *             \c REPEAT_START * calling-interval of \c DEB_Service()
 *             this simulates the user repeatedly pressing and releasing a key.
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if key is not held down and
 *             - 1 if it is
 */
debounce_t DEB_GetKeyRepeat(debounce_t key_mask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      key_mask &= key_rpt;    // read state
      key_rpt ^= key_mask;    // delete read state
   }
   return key_mask;
}

/**
 * @brief      Check if a short/ normal key press occurred.
 * @details    Returns key_mask <b>after releasing</b> the key when it was
 *             pressed shorter than
 *             \c REPEAT_START * calling-interval of \c DEB_Service().
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if key was not pressed shortly and
 *             - 1 if it was
 */
debounce_t DEB_GetKeyShort(debounce_t key_mask)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      key_mask = DEB_GetKeyPress( ~key_state & key_mask );
   }
   return key_mask;
}

/**
 * @brief      Check if a long key press occurred.
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if key is not pressed long and
 *             - 1 if it is
 */
debounce_t DEB_GetKeyLong(debounce_t key_mask)
{
   return DEB_GetKeyPress( DEB_GetKeyRepeat(key_mask) );
}

/**
 * @brief      Check if two keys are being pressed simultaneously.
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if keys are not pressed simultaneously and
 *             - 1 if they are
 * @see        http://www.mikrocontroller.net/topic/48465#1753367
 */
debounce_t DEB_GetKeyCommon(debounce_t key_mask)
{
   return DEB_GetKeyPress( (key_press & key_mask) == key_mask ? key_mask : 0 );
}

/**
 * @brief      Check if a long key press occurred or if the key is held down.
 * @attention  MUST BE USED IN CONJUNCTION WITH \c DEB_GetKeyRepeat_lng()!
 * @code
 *             ...
 *             if( get_key_long_r( 1<<KEY0 )) LED1(ON);
 *             if( get_key_rpt_l( 1<<KEY0 ))  LED2(ON);
 *             ...
 * @endcode
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if key is not pressed long/ held down and
 *             - 1 if it is
 * @see        http://www.mikrocontroller.net/topic/48465#1750482
 */
debounce_t DEB_GetKeyLong_rpt(debounce_t key_mask) // if repeat function needed
{
   return DEB_GetKeyPress( DEB_GetKeyRepeat( key_press & key_mask ));
}

/**
 * @brief      Check if a key is held down/ pressed repeatedly
 * @details    When a key is held down/ pressed longer than
 *             (\c REPEAT_START + \c REPEAT_NEXT) * calling-interval of
 *             \c DEB_Service() this simulates the user repeatedly pressing and
 *             releasing a key.
 * @attention  MUST BE USED IN CONJUNCTION WITH \c DEB_GetKeyLong_rpt()!
 * @code
 *             ...
 *             if( get_key_long_r( 1<<KEY0 )) LED1(ON);
 *             if( get_key_rpt_l( 1<<KEY0 ))  LED2(ON);
 *             ...
 * @endcode
 * @note       With calling this function returned data will be deleted.
 * @param      key_mask is a bit mask with the signals to be checked.
 * @return     State of the checked signals
 *             - 0 if key is not pressed long/ repeatedly and
 *             - 1 if it is
 * @see        http://www.mikrocontroller.net/topic/48465#1750482
 */
debounce_t DEB_GetKeyRepeat_lng(debounce_t key_mask) // if long function needed
{
   return DEB_GetKeyRepeat( ~key_press & key_mask );
}
