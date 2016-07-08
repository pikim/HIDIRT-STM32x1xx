/**
 * @file       debounce.h
 * @brief      Module for key and/ or signal debouncing.
 *
 * @details    Samples the logic level of each key or signal (bit) and accepts a
 *             new state only if the level was stable for 4 samples. Depending
 *             on the used data type (\c debounce_t) it is possible to debounce
 *             a large number of keys/ signals (e.g. 32 keys using a
 *             \c uint32_t) at one time.
 *
 *             Several auxiliary functions help to provide different
 *             functionalities, e.g. detecting a short or long key press, key
 *             release and some more.
 *
 * @par        Functionality
 @verbatim

  Mode 0 (\c get_key_press or \c get_key_repeat)
  Mode 1 (\c get_key_press with \c get_key_repeat)
  =================================================
               time ---->
                    __      _________________      __
  key_state    ____/  \____/                 \____/  \____
  key_press    ----X-------X----------------------X-------
  key_repeat   --------------------X--X--X--X-------------
  key_release  -------X----------------------X-------X----
                           |       |  |  |
                           |       |__|__|
                           |       | \ /
                           |_______|  REPEAT_NEXT
                                 \
                                  REPEAT_START

  Mode 2 (\c get_key_short with \c get_key_long)
  =================================================
                    __      _________________      __
  key_state    ____/  \____/                 \____/  \____
  key_short    -------X------------------------------X----
  key_long     --------------------X----------------------
  key_release  -------X----------------------X-------X----

  Mode 3 (\c get_key_short with \c get_key_long_r and \c get_key_repeat_l)
  =========================================================================
                    __      _________________      __
  key_state    ____/  \____/                 \____/  \____
  key_short    -------X------------------------------X----
  key_long_rpt --------------------X----------------------
  key_rpt_long -----------------------X--X--X-------------
  key_release  -------X----------------------X-------X----

  Note: \c get_key_long_r and \c get_key_rpt_l MUST always be used in
        conjunction, even if only one of both functionalities is being used.

  The graphics were taken from http://www.mikrocontroller.net/topic/48465#1844458

 @endverbatim
 * @author     Peter Dannegger (2006-2010)
 * @author     Jonas P. (2010)
 * @author     Michael K. (2015)
 * @see        http://www.mikrocontroller.net/topic/48465
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>          // necessary depending on type of debounce_t

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/**
 * @brief      Define(s) for masking the bit position(s) of the key(s) that
 *             shall be debounced.
 * @{
 */
#define  DEB_PSU_SENSE       (debounce_t)1
#define  DEB_USB_SENSE       (debounce_t)2
/**
 * @}
 */

/* Exported types ------------------------------------------------------------*/
/**
 * @brief      The type of the variable to be debounced. Depends on the number
 *             of keys/ signals that need to be debounced.
 * @note       Arrays, structs or unions MUST NOT be used without modifications.
 */
typedef  uint8_t             debounce_t;
//typedef  uint16_t            debounce_t;
//typedef  uint32_t            debounce_t;
//typedef  uint64_t            debounce_t;

/* Exported functions ------------------------------------------------------- */
//debounce_t DEB_CollectKeys (void) __attribute__ ((weak));
void DEB_Init (debounce_t default_state);
void DEB_Service (void);
debounce_t DEB_GetKeyState (debounce_t key_mask);
debounce_t DEB_GetKeyPress (debounce_t key_mask);
debounce_t DEB_GetKeyRelease (debounce_t key_mask);
debounce_t DEB_GetKeyRepeat (debounce_t key_mask);
debounce_t DEB_GetKeyShort (debounce_t key_mask);
debounce_t DEB_GetKeyLong (debounce_t key_mask);
debounce_t DEB_GetKeyCommon (debounce_t key_mask);
debounce_t DEB_GetKeyLong_rpt (debounce_t key_mask);
debounce_t DEB_GetKeyRepeat_lng (debounce_t key_mask);

#endif /* DEBOUNCE_H */
