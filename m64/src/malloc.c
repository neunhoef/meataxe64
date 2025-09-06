#include <stdio.h>
#include "malloc.h"
#include "parse.h"
void *mymalloc(size_t x)
{
  void *ret = (malloc)(x);
  if (NULL == ret) {
    fprintf(stderr, "Failed to allocate %lu bytes, exiting\n", x);
    exit(100);
  }
  if (verbose && x >= 0x80000000UL) {
    printf("Large allocation of %lu\n", x);
    fflush(stdout);
  }
  return ret;
}
