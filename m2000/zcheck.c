/*
 * $Id: zcheck.c,v 1.9 2005/07/24 09:32:46 jon Exp $
 *
 * Check no non-zero values off ends of rows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "utils.h"

static const char *name = "zcheck";

static void check_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  const header *h;
  u32 prime, nob, noc, nor, len, i, j, elts_per_word;
  word *row, mask;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    check_usage();
    exit(1);
  }
  in = argv[1];
  memory_init(name, memory);
  endian_init();
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
  header_free(h);
  mask = get_mask_and_elts(nob, &elts_per_word);
  if (0 != noc % elts_per_word){
    /* Possible left over bits, need to check */
    if (memory_rows(len, 1000) < 1) {
      fprintf(stderr, "%s: cannot fit one row into memory, terminating\n", name);
      exit(1);
    }
    row = memory_pointer(0);
    for (i = 0; i < nor; i++) {
      errno = 0;
      if (0 == endian_read_row(inp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read one row from %s, terminating\n", name, in);
        exit(1);
      }
      assert(noc < len * elts_per_word);
      for (j = noc; j < len * elts_per_word; j++) {
        word elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, row);
        if (0 != elt) {
          fprintf(stderr, "%s: found value %u at offset %u (>= %u) in row %u of %s\n",
                  name, (unsigned int)elt, j, noc, i, in);
        }
      }
    }
  }

  memory_dispose();
  return 0;
}
