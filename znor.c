/*
 * $Id: znor.c,v 1.3 2002/10/14 19:11:51 jon Exp $
 *
 * Print the number of rows of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "read.h"

static const char *name = "znor";

static void nor_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  unsigned int nor;
  const header *h;

  endian_init();
  argv = parse_line(argc, argv, &argc);
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
