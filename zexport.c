/*
 * $Id: zexport.c,v 1.1 2002/03/07 13:43:30 jon Exp $
 *
 * Export matrix to old system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zexport";

static void export_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  const header *h_in;
  header *h_out;
  unsigned int prime, nob, noc, nor, nod, i, j, *row, len;
  FILE *f_in;
  FILE *f_out;

  if (3 != argc) {
    export_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  if (0 == open_and_read_binary_header(&f_in, &h_in, in, name)) {
    fprintf(stderr, "%s: failed to open %s for input, terminating\n", name, in);
    exit(1);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  nod = header_get_nod(h_in);
  len = header_get_len(h_in);
  memory_init(name, 0);
  endian_init();
  if (memory_rows(len, 1) < 1) {
    fprintf(stderr, "%s: cannot fit a row of %s for input, terminating\n", name, in);
    exit(1);
  }
  row = memory_pointer(0);
  h_out = header_create(prime, nob, nod, noc, nor);
  header_set_prime(h_out, endian_invert(prime));
  header_set_noc(h_out, endian_invert(noc));
  header_set_nor(h_out, endian_invert(nor));
  if (0 == open_and_write_binary_header(&f_out, h_out, out, name)) {
    fprintf(stderr, "%s: failed to open %s for output, terminating\n", name, out);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(f_in, row, len)) {
      fprintf(stderr, "%s: failed to read row %d from %s, terminating\n", name, i, in);
      exit(1);
    }
    /* TODO: check this for prime != 2 */
    for (j = 0; j < len * sizeof(unsigned int); j++) {
      ((unsigned char *)row)[j] = convert_char(((unsigned char *)row)[j]);
    }
    if (0 == endian_write_row(f_out, row, len)) {
      fprintf(stderr, "%s: failed to write row %d to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  memory_dispose();
  return 0;
}
