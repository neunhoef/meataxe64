/*
 * $Id: zcv64to32.c,v 1.3 2005/10/12 18:20:31 jon Exp $
 *
 * Convert from the 64 bit word meataxe to the 32 bit word meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zcvu64to32";

static void cv_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  u32 prime, nob, noc, nor, u32len, u64len;
  u32 i, j;
  const header *h;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    cv_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  inp = fopen(in, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s for input\n", name, in);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h, in, name)) {
    fclose(inp);
    fprintf(stderr, "%s: cannot read header from %s\n", name, in);
    exit(1);
  }
  if (PRIME_BIT != (header_get_raw_prime(h) & PRIME_BITS)) {
    fclose(inp);
    header_free(h);
    fprintf(stderr, "%s: %s is not a 64 bit meataxe file\n", name, in);
    exit(1);
  }
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  nob = header_get_nob(h);
  u32len = header_get_u32_len(h);
  u64len = header_get_u64_len(h);
  header_set_raw_prime((header *)h, prime);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    header_free(h);
    fprintf(stderr, "%s: failed to open %s for output\n", name, in);
    exit(1);
  }
  header_free(h);
  if (1 == prime) {
    /* Do permutation stuff */
    u64 elt_in;
    u32 elt_out;
    for (i = 0; i < nor; i++) {
      if (0 == endian_read_u64(&elt_in, inp)) {
        fclose(inp);
        fclose(outp);
        fprintf(stderr, "%s: failed to read u64 %u from %s\n", name, i, in);
        exit(1);
      }
      elt_out = elt_in;
      if (0 == endian_write_u32(elt_out, outp)) {
        fclose(inp);
        fclose(outp);
        fprintf(stderr, "%s: failed to write u32 %u to %s\n", name, i, out);
        exit(1);
      }
    }
  } else {
    /* Do standard file stuff */
    u64 *row_in = my_malloc(u64len * sizeof(u64));
    u32 *row_out = my_malloc(u32len * sizeof(u32));
    for (i = 0; i < nor; i++) {
      if (0 == endian_read_u64_row(inp, row_in, u64len)) {
        fclose(inp);
        fclose(outp);
        fprintf(stderr, "%s: failed to read row %u from %s\n", name, i, in);
        exit(1);
      }
      memset(row_out, 0, u32len * sizeof(u32));
      for (j = 0; j < noc; j++) {
        u64 elt = get_element_from_u64_row(nob, j, row_in);
        put_element_to_u32_row(nob, j, row_out, (u32)elt);
      }
      if (0 == endian_write_u32_row(outp, row_out, u32len)) {
        fclose(inp);
        fclose(outp);
        fprintf(stderr, "%s: failed to write row %u to %s\n", name, i, out);
        exit(1);
      }
    }
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
