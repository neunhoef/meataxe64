/*
 * Add a header to an intermediate tensor spin file matrix
 * Essentially a disaster recovery program
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mv.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zath";

static void ath_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <nor> <noc1> <noc2> <prime>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  u32 prime, nob, nor, noc1, noc2, noc, in_len, out_len, memory = 0;
  word *in_row, *out_row;
  word **rows;
  u32 i, j;
  s64 ptr;
  const header *h;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc) {
    ath_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  memory_init(name, memory);
  endian_init();
  errno = 0;
  nor = strtoul(argv[3], NULL, 0);
  noc1 = strtoul(argv[4], NULL, 0);
  noc2 = strtoul(argv[5], NULL, 0);
  prime = strtoul(argv[6], NULL, 0);
  if (0 == nor || 0 == noc1 || 0 == noc2 || 0 == is_a_prime_power(prime)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: bad parametrs amongst %s, %s, %s, %s, terminating\n", name, argv[3], argv[4], argv[5], argv[6]);
    exit(1);
  }
  noc = noc1 * noc2;

  errno = 0;
  inp = fopen(in, "r");
  if (NULL == inp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  nob = bits_of(prime);
  in_len = compute_len(nob, noc2);
  h = header_create(prime, nob, 1, noc, nor);
  out_len = header_get_len(h);
  if (memory_rows(in_len * noc1, 1000) < 2) {
    fprintf(stderr, "%s: cannot obtain memory for two rows of %s, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  in_row = memory_pointer(0);
  out_row = memory_pointer(500);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    header_free(h);
    exit(1);
  }

  /*
   * We read sets of noc1 rows of length noc2 and glue them together
   * into single rows of length noc, then output them
   */
  rows = matrix_malloc(noc1);
  create_pointers(in_row, rows, noc1, in_len);
  for (i = 0; i < nor; i++) {
    for (j = 0; j < noc1; j++) {
      errno = 0;
      if (0 == endian_read_row(inp, rows[j], in_len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row %u from %s, terminating\n", name, i, in);
        fclose(inp);
        fclose(outp);
        exit(1);
      }
    }
    m_to_v(rows, out_row, noc1, noc2, prime);
    errno = 0;
    if (0 == endian_write_row(outp, out_row, out_len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row %u to %s, terminating\n", name, i, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  ptr = ftello(inp);
  fseeko(inp, 0, SEEK_END);
  if (ftello(inp) != ptr) {
    fprintf(stderr, "%s: not all of input %s read, terminating\n", name, in);
    fclose(inp);
    fclose(outp);
    (void)remove(out);
    exit(1);
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
