/*
 * $Id: zqf.c,v 1.10 2005/06/22 21:52:54 jon Exp $
 *
 * Clear to zero the upper triangle of a matrix, including the diagonal
 * This is a utility program for computation of quadratic forms
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "write.h"

static const char *name = "zqf";

static void qf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 i, j;
  const char *in, *out;
  FILE *inp, *outp;
  const header *h;
  u32 prime, nob, nor, noc, len, elts_per_word;
  word mask, *row;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    qf_usage();
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
    for (j = 0; j <= i; j++) {
      put_element_to_row_with_params(nob, j, mask, elts_per_word, row, 0);
      /* Clear lower triangle, including diagonal */
    }
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
