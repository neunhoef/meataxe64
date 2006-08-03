/*
 * $Id: extend_matrix.c,v 1.6 2006/08/03 21:45:12 jon Exp $
 *
 * Function to field extend a matrix
 *
 */

#include "extend_matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "extend.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

int extend_matrix(const char *in, const char *out, u32 out_prime, const char *name)
{
  u32 nor, in_prime, noc, nod, in_nob, out_nob, in_len, out_len, i;
  word *in_row, *out_row;
  const header *h;
  header *h_out;
  FILE *inp, *outp;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != name);
  assert(0 != out_prime);
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    return 0;
  }
  in_prime = header_get_prime(h);
  if (1 == in_prime) {
    /* This is just a copy */
    if (0 == open_and_write_binary_header(&outp, h, out, name)) {
      header_free(h);
      fclose(inp);
      return 0;
    }
    copy_rest(outp, inp);
    fclose(inp);
    fclose(outp);
    header_free(h);
    return 0;
  }
  in_nob = header_get_nob(h);
  nor = header_get_nor(h);
  in_len = header_get_len(h);
  noc = header_get_noc(h);
  out_nob = bits_of(out_prime);
  nod = digits_of(out_prime);
  h_out = header_create(out_prime, out_nob, nod, noc, nor);
  out_len = header_get_len(h_out);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fclose(inp);
    header_free(h_out);
    return 0;
  }
  header_free(h_out);
  if (memory_rows(in_len, 500) < 1 || memory_rows(out_len, 500) < 1) {
    fprintf(stderr, "%s: cannot allocate memory for input, terminating\n", name);
    return 0;
  }
  in_row = memory_pointer_offset(0, 0, in_len);
  out_row = memory_pointer_offset(500, 0, out_len);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, in_row, in_len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row from %s, terminating\n", name, in);
      fclose(inp);
      fclose(outp);
      return 0;
    }
    if (0 == extend(in_row, out_row, in_nob, out_nob, in_prime, out_prime, noc, name)) {
      fclose(inp);
      fclose(outp);
      return 0;
    }
    errno = 0;
    if (0 == endian_write_row(outp, out_row, out_len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row to %s, terminating\n", name, out);
      fclose(inp);
      fclose(outp);
      return 0;
    }
  }
  fclose(inp);
  fclose(outp);
  return 1;
}
