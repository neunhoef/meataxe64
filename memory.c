/*
 * $Id: memory.c,v 1.14 2003/02/10 23:20:55 jon Exp $
 *
 * Large memory manipulation for meataxe
 *
 */

#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include "utils.h"

static unsigned int *memory = NULL;
static size_t extent = 0;

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * 1000 is used */
void memory_init(const char *name, size_t size)
{
  assert(NULL == memory);
  extent = ((0 != size) ? size : (MEM_SIZE));
  extent /= sizeof(unsigned int);
  errno = 0;
  if (UINT_MAX / (1000 * sizeof(unsigned int)) < extent) {
    fprintf(stderr, "%s: memory request %u exceeds system maximum, exiting\n", name, extent * sizeof(unsigned int));
    exit(1);
  }
  memory = malloc(extent * 1000 * sizeof(unsigned int));
  if (NULL == memory) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to allocate %u bytes, exiting\n", name, extent * 1000 * sizeof(unsigned int));
    exit(1);
  }
}


/* Dispose of the memory */
void memory_dispose(void)
{
  assert(NULL != memory);
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
/* len is number of words per row */

void *memory_pointer_offset(unsigned int n, unsigned int i, unsigned int len)
{
  unsigned int offset = i * len + n * extent;
  assert(n < 1000);
  assert(0 != len);
  assert(offset + len <= 1000 * extent);
  return memory + offset;
}

unsigned int memory_rows(unsigned int len, unsigned int size)
{
  assert(0 != len);
  assert(1000 >= size);
  return (size * extent) / len;
}

unsigned int find_extent(unsigned int nor, unsigned int len)
{
  unsigned int avail, required = 1000;
  assert(0 != nor);
  assert(0 != len);
  avail = memory_rows(len, 1000);
  if (avail < nor) {
    return 1001;
  } else {
    required = 1000 * nor / avail + 1;
  }
  if (required > 1000) {
    required = 1000;
  }
  while (required > 0 && memory_rows(len, required) > nor) {
    required--;
  }
  return required + 1;
}

unsigned int split_memory(unsigned int len1, unsigned int len2, unsigned int ext)
{
  unsigned int total = len1 + len2;
  unsigned int res;
  assert(ext >= 20 && ext <= 1000);
  assert(0 != len1);
  assert(0 != len2);
  assert(total > len1 && total > len2); /* Check we don't integer overflow */
  res = (len1 * ext) / total;
  if (res < 10) {
    res = 10;
  } else if (res + 10 > ext) {
    res = ext - 10;
  }
  return res;
}
