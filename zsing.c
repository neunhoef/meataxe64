/*
 * $Id: zsing.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Compute a singular vector for a form on a space
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "singular.h"
#include "parse.h"

static const char *name = "zsing";

static void sing_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <space> <form> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int n;
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    sing_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = singular(argv[1], argv[2], argv[3], name);
  memory_dispose();
  if (0 == n) {
    return 0;
  } else if (255 == n) {
    return 255;
  } else {
    return 1;
  }
}
