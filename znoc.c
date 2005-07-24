/*
 * $Id: znoc.c,v 1.6 2005/07/24 09:32:46 jon Exp $
 *
 * Print the number of columns of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "read.h"

static const char *name = "znoc";

static void noc_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  FILE *inp;
  u32 noc;
  const header *h;

  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    noc_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  noc = header_get_noc(h);
  header_free(h);
  printf("%u\n", noc);
  fclose(inp);
  return 0;
}
