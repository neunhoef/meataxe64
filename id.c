/*
 * $Id: id.c,v 1.5 2001/09/18 23:15:46 jon Exp $: ad.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Generate identity matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "id";

static void id_usage(void)
{
  fprintf(stderr, "%s: usage: %s <prime> <nor> <noc> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, nob, nod, noc, nor, len;
  unsigned int i;
  unsigned int *row;
  FILE *outp;
  const header *h;

  if (5 != argc) {
    id_usage();
    exit(1);
  }
  out = argv[4];
  if (0 == read_decimal(argv[1], strlen(argv[1]), &prime)) {
    fprintf(stderr, "%s: failed to read prime from command line\n", name);
    return 0;
  }
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: non prime %d\n", name, prime);
    exit(1);
  }
  if (0 == read_decimal(argv[2], strlen(argv[2]), &nor)) {
    fprintf(stderr, "Failed to read nor from command line\n");
    return 0;
  }
  if (0 == read_decimal(argv[3], strlen(argv[3]), &noc)) {
    fprintf(stderr, "Failed to read noc from command line\n");
    return 0;
  }
  nob = bits_of(prime);
  nod = digits_of(prime);
  endian_init();
  memory_init(name, 0);
  outp = fopen(out, "wb");
  if (0 == outp) {
    fprintf(stderr, "%s: cannot open %s\n", name, out);
    exit(1);
  }
  h = header_create(prime, nob, nod, noc, nor);
  len = header_get_len(h);
  if (0 == write_binary_header(outp, h, out)) {
    fprintf(stderr, "%s: cannot write header\n", name);
    fclose(outp);
    exit(1);
  }
  if (memory_rows(len, 1000) < 3) {
    fprintf(stderr, "%s: cannot create output row\n", name);
    fclose(outp);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    row_init(row, len);
    if (i < noc) {
      put_element_to_row(nob, i, row, 1);
    }
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s: write output row to %s\n", name, out);
      fclose(outp);
      exit(1);
    }
  }
  fclose(outp);
  memory_dispose();
  return 0;
}
