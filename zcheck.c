/*
 * $Id: zcheck.c,v 1.1 2001/12/23 23:31:42 jon Exp $
 *
 * Check no non-zero values off ends of rows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

static const char *name = "zcheck";

static void check_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  const header *h;
  unsigned int prime, nob, noc, nor, len, elts_per_word, *row, i, j;

  if (2 != argc) {
    check_usage();
    exit(1);
  }
  in = argv[1];
  memory_init(name, 0);
  endian_init();
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  header_free(h);
  elts_per_word = bits_in_unsigned_int / nob;
  if (0 != noc % elts_per_word){
    /* Possible left over bits, need to check */
    if (memory_rows(len, 1000) < 1) {
      fprintf(stderr, "%s: cannot fit one row into memory, terminating\n", name);
      exit(1);
    }
    row = memory_pointer(0);
    for (i = 0; i < nor; i++) {
      if (0 == endian_read_row(inp, row, len)) {
        fprintf(stderr, "%s: cannot read one row from %s, terminating\n", name, in);
        exit(1);
      }
      assert(noc < len * elts_per_word);
      for (j = noc; j < len * elts_per_word; j++) {
        unsigned int elt = get_element_from_row(nob, j, row);
        if (0 != elt) {
          fprintf(stderr, "%s: found value %d at offset %d (>= %d) in row %d of %s\n",
                  name, elt, j, noc, i, in);
        }
      }
    }
  }

  memory_dispose();
  return 0;
}
