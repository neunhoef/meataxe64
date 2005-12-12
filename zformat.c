/*
 * $Id: zformat.c,v 1.1 2005/12/12 23:16:03 jon Exp $
 *
 * Indicate the meataxe format of a file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zformat";

static void format_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  u32 raw_prime, size;
  const header *h;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    format_usage();
    exit(1);
  }
  in = argv[1];
  inp = fopen(in, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s for input\n", name, in);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h, in, name)) {
    fclose(inp);
    fprintf(stderr, "%s: cannot read header from %s\n", name, in);
    exit(1);
  }
  raw_prime = header_get_raw_prime(h);
  if (raw_prime & PRIME_BITS) {
    if ((raw_prime & PRIME_BITS) == U64PRIME_BIT) {
      size = 64;
    } else {
      fprintf(stderr, "%s: unknown size designator 0x%x\n", name, raw_prime & PRIME_BITS);
      exit(1);
    }
  } else {
    size = 32;
  }
  header_free(h);
  fclose(inp);
  printf("%u bit meataxe\n", size);
  return 0;
}
