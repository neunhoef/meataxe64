/*
 * $Id: memory.c,v 1.1 2001/09/16 20:20:39 jon Exp $
 *
 * Large memory manipulation for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "memory.h"

static unsigned char *memory = NULL;
static size_t extent = 0;

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * (1 << 20) is used */
void memory_init(const char *name, size_t size)
{
  extent = ((0 != size) ? size : (MEM_SIZE));
  assert(NULL == memory);
  memory = malloc(extent * 1000);
  if (NULL == memory) {
    fprintf(stderr, "%s: failed to allocated %d bytes, exiting\n", name, extent * 1000);
    exit(1);
  }
}


/* Dispose of the memory */
void memory_dispose(void)
{
  free(memory);
  memory = NULL;
}

/* Get a pointer to the first row n thousandths of the way through the memory */
void *memory_pointer(unsigned int n)
{
  assert(n < 1000);
  return memory + n * extent;
}

/* Get a pointer to the ith row of given size starting n thousandths of the way through the memory */
/* i is number of bytes per row, rounded up to a multiple of four */
void *memory_pointer_offset(unsigned int n, unsigned int i, unsigned int len)
{
  assert(n < 1000);
  assert(0 == len % 4);
  assert((i + 1) * len + n * extent <= 1000 * extent);
  return memory + n * extent + i * len;
}

unsigned int memory_rows(unsigned int len, unsigned int size)
{
  assert(0 == len % 4);
  assert(1000 >= size);
  return (size * extent) / len;
}
