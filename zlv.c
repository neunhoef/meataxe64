/*
 * $Id: zlv.c,v 1.3 2002/10/14 19:11:51 jon Exp $: zss.c,v 1.1 2001/11/25 00:17:19 jon Exp $
 *
 * Calculate inverse quotient space representation, ie lift
 *
 */

#include <stdio.h>
#include "endian.h"
#include "lv.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zlv";

static void lv_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <range> <vectors> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    lv_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  lift(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
