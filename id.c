/*
 * $Id: id.c,v 1.1 2001/09/02 22:16:41 jon Exp $: ad.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Generate identity matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "header.h"
#include "write.h"
#include "rows.h"
#include "elements.h"
#include "utils.h"

static void id_usage(void)
{
  fprintf(stderr, "id: usage: id <prime> <nor> <noc> <out_file>\n");
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, nob, nod, noc, nor;
  unsigned int i;
  unsigned int *row;
  FILE *outp;
  header h;

  if (5 != argc) {
    id_usage();
    exit(1);
  }
  out = argv[4];
  prime = read_decimal(argv[1], strlen(argv[1]));
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "id: non prime %d\n", prime);
    exit(1);
  }
  nor = read_decimal(argv[2], strlen(argv[2]));
  noc = read_decimal(argv[3], strlen(argv[3]));
  nob = bits_of(prime);
  nod = digits_of(prime);
  endian_init();
  outp = fopen(out, "wb");
  if (0 == outp) {
    fprintf(stderr, "id: cannot open %s\n", out);
    exit(1);
  }
  h = header_create(prime, nob, nod, noc, nor);
  write_binary_header(outp, h, out);
  if (0 == row_malloc(nob, noc, &row)) {
    fprintf(stderr, "id: cannot create output row\n");
    fclose(outp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    row_init(row, nob, noc);
    if (i < noc) {
      put_element_to_row(nob, i, row, 1);
    }
    if (0 == endian_write_row(outp, row, nob, noc)) {
      fprintf(stderr, "id: write output row to %s\n", out);
      fclose(outp);
      exit(1);
    }
  }
  fclose(outp);
  return 1;
}
