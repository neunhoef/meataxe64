/*
 * $Id: zse.c,v 1.1 2001/11/29 01:13:09 jon Exp $
 *
 * Select a row of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zsel";

static void sel_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <row number>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  unsigned int prime, nob, noc, nor, len;
  unsigned int i, vector;
  const header *h_in, *h_out;
  unsigned int *row;

  if (4 != argc) {
    sel_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  vector = strtoul(argv[3], NULL, 0);
  inp = fopen(in, "r");
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  prime = header_get_prime(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  nob = header_get_nob(h_in);
  len = header_get_len(h_in);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, 1);
  if (vector >= nor) {
    fprintf(stderr, "%s: vector %d requested outside of matrix %s, terminating\n", name, vector, in);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, out);
    exit(1);
  }
  endian_init();
  memory_init(name, 0);
  header_free(h_in);
  header_free(h_out);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate 1 row for %s, terminating\n", name, in);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i <= vector; i++) {
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s: failed to read row %d to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  if (0 == endian_write_row(outp, row, len)) {
    fprintf(stderr, "%s: failed to write row %d to %s, terminating\n", name, i, out);
    exit(1);
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
