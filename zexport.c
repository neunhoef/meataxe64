/*
 * $Id: zexport.c,v 1.6 2002/06/27 08:24:08 jon Exp $
 *
 * Export matrix to old system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
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
  unsigned int prime, nob, noc, nor, eperb, i, j, *in_row, len, blen;
  unsigned int *out_row;
  FILE *f_in;
  FILE *f_out;

  if (3 != argc) {
    export_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  if (0 == open_and_read_binary_header(&f_in, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(f_in);
    header_free(h_in);
    exit(1);
  }
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  eperb = header_get_eperb(h_in);
  blen = header_get_blen(h_in);
  memory_init(name, 0);
  endian_init();
  if (memory_rows(len, 500) < 1 || memory_rows((blen + sizeof(unsigned int) - 1) / sizeof(unsigned int), 500) < 1) {
    fprintf(stderr, "%s: cannot fit row of %s for input and row of %s for output, terminating\n", name, in, out);
    exit(1);
  }
  in_row = memory_pointer(0);
  out_row = memory_pointer(500);
  if (0 == open_and_write_binary_header(&f_out, h_in, out, name)) {
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(f_in, in_row, len)) {
      fprintf(stderr, "%s: failed to read row %d from %s, terminating\n", name, i, in);
      exit(1);
    }
    row_init(out_row, (blen + sizeof(unsigned int) - 1) / (sizeof(unsigned int)));
    for (j = 0; j < noc; j++) {
      unsigned int elt = get_element_from_row(nob, j, in_row);
      put_element_to_char_row(eperb, prime, j, (char *)out_row, elt);
    }
    if (blen != fwrite(out_row, 1, blen, f_out)) {
      fprintf(stderr, "%s: failed to write row %d to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  header_free(h_in);
  memory_dispose();
  return 0;
}
