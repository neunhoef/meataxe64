/*
 * $Id: zlv.c,v 1.1 2002/06/25 10:30:12 jon Exp $: zss.c,v 1.1 2001/11/25 00:17:19 jon Exp $
 *
 * Calculate inverse quotient space representation, ie lift
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "lv.h"

static const char *name = "zlv";

static void lv_usage(void)
{
  fprintf(stderr, "%s: usage: %s <range> <vectors> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    lv_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  lift(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
