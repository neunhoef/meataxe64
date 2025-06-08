/*
 * $Id: zsign.c,v 1.6 2004/08/21 13:22:32 jon Exp $
 *
 * Compute the orthogonal group sign of a form
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "sign.h"
#include "parse.h"

static const char *name = "zsign";

static void sign_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <quadratic form> <bilinear form>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int n;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    sign_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = sign(argv[1], argv[2], name);
  memory_dispose();
  if (0 == n) {
    return 0;
  } else if (255 == n) {
    return 255;
  } else {
    return 1;
  }
}
