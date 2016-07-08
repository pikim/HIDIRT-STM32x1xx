/**
 * @file       fifo.c
 * @brief      Module for a single FIFO or a single instance if included via
 *             fifos.c.
 * @see        fifo.h for informations about how to use this module and how it
 *             works.
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#ifndef FIFO_PREFIX
#include "fifo.h"
#else
#include "fifos.h"
#endif

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
* @brief       A variable that holds the mask: that is the FIFO size minus 1.
*/
#ifndef FIFO_PREFIX
static const uint16_t fifo_mask = FIFO_SIZE-1;
#else
static const uint16_t FIFOS(FIFO_PREFIX,fifo_mask) = FIFOS(FIFO_PREFIX,FIFO_ELEMENTS)-1;
#endif

/* Extern variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Extern functions ----------------------------------------------------------*/
/**
* @brief       Reads an entry from the FIFO.
* @param[in]   fifo from which the data shall be read.
* @param[out]  entry that was read.
* @return      Execution state
*              - \c true if an entry was read successfully
*              - \c false if the FIFO was empty
*/
#ifndef FIFO_PREFIX
bool FIFO_Read(fifo_t* fifo, fifo_entry_t* entry)
#else
bool FIFOS(FIFO_PREFIX,FIFO_Read)(FIFOS(FIFO_PREFIX,fifo_t)* fifo, FIFOS(FIFO_PREFIX,fifo_entry_t)* entry)
#endif
{
   if(fifo->read == fifo->write)
   {
      return false;
   }
#ifndef FIFO_PREFIX
   memcpy(entry, &fifo->entry[fifo->read], sizeof(fifo_entry_t));
   fifo->read = (fifo->read + 1) & fifo_mask;
#else
   memcpy(entry, &fifo->entry[fifo->read], sizeof(FIFOS(FIFO_PREFIX,fifo_entry_t)));
   fifo->read = (fifo->read + 1) & FIFOS(FIFO_PREFIX,fifo_mask);
#endif
   return true;
}

/**
* @brief       Writes an entry into the FIFO.
* @param[in]   fifo into which the data shall be written.
* @param[in]   entry that shall be written.
* @return      Execution state
*              - \c true if the entry was stored successfully
*              - \c false if the FIFO was full
*/
#ifndef FIFO_PREFIX
bool FIFO_Write(fifo_t* fifo, fifo_entry_t* entry)
{
   fifo_idx_t next;
   next = (fifo->write + 1) & fifo_mask;
#else
bool FIFOS(FIFO_PREFIX,FIFO_Write)(FIFOS(FIFO_PREFIX,fifo_t)* fifo, FIFOS(FIFO_PREFIX,fifo_entry_t)* entry)
{
   FIFOS(FIFO_PREFIX,fifo_idx_t) next;
   next = (fifo->write + 1) & FIFOS(FIFO_PREFIX,fifo_mask);
#endif
   if(fifo->read == next)
   {
      return false;
   }
//   memcpy(&fifo->entry[fifo->write], entry, sizeof(entry));
#ifndef FIFO_PREFIX
   memcpy(&fifo->entry[fifo->write & fifo_mask], entry, sizeof(fifo_entry_t));
#else
   memcpy(&fifo->entry[fifo->write & FIFOS(FIFO_PREFIX,fifo_mask)], entry, sizeof(FIFOS(FIFO_PREFIX,fifo_entry_t)));
#endif
   fifo->write = next;
   return true;
}

/**
* @brief       Checks if the FIFO is empty.
* @param[in]   fifo which shall be checked.
* @return      \c true if the FIFO is empty, \c false if it is not.
*/
#ifndef FIFO_PREFIX
bool FIFO_IsEmpty(fifo_t* fifo)
#else
bool FIFOS(FIFO_PREFIX,FIFO_IsEmpty)(FIFOS(FIFO_PREFIX,fifo_t)* fifo)
#endif
{
   if(fifo->read == fifo->write)
   {
      return true;
   }
   return false;
}

/**
* @brief       Checks if the FIFO is full.
* @param[in]   fifo which shall be checked.
* @return      \c true if the FIFO is full, \c false if it is not.
*/
#ifndef FIFO_PREFIX
bool FIFO_IsFull(fifo_t* fifo)
{
   if(fifo->read == ((fifo->write + 1) & fifo_mask))
#else
bool FIFOS(FIFO_PREFIX,FIFO_IsFull)(FIFOS(FIFO_PREFIX,fifo_t)* fifo)
{
   if(fifo->read == ((fifo->write + 1) & FIFOS(FIFO_PREFIX,fifo_mask)))
#endif
   {
      return true;
   }
   return false;
}

/**
* @brief       Clears the FIFO (sets both indices to 0).
* @param[in]   fifo to be cleared.
*/
#ifndef FIFO_PREFIX
void FIFO_Clear(fifo_t* fifo)
#else
void FIFOS(FIFO_PREFIX,FIFO_Clear)(FIFOS(FIFO_PREFIX,fifo_t)* fifo)
#endif
{
   fifo->read = 0;
   fifo->write = 0;
}

/**
* @brief       Counts the number of elements that are stored in the FIFO.
* @param[in]   fifo whose elements shall be counted.
* @return      The number of stored elements.
*/
#ifndef FIFO_PREFIX
fifo_idx_t FIFO_Count(fifo_t* fifo)
{
   return (fifo->write - fifo->read) & fifo_mask;
#else
FIFOS(FIFO_PREFIX,fifo_idx_t) FIFOS(FIFO_PREFIX,FIFO_Count)(FIFOS(FIFO_PREFIX,fifo_t)* fifo)
{
   return (fifo->write - fifo->read) & FIFOS(FIFO_PREFIX,fifo_mask);
#endif
}
