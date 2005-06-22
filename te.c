/*
 * $Id: te.c,v 1.10 2005/06/22 21:52:54 jon Exp $
 *
 * Function to tensor two matrices to give a third
 *
 */

#include "te.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "map_or_row.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int cleanup(FILE *inp1, FILE *inp2, FILE *outp)
{
  if (NULL != inp1)
    fclose(inp1);
  if (NULL != inp2)
    fclose(inp2);
  if (NULL != outp)
    fclose(outp);
  return 0;
}

int tensor(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  u32 prime, nob, nod, noc1, nor1, len1, noc2, nor2, len2, noc3, nor3, len3, elts_per_word;
  word mask;
  const header *h1 = NULL, *h2 = NULL, *h3 = NULL;
  u32 i, j, k, l;
  word **rows1, **rows2, *row_out, *row_copy;
  row_ops row_operations;
  int is_perm1, is_perm2;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != h1) {
      header_free(h1);
    }
    return cleanup(inp1, inp2, outp);
  }
  prime = header_get_prime(h1);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
  if (nor1 != noc1 ||
      nor2 != noc2) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  nor3 = nor1 * nor2;
  noc3 = nor3;
  is_perm1 = (1 == prime);
  is_perm2 = (1 == header_get_prime(h2));
  if (is_perm1 && is_perm2) {
    /* A pair of maps, result is also a map */
    word *map1 = malloc_map(nor1);
    word *map2 = malloc_map(nor2);
    word *map_o = malloc_map(nor3);
    header_free(h1);
    header_free(h2);
    if (0 == read_map(inp1, nor1, map1, name, m1) ||
        0 == read_map(inp2, nor2, map2, name, m2)) {
      map_free(map1);
      map_free(map2);
      map_free(map_o);
      return cleanup(inp1, inp2, outp);
    }
    fclose(inp1);
    fclose(inp2);
    for (i = 0; i < nor1; i++) {
      /* Down rows of m1 */
      for (j = 0; j < nor2; j++) {
        /* Down rows of m2 */
        map_o[i * nor2 + j] = map1[i] * nor2 + map2[j];
      }
    }
    map_free(map1);
    map_free(map2);
    h3 = header_create(1, 0, 0, noc3, nor3);
    if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
      map_free(map_o);
      return cleanup(NULL, NULL, outp);
    }
    if (0 == write_map(outp, nor3, map_o, name, m3)) {
      map_free(map_o);
      return cleanup(NULL, NULL, outp);
    }
    map_free(map_o);
  } else {
    nob = (is_perm1) ? header_get_nob(h2) : header_get_nob(h1);
    nod = (is_perm1) ? header_get_nod(h2) : header_get_nod(h1);
    prime = (is_perm1) ? header_get_prime(h2) : header_get_prime(h1);
    mask = get_mask_and_elts(nob, &elts_per_word);
    if (is_perm1) {
      const header *h = header_create(prime, nob, nod, noc1, nor1);
      len1 = header_get_len(h);
      header_free(h);
    } else {
      len1 = header_get_len(h1);
    }
    if (is_perm2) {
      const header *h = header_create(prime, nob, nod, noc2, nor2);
      len2 = header_get_len(h);
      header_free(h);
    } else {
      len2 = header_get_len(h2);
    }
    if ((0 == is_perm1 && 0 == is_perm2) &&
        (header_get_prime(h2) != prime ||
         header_get_nob(h2) != nob)) {
      fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
      header_free(h1);
      header_free(h2);
      return cleanup(inp1, inp2, NULL);
    }
    h3 = header_create(prime, nob, nod, noc3, nor3);
    len3 = header_get_len(h3);
    if (memory_rows(len1, 450) < nor1 ||
        memory_rows(len2, 450) < nor2 ||
        memory_rows(len3, 100) < 2) {
      fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
      (void) cleanup(inp1, inp2, outp);
      exit(2);
    }
    rows1 = matrix_malloc(nor1);
    rows2 = matrix_malloc(nor2);
    for (i = 0; i < nor1; i++) {
      rows1[i] = memory_pointer_offset(0, i, len1);
    }
    for (i = 0; i < nor2; i++) {
      rows2[i] = memory_pointer_offset(450, i, len2);
    }
    row_out = memory_pointer(900);
    row_copy = memory_pointer(950);
    if (0 == read_rows(is_perm1, inp1, rows1, nob, noc1, len1,
                       nor1, m1, name) ||
        0 == read_rows(is_perm2, inp2, rows2, nob, noc2, len2,
                       nor2, m2, name)) {
      return cleanup(inp1, inp2, outp);
    }
    if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
      return cleanup(inp1, inp2, outp);
    }
    fclose(inp1);
    fclose(inp2);
    header_free(h1);
    header_free(h2);
    header_free(h3);
    rows_init(prime, &row_operations);
    for (i = 0; i < nor1; i++) {
      /* Down the rows of m1 */
      for (j = 0; j < nor2; j++) {
        /* Down the rows of m2 */
        u32 offset = 0;
        row_init(row_out, len3);
        for (k = 0; k < noc1; k++) {
          /* Along the columns of m1 */
          word elt = get_element_from_row_with_params(nob, k, mask, elts_per_word, rows1[i]);
          if ( 0 != elt) {
            word *row = rows2[j];
            if (1 != elt) {
              (*row_operations.scaler)(row, row_copy, len2, elt);
              row = row_copy;
            }
            for (l = 0; l < noc2; l++) {
              elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, row);
              put_element_to_clean_row_with_params(nob, offset + l, elts_per_word, row_out, elt);
            }
          }
          offset += noc2;
        }
        errno = 0;
        if (0 == endian_write_row(outp, row_out, len3)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m3);
          fclose(outp);
          return 0;
        }
      }
    }
    matrix_free(rows1);
    matrix_free(rows2);
  }
  fclose(outp);
  return 1;
}
