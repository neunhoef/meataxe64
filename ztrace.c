/*
 * $Id: ztrace.c,v 1.1 2001/12/03 00:07:48 jon Exp $
 *
 * Compute the trace of a matrix
 *
 */

#include <stdio.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"

static const char *name = "ztrace";

static void trace_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  unsigned int prime, noc, nor, nob, len, row1, row2, elt;
  unsigned int i;
  const header *h;
  unsigned int *row;
  prime_ops prime_operations;
  row_ops row_operations;

  endian_init();
  memory_init(name, 0);
  if (2 != argc) {
    trace_usage();
    exit(1);
  }
  in = argv[1];
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  rows_init(prime, &row_operations);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  row_init(&row1, 1);
  row_init(&row2, 1);
  for (i = 0; i < nor; i++) {
    unsigned int elt;
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, in);
      fclose(inp);
      exit(1);
    }
    elt = get_element_from_row(nob, i, row);
    put_element_to_row(nob, 0, &row2, elt);
    (*row_operations.incer)(&row2, &row1, 1);
  }
  fclose(inp);
  memory_dispose();
  elt = get_element_from_row(nob, 0, &row1);
  printf("%d\n", elt);
  return 0;
}
