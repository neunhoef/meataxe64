#include <stdlib.h>
extern void *mymalloc(size_t x);

#define malloc(x) mymalloc(x)
