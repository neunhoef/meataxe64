/*
 * $Id: zqf.c,v 1.2 2002/04/10 23:33:27 jon Exp $
 *
 * Clear to zero the upper triangle of a matrix, including the diagonal
 * This is a utility program for computation of quadratic forms
 *
 */

#include <stdio.h>
#include "elements.h"
#include "endian.h"
#include "memory.h"
#include "header.h"
#include "read.h"
#include "write.h"

static const char *name = "zqf";

static void qf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int i, j;
  const char *in, *out;
  FILE *inp, *outp;
  const header *h;
  unsigned int prime, nob, nor, noc, len, *row;

  if (3 != argc) {
    qf_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  memory_init(name, MEM_SIZE);
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
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, in);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    for (j = i; j < noc; j++) {
      put_element_to_row(nob, j, row, 0);
      /* Clear upper traingle, including diagonal */
    }
    if (0 == endian_write_row(outp, row, len)) {
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
