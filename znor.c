/*
 * $Id: znor.c,v 1.1 2001/11/29 01:13:09 jon Exp $
 *
 * Print the number of rows of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "header.h"
#include "read.h"

static const char *name = "znor";

static void nor_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  unsigned int nor;
  const header *h;

  endian_init();
  if (2 != argc) {
    nor_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  nor = header_get_nor(h);
  header_free(h);
  printf("%d\n", nor);
  fclose(inp);
  return 0;
}
