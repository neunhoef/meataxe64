/*
 * $Id: znexp.c,v 1.1 2012/03/24 13:32:21 jon Exp $
 *
 * Export matrix to meataxe 64 system
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "nheader.h"
#include "parse.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "nwrite.h"

#define align_new_row_length(len) ((len) + ((len) % 2))

static const char *name = "znexp";

static void nexp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  const header *h_in;
  nheader *h_out;
  u32 prime, nob, noc, nor, i, len, out_len;
  word *in_row, *out_row;
  FILE *f_in;
  FILE *f_out;
  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    nexp_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  if (0 == open_and_read_binary_header(&f_in, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(f_in);
    header_free(h_in);
    exit(1);
  }
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  /* Compute the length in the output */
  /* Should be abstracted into nheader. but for the moment put it here */
  out_len = align_new_row_length(len);
  if (verbose) {
    printf("Aligning new row length to %" U32_F " from %" U32_F "\n", out_len, len);
  }
  memory_init(name, memory);
  if (memory_rows(len, 500) < 1 || memory_rows(out_len, 500) < 1) {
    fprintf(stderr, "%s: cannot fit row of %s for input and row of %s for output, terminating\n", name, in, out);
    exit(1);
  }
  in_row = memory_pointer(0);
  out_row = memory_pointer(500);
  /* TBD: Create new header */
  h_out = nheader_create(prime, nob, header_get_nod(h_in), noc, nor);
  /* Open the output, and write the new header */
  if (0 == open_and_write_binary_nheader(&f_out, h_out, out, name)) {
    exit(1);
  }
  /* Finally, copy the input to the output */
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(f_in, in_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row %u from %s, terminating\n", name, i, in);
      exit(1);
    }
    /* Copy the main content */
    memcpy(out_row, in_row, len * sizeof(word));
    if (out_len > len) {
      /* Clear the bit on the end */
      memset(out_row + len, 0, sizeof(word));
    }
    if (out_len != fwrite(out_row, sizeof(word), out_len, f_out)) {
      fprintf(stderr, "%s: failed to write row %u to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  header_free(h_in);
  nheader_free(h_out);
  memory_dispose();
  return 0;
}
