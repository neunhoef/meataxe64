#include <stdio.h>
#include "malloc.h"
void *mymalloc(size_t x)
{
  void *ret = (malloc)(x);
  if (NULL == ret) {
    fprintf(stderr, "Failed to allocate %lu bytes, exiting\n", x);
    exit(100);
  }
  return ret;
}
