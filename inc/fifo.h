/**
 * @file       fifo.h
 * @brief      Module for a single FIFO or one single instance if included via
 *             fifos.h.
 *
 * @details    The data type of the entries can be easily customized, as well as
 *             the number of entries the FIFO can store.
 *
 * @remark     Code in the else-paths of \code #ifndef FIFO_PREFIX \endcode is
 *             relevant only if this modules' files are included by fifos.c and
 *             fifos.h to use it with multiple different fifos.
 *
 * @author     Michael Kipp (2015)
 * @see        http://www.mikrocontroller.net/articles/FIFO#2n-Ringpuffer_-_die_schnellste_L.C3.B6sung
 * @see        http://arnold.uthar.net/index.php?n=Work.TemplatesC
 */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#ifndef FIFO_PREFIX

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FIFO_H
#define FIFO_H

/* Includes ------------------------------------------------------------------*/
#include "irmp.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/**
* @brief       The number of elements the FIFO can hold. MUST be 2^n.
*/
#define FIFO_SIZE       16

/* Exported types ------------------------------------------------------------*/
/**
* @brief       Data type of the read and write indices.
*/
//typedef uint8_t         fifo_idx_t;
typedef uint16_t        fifo_idx_t;

/**
* @brief       The format of one single FIFO entry.
*/
typedef struct FIFO_ENTRY {
   IRMP_DATA data;
} fifo_entry_t;

/**
* @brief       The whole FIFO (entries and indices).
*/
typedef struct FIFO {
   fifo_idx_t           read;             /**< Read index.*/
   fifo_idx_t           write;            /**< Write index.*/
   fifo_entry_t         entry[FIFO_SIZE]; /**< The data/ entries itself.*/
} fifo_t;

/* Exported functions ------------------------------------------------------- */
bool FIFO_Read (fifo_t* fifo, fifo_entry_t* entry);
bool FIFO_Write (fifo_t* fifo, fifo_entry_t* entry);
bool FIFO_IsEmpty (fifo_t* fifo);
bool FIFO_IsFull (fifo_t* fifo);
void FIFO_Clear (fifo_t* fifo);
fifo_idx_t FIFO_Count (fifo_t* fifo);

#endif /* #ifndef FIFO_H */

#else /* #ifndef FIFO_PREFIX */

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/**
* @brief       Enum to allow the usage of a macro for setting the FIFO size.
*/
enum {FIFOS(FIFO_PREFIX,FIFO_ELEMENTS) = FIFO_SIZE};

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
* @brief       Data type of the read and write indices.
*/
//typedef uint8_t         FIFOS(FIFO_PREFIX,fifo_idx_t);
typedef uint16_t        FIFOS(FIFO_PREFIX,fifo_idx_t);

/**
* @brief       The whole FIFO (entries and indices).
*/
typedef struct FIFOS(FIFO_PREFIX,FIFO) {
   FIFOS(FIFO_PREFIX,fifo_idx_t)   read;  /**< Read index.*/
   FIFOS(FIFO_PREFIX,fifo_idx_t)   write; /**< Write index.*/
   FIFOS(FIFO_PREFIX,fifo_entry_t) entry[FIFOS(FIFO_PREFIX,FIFO_ELEMENTS)];/**< The data/ entries itself.*/
} FIFOS(FIFO_PREFIX,fifo_t);

/* Exported functions ------------------------------------------------------- */
bool FIFOS(FIFO_PREFIX,FIFO_Read)(FIFOS(FIFO_PREFIX,fifo_t)* fifo, FIFOS(FIFO_PREFIX,fifo_entry_t)* entry);
bool FIFOS(FIFO_PREFIX,FIFO_Write)(FIFOS(FIFO_PREFIX,fifo_t)* fifo, FIFOS(FIFO_PREFIX,fifo_entry_t)* entry);
bool FIFOS(FIFO_PREFIX,FIFO_IsEmpty)(FIFOS(FIFO_PREFIX,fifo_t)* fifo);
bool FIFOS(FIFO_PREFIX,FIFO_IsFull)(FIFOS(FIFO_PREFIX,fifo_t)* fifo);
void FIFOS(FIFO_PREFIX,FIFO_Clear)(FIFOS(FIFO_PREFIX,fifo_t)* fifo);
FIFOS(FIFO_PREFIX,fifo_idx_t) FIFOS(FIFO_PREFIX,FIFO_Count)(FIFOS(FIFO_PREFIX,fifo_t)* fifo);

#endif /* #ifndef FIFO_PREFIX */

#if (FIFO_SIZE != 1) && (FIFO_SIZE != 2) && (FIFO_SIZE != 4) \
 && (FIFO_SIZE != 8) && (FIFO_SIZE != 16) && (FIFO_SIZE != 32) \
 && (FIFO_SIZE != 64) && (FIFO_SIZE != 128) && (FIFO_SIZE != 256) \
 && (FIFO_SIZE != 512) && (FIFO_SIZE != 1024) && (FIFO_SIZE != 2048) \
 && (FIFO_SIZE != 4096) && (FIFO_SIZE != 8192) && (FIFO_SIZE != 16384) \
 && (FIFO_SIZE != 32768) && (FIFO_SIZE != 65536)
#error FIFO_SIZE is invalid! It MUST be 2^n! Fix this in fifos.h or fifo.h.
#endif
