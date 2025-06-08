/*
 * $Id: memory.c,v 1.19 2018/05/16 07:51:26 jon Exp $
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

static word *memory = NULL;
static size_t extent = 0;

/* Initialise the memory system with given size */
/* If a size of 0 is given, the default value MEM_SIZE * 1000 is used */
void memory_init(const char *name, size_t size)
{
  assert(NULL == memory);
  extent = ((0 != size) ? size : (MEM_SIZE));
  extent /= sizeof(word);
  errno = 0;
  if (SIZE_T_MAX / (1000 * sizeof(word)) < extent) {
    fprintf(stderr, "%s: memory request %" SIZE_F " exceeds system maximum, exiting\n", name, extent * sizeof(word));
    exit(1);
  }
  memory = malloc(extent * 1000 * sizeof(word));
  if (NULL == memory) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to allocate %" SIZE_F " bytes, exiting\n", name, extent * 1000 * sizeof(word));
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
word *memory_pointer(u32 n)
{
  assert(n < 1000);
  return memory + n * extent;
}

/* Get a pointer to the ith row of given size starting n thousandths of the way through the memory */
/* len is number of words per row */

word *memory_pointer_offset(u32 n, u32 i, u32 len)
{
  size_t offset = i * (size_t)len + n * extent;
  assert(n < 1000);
  assert(0 != len);
  assert(offset + len <= 1000 * extent);
  return memory + offset;
}

u32 memory_rows(u32 len, u32 size)
{
  assert(0 != len);
  assert(1000 >= size);
  return (size * extent) / len;
}

u32 find_extent(u32 nor, u32 len)
{
  u32 avail, required = 1000;
  assert(0 != nor);
  assert(0 != len);
  avail = memory_rows(len, 1000);
  if (avail < nor) {
    return 1001;
  } else {
    /* Avoid integer overflow below */
    u64 total = 1000 * (u64)nor;
    required = total / avail + 1;
  }
  if (required > 1000) {
    required = 1000;
  }
  while (required > 0 && memory_rows(len, required) > nor) {
    required--;
  }
  return required + 1;
}

u32 split_memory(u32 len1, u32 len2, u32 ext)
{
  u32 total = len1 + len2;
  u32 res;
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
