/*
 * $Id: zdiag.c,v 1.8 2004/01/04 21:22:50 jon Exp $
 *
 * Clear to zero the off diagonal elements of a matrix
 * This is a utility program for computation of quadratic forms
 *
 */

#include <stdio.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "memory.h"
#include "header.h"
#include "parse.h"
#include "read.h"
#include "rows.h"
#include "write.h"

static const char *name = "zdiag";

static void diag_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int i;
  const char *in, *out;
  FILE *inp, *outp;
  const header *h;
  unsigned int prime, nob, nor, noc, len, elt, *row, mask, elts_per_word;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    diag_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  memory_init(name, memory);
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    header_free(h);
    exit(1);
  }
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, argv[1]);
    header_free(h);
    fclose(inp);
    exit(2);
  }
  row = memory_pointer_offset(0, 0, len);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    header_free(h);
    fclose(inp);
    exit(1);
  }
  header_free(h);
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, in);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    elt = get_element_from_row_with_params(nob, i, mask, elts_per_word, row);
    row_init(row, len);
    put_element_to_clean_row_with_params(nob, i, elts_per_word, row, elt);
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  memory_dispose();
  return 0;
}
