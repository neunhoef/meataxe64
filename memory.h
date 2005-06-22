/*
 * $Id: memory.h,v 1.8 2005/06/22 21:52:53 jon Exp $
 *
 * Large memory manipulation for meataxe
 *
 */

#ifndef included__memory
#define included__memory

#include <stddef.h>

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * (1 << 20) is used */
extern void memory_init(const char *, size_t);

/* Dispose of the memory */
extern void memory_dispose(void);

/* Get a pointer to the first row n thousandths of the way through the memory */
extern word *memory_pointer(u32);

/* Get a pointer to the ith row of given size starting n thousandths of the way through the memory */
/* len is in words */
extern word *memory_pointer_offset(u32 n, u32 i, u32 len);

/* Find out how many rows of given size fit in given memory */
/* len is in words */
extern u32 memory_rows(u32 len, u32 size);

/* Find out how much memory is required for a given number of rows */
extern u32 find_extent(u32 nor, u32 len);

/* Compute the amount of memory required for rows of length len1
 * Such that we can have the same number of each length in the given extent
 * Clamp so that neither length goes below 10 rows, or above 990
 */
extern u32 split_memory(u32 len1, u32 len2, u32 ext);

#endif
