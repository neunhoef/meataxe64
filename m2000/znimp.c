/*
 * $Id: znimp.c,v 1.1 2012/06/17 10:41:29 jon Exp $
 *
 * Import matrix from meataxe 64 system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "nheader.h"
#include "parse.h"
#include "nread.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

#define align_new_row_length(len) ((len) + ((len) % 2))

static const char *name = "znimp";

static void nimp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  const nheader *h_in;
  header *h_out;
  u64 prime, nob, noc, nor, i, in_len;
  u32 len;
  word *in_row, *out_row;
  FILE *f_in;
  FILE *f_out;
  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    nimp_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  if (0 == open_and_read_binary_nheader(&f_in, &h_in, in, name)) {
    exit(1);
  }
  prime = nheader_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(f_in);
    nheader_free(h_in);
    exit(1);
  }
  nob = nheader_get_nob(h_in);
  noc = nheader_get_noc(h_in);
  nor = nheader_get_nor(h_in);
  /* Create new header */
  h_out = header_create((u32)prime, (u32)nob,
                        (u32)nheader_get_nod(h_in), (u32)noc,
                        (u32)nor);
  len = header_get_len(h_out);
  /* Compute the length in the input */
  /* Should be abstracted into nheader. but for the moment put it here */
  in_len = align_new_row_length(len);
  assert(len <= in_len);
  if (verbose) {
    printf("Aligning new row length to %" U32_F " from %" U64_F "\n", len, in_len);
  }
  memory_init(name, memory);
  if (memory_rows(in_len, 500) < 1 || memory_rows(len, 500) < 1) {
    fprintf(stderr, "%s: cannot fit row of %s for input and row of %s for output, terminating\n", name, in, out);
    exit(1);
  }
  in_row = memory_pointer(0);
  out_row = memory_pointer(500);
  h_out = header_create(prime, nob, nheader_get_nod(h_in), noc, nor);
  /* Open the output, and write the new header */
  if (0 == open_and_write_binary_header(&f_out, h_out, out, name)) {
    exit(1);
  }
  /* Finally, copy the input to the output */
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (in_len != fread(in_row, sizeof(word), in_len, f_in)) {
      fprintf(stderr, "%s: failed to read row %" U64_F " from %s, terminating\n", name, i, in);
      exit(1);
    }
    /* Copy the main content */
    memcpy(out_row, in_row, len * sizeof(word));
    if (0 == endian_write_row(f_out, out_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row %" U64_F " from %s, terminating\n", name, i, in);
      exit(1);
    }
  }
  nheader_free(h_in);
  header_free(h_out);
  memory_dispose();
  return 0;
}
