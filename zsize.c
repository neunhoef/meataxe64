/*
 * $Id: zsize.c,v 1.1 2005/12/12 08:55:58 jon Exp $
 *
 * Print the number of bits in a meataxe word
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "parse.h"

static const char *name = "zsize";

static void size_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (1 != argc) {
    size_usage();
    exit(1);
  }
  printf("%u bit meataxe\n", sizeof(word) * CHAR_BIT);
  return 0;
}
