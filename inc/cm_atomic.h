/**
 * @file       cm_atomic.h
 * @brief      Allows atomically executed code blocks
 *
 * @details    Implements ATOMIC_BLOCKs for ARM Cortex-Mx MCUs similar to the
 *             ones in atomic.h from avr-libc.
 *
 *             Usage:
 * @code
 *             #include "cm_atomic.h"
 *
 *             // inside a function
 *             ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
 *             {
 *                key_mask &= key_state; // atomic operations
 *             }
 * @endcode
 *
 * @note       The macros herein require the ISO/IEC 9899:1999 ("ISO C99")
 *             feature of for loop variables that are declared inside the for
 *             loop itself. Therefore the macros can only be used when the
 *             compilers' standard level is at least set to either \c c99 or
 *             \c gnu99 (option --std=). \c c11 or \c gnu11 will also work.
 *
 *             The macros in this file ease the creation of code blocks that
 *             have to be executed atomically. In this context "atomic
 *             execution" means that the corresponding code block cannot be
 *             interrupted.
 *
 *             The macros handle their exit paths automatically so the special
 *             registers that are used will have the same state as they had
 *             before the block was entered.
 *
 * @author     Philipp Schröter (2016)
 * @author     A.K. (2016)
 * @author     Michael K. (2016)
 * @see        http://www.nongnu.org/avr-libc/user-manual/group__util__atomic.html
 * @see        https://www.mikrocontroller.net/topic/312246#4560455
 */

#ifndef CM_ATOMIC_H
#define CM_ATOMIC_H

/**
 * @def        ATOMIC_BLOCK(type)
 * @brief      Prevents a piece of code from being interrupted using \c PRIMASK.
 * @details    How it works:
 *             1. the state of the \c PRIMASK bit is read and stored
 *             2. the \c PRIMASK bit is set and from there on all interrupts
 *                with configurable priority are blocked
 *             3. code is executed atomically
 *             4. the \c PRIMASK bit is deleted, depending on the given
 *                parameter and its state when the block was entered
 *
 * @attention  NMI and hard fault exceptions cannot be masked and are even able
 *             to interrupt the ATOMIC_BLOCKs.
 * @attention  You MUST NOT use \c break and \c continue statements inside the
 *             atomic block as it is constructed using a \c for loop.
 *
 *             The behavior of the macro can be controlled with the parameters
 *             \c ATOMIC_FORCEON and \c ATOMIC_RESTORESTATE.
 *
 *             This macro expands to:
 * @code
 *             uint32_t __cond = 1;
 *             uint32_t __prim = __get_PRIMASK();
 *             __cond != 0 ? (__disable_irq(), 0) : 0;
 *             while(__cond != 0)
 *             {
 *                // atomic operations go here
 *
 *                (type && __prim) == 0 ? (__enable_irq(), 0) : 0;
 *                __cond = 0;
 *                __cond != 0 ? (__disable_irq(), 0) : 0;
 *             }
 * @endcode
*/
#define ATOMIC_BLOCK(type) for( uint32_t __cond = 1, __prim = __get_PRIMASK(); \
                                __cond != 0 ? (__disable_irq(), 0) : 0, __cond != 0; \
                                (type && __prim) == 0 ? (__enable_irq(), 0) : 0, __cond = 0 )

/**
 * @def        ATOMIC_FORCEON
 * @brief      ATOMIC_BLOCK parameter that forces \c PRIMASK to 0 in the end.
 * @details    When this parameter is passed to \c ATOMIC_BLOCK(), the
 *             previously saved \c PRIMASK state will be ignored and \c PRIMASK
 *             will be set to 0. This allows all interrupts to occur and saves
 *             some memory and time as nothing has to be saved and restored.
 *
 * @attention  Use this option only if you know the previous \c PRIMASK state or
 *             if you know and understand all the consequences of enabling the
 *             interrupts.
*/
#define ATOMIC_FORCEON          0

/**
 * @def        ATOMIC_RESTORESTATE
 * @brief      ATOMIC_BLOCK parameter that restores \c PRIMASK state in the end.
 * @details    When this parameter is passed to \c ATOMIC_BLOCK(), the
 *             previously saved \c PRIMASK state will be restored after the
 *             block's content has been executed.
*/
#define ATOMIC_RESTORESTATE     1

/**
 * @def        ATOMIC_BLOCK_PRIO(prio)
 * @brief      Prevents a piece of code from being interrupted using \c BASEPRI.
 * @details    How it works:
 *             1. the value of the \c BASEPRI field is read and stored
 *             2. the \c BASEPRI field is adjusted and from there on interrupts
 *                with a priority lower than \c prio are not processed
 *             3. code is executed atomically
 *             4. the previously saved \c BASEPRI value is restored
 *
 * @attention  You MUST NOT use \c break and \c continue statements inside the
 *             atomic block as it is constructed using a \c for loop.
 *
 *             The parameter \c prio selects the \c BASEPRI to be set while
 *             executing the contained code.
 *
 *             This macro expands to:
 * @code
 *             uint32_t __cond = 1;
 *             uint32_t __prio = __get_BASEPRI();
 *             __cond != 0 ? (__set_BASEPRI_MAX(prio), 0) : 0;
 *             while(__cond != 0)
 *             {
 *                // atomic operations go here
 *
 *                __set_BASEPRI(__prio);
 *                __cond = 0;
 *                __cond != 0 ? (__set_BASEPRI_MAX(prio), 0) : 0;
 *             }
 * @endcode
*/
#define ATOMIC_BLOCK_PRIO(prio) for( uint32_t __cond = 1, __prio = __get_BASEPRI(); \
                                     __cond != 0 ? (__set_BASEPRI_MAX(prio), 0) : 0, __cond != 0; \
                                     __set_BASEPRI(__prio), __cond = 0 )
#endif /* CM_ATOMIC_H */
