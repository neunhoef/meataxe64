/*
 * $Id: memory.h,v 1.4 2001/11/07 22:35:27 jon Exp $
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

#endif
