/*
 * $Id: ztrecover.c,v 1.2 2002/09/21 16:59:46 jon Exp $
 *
 * Recover a tensor subspace
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mv.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "ztrecover";

static void trecover_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <cols> <cols2> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp, *outp;
  unsigned int memory, noc1, noc2, nor, nob, nod, prime, len2, len, len_o, *row_in, *row_out, d, **mat_rows;
  const header *h_in;
  header *h_out, *h_2;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc && 6 != argc) {
    trecover_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  noc1 = strtoul(argv[3], NULL, 0);
  noc2 = strtoul(argv[4], NULL, 0);
  if (0 == noc1 || 0 == noc2) {
    fprintf(stderr, "%s: zero columns not acceptable, terminating\n", name);
    exit(1);
  }
  if (6 == argc) {
    memory = strtoul(argv[5], NULL, 0);
  }
  memory_init(name, 0);
  endian_init();
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    exit(1);
  }
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor = header_get_nor(h_in);
  prime = header_get_prime(h_in);
  len = header_get_len(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: maps cannot be recovered, terminating\n", name);
    fclose(inp);
    header_free(h_in);
    exit(1);
  }
  h_2 = header_create(prime, nob, nod, noc2, noc2);
  len2 = header_get_len(h_2);
  header_free(h_2);
  if (len2 * noc1 != len) {
    fprintf(stderr, "%s: incompatible row lengths %d, %d\n", name, len2 * noc1, len);
    fclose(inp);
    header_free(h_in);
    exit(1);
  }
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: insufficient space for %s\n", name, in);
    fclose(inp);
    header_free(h_in);
    exit(1);
  }
  row_in = memory_pointer(0);
  row_out = memory_pointer(500);
  h_out = header_create(prime, nob, nod, noc1 * noc2, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fclose(inp);
    header_free(h_in);
    header_free(h_out);
    exit(1);
  }
  len_o = header_get_len(h_out);
  header_free(h_in);
  header_free(h_out);
  mat_rows = matrix_malloc(noc1);
  create_pointers(row_in, mat_rows, noc1, len2, prime);
  for (d = 0; d < nor; d++) {
    errno = 0;
    if (0 == endian_read_row(inp, row_in, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: unable to read %s, terminating\n", name, in);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    m_to_v(mat_rows, row_out, noc1, noc2, prime);
    errno = 0;
    if (0 == endian_write_row(outp, row_out, len_o)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: unable to write %s, terminating\n", name, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  matrix_free(mat_rows);
  memory_dispose();
  /* TODO: free h_in at the right point */
  return 0;
}
