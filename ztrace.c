/*
 * $Id: ztrace.c,v 1.5 2002/07/09 09:08:12 jon Exp $
 *
 * Compute the trace of a matrix
 *
 */

#include <stdio.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "parse.h"
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
  unsigned int i, mask, elts_per_word;
  const header *h;
  unsigned int *row;
  prime_ops prime_operations;
  row_ops row_operations;

  endian_init();
  memory_init(name, 0);
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    trace_usage();
    exit(1);
  }
  in = argv[1];
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
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor; i++) {
    unsigned int elt;
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, in);
      fclose(inp);
      exit(1);
    }
    elt = get_element_from_row_with_params(nob, i, mask, elts_per_word, row);
    put_element_to_row(nob, 0, &row2, elt);
    (*row_operations.incer)(&row2, &row1, 1);
  }
  fclose(inp);
  memory_dispose();
  elt = get_element_from_row(nob, 0, &row1);
  printf("%d\n", elt);
  return 0;
}
