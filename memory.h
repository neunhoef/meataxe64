/*
 * $Id: memory.h,v 1.1 2001/09/12 23:13:04 jon Exp $
 *
 * Large memory manipulation for meataxe
 *
 */

#ifndef included__memory
#define included__memory

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * (1 << 20) is used */
extern void memory_init(size_t);

/* Dispose of the memory */
extern void memory_dispose(void);

/* Get a pointer to the first row n thousandths of the way through the memory */
extern char *memory_pointer(unsigned int);

/* Get a pointer to the ith row of given size starting n thousandths of the way through the memory */
extern char *memory_pointer(unsigned int n, unsigned int i);

#endif
