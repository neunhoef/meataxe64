/*
 * $Id: lv.c,v 1.3 2002/06/28 08:39:16 jon Exp $
 *
 * Function to lift vectors from a quotient space
 *
 */

#include "lv.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "ss_map.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

void lift(const char *range, const char *vectors,
          const char *out, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, nor_o, noc_i, len_i, noc_o, len, len_o, i, j, elt;
  unsigned int *row_in, *row_out;
  int *map;
  unsigned int *col_incs;
  assert(NULL != range);
  assert(NULL != vectors);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, vectors, name)) {
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  nob = header_get_nob(h_in1);
  nor = header_get_nor(h_in1);
  noc = header_get_noc(h_in1);
  len = header_get_len(h_in1);
  /* One entry for each incoming and for each subspace row */
  noc_i = header_get_noc(h_in2);
  len_i = header_get_len(h_in2);
  noc_o = noc_i + nor;
  nor_o = header_get_nor(h_in2);
  if (noc != noc_o ||
      prime != header_get_prime(h_in2) ||
      nob != header_get_nob(h_in2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, vectors);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (nor > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, vectors);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate space for two rows for %s, terminating\n",
            name, range);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  row_in = memory_pointer(0);
  row_out = memory_pointer(500);
  h_out = header_create(prime, nob, header_get_nod(h_in1), noc_o, nor_o);
  len_o = header_get_len(h_out);
  assert(len == len_o);
  header_free(h_in1);
  header_free(h_in2);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  header_free(h_out);
  map = my_malloc(nor * sizeof(int));
  col_incs = my_malloc(noc * sizeof(unsigned int));
  if (0 == subspace_map(inp1, map, nor, len, nob, row_in, range, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  /* inp1 no longer necessary */
  fclose(inp1);
  /* Set up the column increment points */
  memset(col_incs, 0, noc * sizeof(unsigned int));
  for (i = 0; i < nor; i += 1) {
    int k = map[i];
    assert(0 <= k && (unsigned int)k < noc);
    /* There's a column to be inserted here */
    assert(0 == col_incs[k]);
    col_incs[k]++;
  }
  /* Now step through vectors computing lifted representation */
  for (i = 0; i < nor_o; i += 1) {
    /* Initialise output row */
    unsigned int l = 0;
    row_init(row_out, len_o);
    /* Read row of vectors */
    errno = 0;
    if (0 == endian_read_row(inp2, row_in, len_i)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, vectors);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
    /* Set the elements */
    for (j = 0; j < noc; j++) {
      if (0 == col_incs[j]) {
        /* This column is in the quotient */
        elt = get_element_from_row(nob, l, row_in);
        put_element_to_row(nob, j, row_out, elt);
        l++;
      } else {
        /* This column is not in the quotient, so leave a zero here */
      }
    }
    errno = 0;
    if (0 == endian_write_row(outp, row_out, len_o)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, out);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
  }
  fclose(inp2);
  fclose(outp);
}
