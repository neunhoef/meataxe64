/*
 * $Id: zimport.c,v 1.10 2004/01/24 14:47:13 jon Exp $
 *
 * Import matrix from old system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "zimport";

static void import_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  const header *h_in;
  unsigned int prime, nob, noc, nor, eperb, i, j, *out_row, len, blen, elts_per_word;
  unsigned char *in_row;
  FILE *f_in;
  FILE *f_out;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    import_usage();
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
  memory_init(name, 0);
  endian_init();
  blen = header_get_blen(h_in);
  if (memory_rows(len, 500) < 1 || memory_rows((blen + sizeof(unsigned int) - 1) / sizeof(unsigned int), 500) < 1) {
    fprintf(stderr, "%s: cannot fit row of %s for input and row of %s for output, terminating\n", name, in, out);
    exit(1);
  }
  in_row = memory_pointer(0);
  out_row = memory_pointer(500);
  if (0 == open_and_write_binary_header(&f_out, h_in, out, name)) {
    exit(1);
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor; i++) {
    if (blen != fread(in_row, 1, blen, f_in)) {
      fprintf(stderr, "%s: failed to read row %d from %s, terminating\n", name, i, in);
      exit(1);
    }
    row_init(out_row, len);
    for (j = 0; j < noc; j++) {
      unsigned int elt = get_element_from_char_row(eperb, prime, j, in_row);
      put_element_to_clean_row_with_params(nob, j, elts_per_word, out_row, elt);
    }
    errno = 0;
    if (0 == endian_write_row(f_out, out_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write row %d to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  header_free(h_in);
  memory_dispose();
  return 0;
}
