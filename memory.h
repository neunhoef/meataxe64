/*
 * $Id: memory.h,v 1.6 2003/02/10 23:20:55 jon Exp $
 *
 * Large memory manipulation for meataxe
 *
 */

#ifndef included__memory
#define included__memory

#include <stdlib.h>

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * (1 << 20) is used */
extern void memory_init(const char *, size_t);

/* Dispose of the memory */
extern void memory_dispose(void);

/* Get a pointer to the first row n thousandths of the way through the memory */
extern void *memory_pointer(unsigned int);

/* Get a pointer to the ith row of given size starting n thousandths of the way through the memory */
/* len is in words */
extern void *memory_pointer_offset(unsigned int n, unsigned int i, unsigned int len);

/* Find out how many rows of given size fit in given memory */
/* len is in words */
extern unsigned int memory_rows(unsigned int len, unsigned int size);

/* Find out how much memory is required for a given number of rows */
extern unsigned int find_extent(unsigned int nor, unsigned int len);

/* Compute the amount of memory required for rows of length len1
 * Such that we can have the same number of each length in the given extent
 * Clamp so that neither length goes below 10 rows, or above 990
 */
extern unsigned int split_memory(unsigned int len1, unsigned int len2, unsigned int ext);

#endif
