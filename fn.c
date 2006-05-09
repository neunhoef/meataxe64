/*
 * $Id: fn.c,v 1.1 2006/05/09 22:04:10 jon Exp $
 *
 * Filter a list based on nullity
 *
 */

#include "fn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "gen.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int filter_nullity(const char *in_list, const char *out_list, u32 nullity,
                   unsigned int argc, const char * const args[],
                   const char *name)
{
  u32 noc = 0, nor = 0, nob = 0, prime = 0, len = 0, i, j, coeffs_nor, coeffs_len, nor_out = 0;
  const header *h;
  gen gens;
  word ***elements;
  word **rows;
  FILE *in_listp, *out_listp;
  row_ops row_operations;
  prime_ops prime_operations;
  NOT_USED(nullity);
  assert(NULL != in_list);
  assert(NULL != out_list);
  assert(NULL != args);
  assert(NULL != name);
  assert(0 != argc);
  /* Read args headers and check */
  gens = my_malloc(argc * sizeof(*gens));
  for (i = 0; i < argc; i++) {
    gens[i].m = args[i];
    if (0 == open_and_read_binary_header(&gens[i].f, &h, args[i], name)) {
      for (j = 0; j < i; j++) {
        fclose(gens[j].f);
      }
      free(gens);
      return 1;
    }
    if (0 == i) {
      nor = header_get_nor(h);
      noc = header_get_noc(h);
      len = header_get_len(h);
      prime = header_get_prime(h);
      nob = header_get_nob(h);
    } else {
      if (header_get_nor(h) != nor ||
          header_get_noc(h) != noc ||
          header_get_prime(h) != prime) {
        fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, gens[0].m, gens[i].m);
        for (j = 0; j < i; j++) {
          fclose(gens[j].f);
        }
        free(gens);
        return 1;
      }
    }
  }
  header_free(h);
  /* Allocate memory */
  if (0 == rows_init(prime, &row_operations) ||
      0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: prime or row initialisation failed, terminating\n", name);
    for (j = 0; j < i; j++) {
      fclose(gens[j].f);
    }
    free(gens);
    return 1;
  }
  i = memory_rows(len, 1000);
  if (i < (argc + 2) * nor) {
    /* Not enough memory to read all generators */
    fprintf(stderr, "%s: insufficient memory for generators and workspace, terminating\n", name);
    for (j = 0; j < i; j++) {
      fclose(gens[j].f);
    }
    free(gens);
    return 2;
  }
  elements = my_malloc((argc + 2) * sizeof(*elements));
  rows = matrix_malloc((argc + 2) * nor);
  for (i = 0; i < nor * (argc + 2); i++) {
    rows[i] = memory_pointer_offset(0, i, len);
  }
  for (i = 0; i < argc + 2; i++) {
    elements[i] = rows + i * nor; /* Pointers to the matrices */
  }
  /* Read args and close */
  for ( i = 0; i < argc; i++) {
    errno = 0;
    if (0 == endian_read_matrix(gens[i].f, elements[i], len, nor)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, gens[i].m);
      for (j = i; j < argc; j++) {
        fclose(gens[j].f);
      }
      free(gens);
      matrix_free(rows);
      free(elements);
      return 1;
    }
    fclose(gens[i].f);
  }
  free(gens);
  /* Open list and output */
  if (0 == open_and_read_binary_header(&in_listp, &h, in_list, name)) {
    matrix_free(rows);
    free(elements);
    return 1;
  }
  if (header_get_noc(h) != argc) {
    fprintf(stderr, "%s: columns and args don't match, terminating\n", name);
    matrix_free(rows);
    free(elements);
    fclose(in_listp);
    header_free(h);
    return 1;
  }
  coeffs_nor = header_get_nor(h);
  coeffs_len = header_get_len(h);
  if (header_get_prime(h) != prime) {
    matrix_free(rows);
    free(elements);
    fclose(in_listp);
    header_free(h);
    return 1;
  }
  if (0 == open_and_write_binary_header(&out_listp, h, out_list, name)) {
    matrix_free(rows);
    free(elements);
    fclose(in_listp);
    header_free(h);
    return 1;
  }
  /* Loop: Read list element */
  while (coeffs_nor > 0) {
    /* Create ident in output area (elements[argc]) */
    word **rows_id = elements[argc];
    word *row, *coeffs_row;
    unsigned int rank = 0;
    for (i = 0; i < nor; i++) {
      row = rows_id[i];
      row_init(row, len);
      put_element_to_row(nob, i, row, 1);
    }
    /* TODO: read a row from in_listp */
    coeffs_row = elements[argc + 1][0];
    errno = 0;
    if (0 == endian_read_row(in_listp, coeffs_row, coeffs_len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, in_list);
      matrix_free(rows);
      free(elements);
      fclose(in_listp);
      fclose(out_listp);
      /* TODO: delete out_list */
      header_free(h);
      return 1;
    }
    /* Add up */
    for (i = 0; i < argc; i++) {
      word elt = get_element_from_row(nob, i, coeffs_row);
      for (j = 0; j < nor; j++) {
        if (0 != elt) {
          if (1 == elt) {
            (*row_operations.incer)(elements[i][j], rows_id[j], len);
          } else {
            (*row_operations.scaled_incer)(elements[i][j], rows_id[j], len, elt);
          }
        }
      }
    }
    /* Compute in memory rank */
    for (i = 0; i < nor; i++) {
      u32 pos;
      word elt = first_non_zero(rows_id[i], nob, len, &pos);
      if (0 != elt) {
        rank++;
        /* Invert */
        if (1 != elt) {
          elt = (*prime_operations.invert)(elt);
          /* Scale */
          (*row_operations.scaler_in_place)(rows_id[i], len, elt);
        }
        /* Clean */
        for (j = i + 1; j < nor; j++) {
          elt = get_element_from_row(nob, pos, rows_id[j]);
          if (0 != elt) {
            elt = (*prime_operations.negate)(elt);
            if (1 != elt) {
              (*row_operations.scaled_incer)(rows_id[i], rows_id[j], len, elt);
            } else {
              (*row_operations.incer)(rows_id[i], rows_id[j], len);
            }
          }
        }
      }
    }
    if (rank + nullity == nor) {
      /* Output or discard */
      nor_out++;
      /* write out row */
      errno = 0;
      if (0 == endian_write_row(out_listp, coeffs_row, coeffs_len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, in_list);
        matrix_free(rows);
        free(elements);
        fclose(in_listp);
        fclose(out_listp);
        /* TODO: delete out_list */
        header_free(h);
        return 1;
      }
    }
    coeffs_nor--;
  }
  /* TODO: rewrite header for out_list */
  matrix_free(rows);
  free(elements);
  fclose(in_listp);
  header_set_nor((header *)h, nor_out);
  fseeko64(out_listp, 0, SEEK_SET);
  if (0 == write_binary_header(out_listp, h, out_list, name)) {
    fclose(out_listp);
    header_free(h);
    return 1;
  }
  header_free(h);
  fclose(out_listp);
  return 0;
}
