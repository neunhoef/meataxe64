/*
 * $Id: base.c,v 1.3 2002/06/28 08:39:16 jon Exp $
 *
 * Form an echelised basis from one file to another
 *
 */

#include "base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "clean_file.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "system.h"
#include "utils.h"
#include "write.h"

static void cleanup_tmp(FILE *echelised, const char *name_echelised)
{
  assert(NULL != echelised);
  assert(NULL != name_echelised);
  fclose(echelised);
  (void)remove(name_echelised);
}

unsigned int base(const char *in, const char *dir,
                  const char *out, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *echelised = NULL;
  const header *h_in;
  header *h_out;
  unsigned int prime, nob, noc, nor = 0, nor1, len, d, max_rows;
  unsigned int **rows1, **rows2;
  int *map;
  int tmps_created = 0;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  char *name_echelised = NULL;
  const char *tmp = tmp_name();
  assert(NULL != in);
  assert(NULL != dir);
  assert(NULL != out);
  assert(NULL != name);
  /* TODO: open the input, and create an empty temporary file */
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor1 = header_get_nor(h_in);
  len = header_get_len(h_in);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: prime %d from %s is not a prime power, terminating\n",
            name, prime, in);
    exit(1);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, noc);
  header_free(h_in);
  d = strlen(tmp) + strlen(dir);
  name_echelised = my_malloc(d + 4);
  sprintf(name_echelised, "%s/%s.1", dir, tmp);
  /* Create the temporary file */
  errno = 0;
  echelised = fopen64(name_echelised, "w+b");
  if (NULL == echelised) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_echelised);
    fclose(inp);
    exit(1);
  }
  tmps_created = 1;
  /* Set up the map for the echelised basis */
  map = my_malloc(nor1 * sizeof(int));
  /* Initialise arithmetic */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  /* Work out how many rows we can handle in store */
  max_rows = memory_rows(len, 450);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    fclose(inp);
    cleanup_tmp(echelised, name_echelised);
    exit(2);
  }
  /* Give up if too few rows available */
  if (max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to get %d rows for %s, terminating\n",
            name, 2 * (prime + 1), in);
    fclose(inp);
    cleanup_tmp(echelised, name_echelised);
    exit(2);
  }
  max_rows = (max_rows > noc) ? noc : max_rows; /* Can never need more than this */
  /* Set up the pointers to the workspace rows */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  for (d = 0; d < nor1; d += max_rows) {
    unsigned int stride = (d + max_rows > nor1) ? nor1 - d : max_rows;
    errno = 0;
    if (0 == endian_read_matrix(inp, rows1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, in);
      fclose(inp);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
    if (0 == clean_file(echelised, &nor, rows1, stride, rows2, max_rows,
                        map, NULL, 0, grease.level, prime, len, nob, 900, name)) {
      fclose(inp);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
  }
  fclose(inp);
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup_tmp(echelised, name_echelised);
    exit(1);
  }
  header_free(h_out);
  fseeko64(echelised, 0, SEEK_SET);
  copy_rest(outp, echelised);
  fclose(outp);
  cleanup_tmp(echelised, name_echelised);
  matrix_free(rows1);
  matrix_free(rows2);
  free(map);
  return nor;
}
