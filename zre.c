/*
 * $Id: zre.c,v 1.1 2001/11/29 01:13:09 jon Exp $
 *
 * Convert a matrix from new to old
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "zre";

static void re_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp, *outp;
  unsigned int prime, nod, noc, nor, nob, len;
  unsigned int i;
  const header *h_in;
  header *h_out;
  unsigned int *row;
  prime_ops prime_operations;

  endian_init();
  memory_init(name, 0);
  if (3 != argc) {
    re_usage();
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h_in, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  len = header_get_len(h_in);
  h_out = header_create(prime, nob, nod, noc, nor);
  header_set_prime(h_out, endian_invert(prime));
  header_set_nob(h_out, endian_invert(nob));
  header_set_noc(h_out, endian_invert(noc));
  header_set_nor(h_out, endian_invert(nor));
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, argv[1]);
    fclose(inp);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, argv[2], name)) {
    fprintf(stderr, "%s: cannot open or write binary header to %s, terminating\n", name, argv[2]);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    unsigned int j;
    unsigned char *char_row = (unsigned char *)row;
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, argv[1]);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    /* Invert the bits */
    for (j = 0; j < len * sizeof(unsigned int); j++) {
      char_row[j] = convert_char(char_row[j]);
    }
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, i, argv[2]);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    /* Invert the bits and write out */
  }
  fclose(inp);
  fclose(outp);
  memory_dispose();
  return 0;
}
