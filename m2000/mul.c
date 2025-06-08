/*
 * $Id: mul.c,v 1.54 2019/01/21 08:32:34 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

#include "mul.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

#define M1_SIZE 450
#define M2_SIZE 100

#define contract(elts,prime,nob) ((2 == (prime)) ? (elts) : elements_contract(elts, prime, nob))

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

static int mul_from_maps(FILE *inp1, FILE *inp2, const header *h1, const header *h2,
                         const char *m1, const char *m2, const char *m3, const char *name)
{
  u32 nor1, nor2, noc2;
  word *map1, *map2, *map3;
  header *h3;
  FILE *outp;
  assert(NULL != inp1);
  assert(NULL != inp2);
  assert(NULL != h1);
  assert(NULL != h2);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  nor1 = header_get_nor(h1);
  nor2 = header_get_nor(h2);
  noc2 = header_get_noc(h2);
  map1 = malloc_map(nor1);
  map2 = malloc_map(nor2);
  map3 = malloc_map(nor1);
  if (0 == read_map(inp1, nor1, map1, name, m1) ||
      0 == read_map(inp2, nor2, map2, name, m2) ||
      0 == mul_map(map1, map2, map3, h1, h2, name)) {
    header_free(h1);
    header_free(h2);
    map_free(map1);
    map_free(map2);
    map_free(map3);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  fclose(inp1);
  fclose(inp2);
  map_free(map1);
  map_free(map2);
  h3 = header_create(1, 0, 0, noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    return 0;
  }
  if (0 == write_map(outp, nor1, map3, name, m3)) {
    header_free(h3);
    return cleanup(NULL, NULL, outp);
  }
  header_free(h3);
  map_free(map3);
  fclose(outp);
  return 1;
}

static int mul_row_by_map(word *row_in, word *row_out, word *map,
                          u32 noc, u32 len, u32 nob,
                          u32 noc_o, prime_opsp operations, const char *m, const char *name)
{
  u32 j, elts_per_word;
  word mask;
  assert(NULL != row_in);
  assert(NULL != row_out);
  assert(NULL != map);
  assert(NULL != operations);
  assert(NULL != m);
  assert(0 != len);
  assert(0 != noc);
  assert(0 != nob);
  assert(0 != noc_o);
  mask = get_mask_and_elts(nob, &elts_per_word);
  row_init(row_out, len);
  for (j = 0; j < noc; j++) {
    word elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, row_in);
    if (0 != elt) {
      word k = map[j], elt2;
      if (k >= noc_o) {
        fprintf(stderr, "%s: element %u from map %s out of range (0 - %u), terminating\n", name, (u32)k, m, noc_o - 1);
        return 0;
      }
      elt2 = get_element_from_row_with_params(nob, k, mask, elts_per_word, row_out);
      if (0 != elt2) {
        elt = (*operations->add)(elt, elt2);
      }
      put_element_to_clean_row_with_params(nob, k, elts_per_word, row_out, elt);
    }
  }
  return 1;
}

static int store_mul_rows_by_map(word **row_in, word **row_out,
                                 word *map,
                                 u32 noc, u32 nor, u32 len, u32 nob,
                                 u32 noc_o, prime_opsp operations,
                                 const char *m, const char *name)
{
  u32 i;
  assert(NULL != row_in);
  assert(NULL != row_out);
  assert(NULL != map);
  assert(NULL != operations);
  assert(NULL != name);
  for (i = 0; i < nor; i++) {
    if (0 == mul_row_by_map(row_in[i], row_out[i], map, noc, len, nob, noc_o, operations, m, name)) {
      return 0;
    }
  }
  return 1;
}

static int mul_rows_by_map(word **row_in, word **row_out, FILE *inp,
                           u32 noc, u32 nor, u32 len, u32 nob,
                           u32 noc_o, prime_opsp operations, const char *m, const char *name)
{
  word *map;
  int ret;
  assert(NULL != row_in);
  assert(NULL != row_out);
  assert(NULL != inp);
  assert(NULL != operations);
  assert(NULL != m);
  assert(NULL != name);
  map = malloc_map(noc);
  if (0 == read_map(inp, noc, map, m, name)) {
    map_free(map);
    return 0;
  }
  ret = store_mul_rows_by_map(row_in, row_out, map, noc, nor, len, nob,
                              noc_o, operations, m, name);
  map_free(map);
  return ret;
}

static int mul_by_map(FILE *inp1, FILE *inp2, const header *h1, const header *h2,
                      const char *m1, const char *m2, const char *m3, const char *name)
{
  u32 nor1, noc1, nor2, noc2, prime, nob, nod, len1, len3, len, i;
  word *map2, *row1, *row2;
  header *h3;
  FILE *outp;
  prime_ops operations;
  assert(NULL != inp1);
  assert(NULL != inp2);
  assert(NULL != h1);
  assert(NULL != h2);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  nor2 = header_get_nor(h2);
  noc2 = header_get_noc(h2);
  nob = header_get_nob(h1);
  nod = header_get_nod(h1);
  len1 = header_get_len(h1);
  map2 = malloc_map(nor2);
  if (0 == read_map(inp2, nor2, map2, name, m2)) {
    header_free(h1);
    header_free(h2);
    map_free(map2);
    return cleanup(inp1, inp2, NULL);
  }
  prime = header_get_prime(h1);
  header_free(h1);
  header_free(h2);
  fclose(inp2);
  if (noc1 != nor2 ||
      0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %s has bad prime power %u, terminating\n", name, m1, prime);
    map_free(map2);
    return cleanup(inp1, NULL, NULL);
  }
  h3 = header_create(prime, nob, nod, noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    header_free(h3);
    return cleanup(inp1, NULL, NULL);
  }
  len3 = header_get_len(h3);
  header_free(h3);
  len = (len1 > len3) ? len1 : len3;
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate rows for %s * %s, terminating\n", name, m1, m2);
    map_free(map2);
    return cleanup(inp1, NULL, outp);
  }
  row1 = memory_pointer(0);
  row2 = memory_pointer(500);
  if (0 == primes_init(prime, &operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations for %s * %s, terminating\n", name, m1, m2);
    return cleanup(inp1, NULL, outp);
  }
  for (i = 0; i < nor1; i++) {
    errno = 0;
    if (0 == endian_read_row(inp1, row1, len1)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, m1);
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
    if (0 == mul_row_by_map(row1, row2, map2, noc1, len3, nob, noc2, &operations, m2, name)) {
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
    errno = 0;
    if (0 == endian_write_row(outp, row2, len3)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, m3);
      map_free(map2);
      return cleanup(inp1, NULL, outp);
    }
  }
  map_free(map2);
  fclose(inp1);
  fclose(outp);
  return 1;
}

int mul(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  u32 prime1, prime2, nob, noc1, nor1, len1, noc2, nor2, len2;
  u32 k, j;
  u32 nox1, nox2, nox3, nox;
  const header *h1 = NULL, *h2 = NULL, *h3 = NULL;
  word **rows1, **rows3;
  int is_perm1, is_perm2;
  row_ops row_operations;
  grease_struct grease;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != h1) {
      header_free(h1);
    }
    return cleanup(inp1, inp2, NULL);
  }
  prime1 = header_get_prime(h1);
  prime2 = header_get_prime(h2);
  is_perm1 = (1 == prime1);
  is_perm2 = (1 == prime2);
  nob = header_get_nob(h2);
  nor1 = header_get_nor(h1);
  noc1 = header_get_noc(h1);
  len1 = header_get_len(h1);
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
  len2 = header_get_len(h2);
  if (is_perm2) {
    if (is_perm1) {
      return mul_from_maps(inp1, inp2, h1, h2, m1, m2, m3, name);
    } else {
      return mul_by_map(inp1, inp2, h1, h2, m1, m2, m3, name);
    }
  }
  /* We still have the possibility that m1 is a permutation */
  if ((0 == is_perm1 &&
      (header_get_nob(h1) != nob ||
       prime1 != prime2)) ||
      nor2 != noc1) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  h3 = header_create(prime2, nob, header_get_nod(h1), noc2, nor1);
  if (0 == open_and_write_binary_header(&outp, h3, m3, name)) {
    header_free(h3);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h3);
  if (0 == rows_init(prime2, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp1, inp2, outp);
  }
  grease_init(&row_operations, &grease);
  if (is_perm1) {
    len1 = compute_len(nob, noc1);
  }
  nox1 = memory_rows(len1, M1_SIZE);
  nox3 = memory_rows(len2, M1_SIZE); /* len3 = len2 */
  nox = (nox1 > nox3) ? nox3 : nox1; /* Deal with the one with bigger rows */
  nox = (nox > nor1) ? nor1 : nox; /* Only deal with as many rows as we have */
  nox2 = memory_rows(len2, M2_SIZE);
  if (0 == nox || 0 == nox2) {
    fprintf(stderr, "%s: cannot initialise input rows, terminating\n", name);
    return cleanup(inp1, inp2, outp);
  }
  /* Compute best lazy grease given nox2 */
  if (0 == grease_level(prime2, &grease, nox2)) {
    fprintf(stderr, "%s: cannot allocate grease space, terminating\n", name);
    exit(2);
  }
  /* Allocate the grease space */
  if (0 == grease_allocate(prime2, len2, &grease, M1_SIZE)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    return cleanup(inp1, inp2, outp);
  }
  rows1 = matrix_malloc(nox);
  rows3 = matrix_malloc(nox);
  for (k = 0; k < nox; k++) {
    rows1[k] = memory_pointer_offset(0, k, len1);
    rows3[k] = memory_pointer_offset(M1_SIZE + M2_SIZE, k, len2);
  }
  for (k = 0; k < nor1; k += nox) {
    u32 rest = (nor1 >= k + nox) ? nox : nor1 - k;
    /* Read matrix 1 */
    /* Convert permutation if necessary */
    if (is_perm1) {
      word *map = malloc_map(rest);
      if (0 == read_map(inp1, rest, map, name, m1)) {
        grease_free(&grease);
        map_free(map);
        return cleanup(inp1, inp2, outp);
      }
      for (j = 0; j < rest; j++) {
        word l = map[j];
        row_init(rows1[j], len1);
        if (l >= noc1) {
          fprintf(stderr, "%s: map entry %u out of range (0 - %u), terminating\n", name, (u32)l, noc1 - 1);
          grease_free(&grease);
          map_free(map);
          return cleanup(inp1, inp2, outp);
        }
        put_element_to_row(nob, l, rows1[j], 1);
      }
      map_free(map);
    } else {
      errno = 0;
      if (0 == endian_read_matrix(inp1, rows1, len1, rest)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: unable to read %s, terminating\n", name, m1);
        grease_free(&grease);
        return cleanup(inp1, inp2, outp);
      }
    }
    if (0 == mul_from_store(rows1, rows3, inp2, 0, noc1, len2, nob, rest, noc2, prime2, &grease, verbose, m2, name)) {
      fclose(inp2);
      fclose(outp);
      matrix_free(rows1);
      matrix_free(rows3);
      grease_free(&grease);
      return 0;
    }
    /* Write matrix 3 */
    errno = 0;
    if (0 == endian_write_matrix(outp, rows3, len2, rest)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: unable to write %s, terminating\n", name, m3);
      matrix_free(rows1);
      matrix_free(rows3);
      grease_free(&grease);
      return cleanup(inp1, inp2, outp);
    }
  }
  grease_free(&grease);
  matrix_free(rows1);
  matrix_free(rows3);
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 1;
}

#define LAZY_GREASE 1

int skip_mul_from_store(u32 offset, word **rows1, word **rows3,
                        FILE *inp, int is_map, u32 noc, u32 len,
                        u32 nob, u32 nor, u32 noc_o, u32 prime,
                        grease grease, int verbose, u32 *indexes,
                        const char *m, const char *name)
{
  s64 pos;
  u32 i, j, l, elts_per_word;
  assert(NULL != rows1);
  assert(NULL != rows3);
  assert(NULL != inp);
  assert(NULL != m);
  assert(NULL != name);
  assert(0 != noc);
  assert(is_a_prime_power(prime)); /* prime refers to rows1 */
  assert(0 != nob); /* nob refers to rows1 */
  if (verbose) {
    printf("%s: multiplying %u rows with %s\n", name, nor, m);
    fflush(stdout);
  }
  /* Remember where we are in row 2 */
  pos = ftello(inp);
  if (is_map) {
    /* Multiply some rows by a map */
    prime_ops operations;
    if (0 == primes_init(prime, &operations)) {
      fprintf(stderr, "%s: cannot initialise prime operations for %s, terminating\n", name, m);
      return 0;
    }
    if (0 == mul_rows_by_map(rows1, rows3, inp, noc, nor, len, nob,
                             noc_o, &operations, m, name)) {
      return 0;
    }
  } else {
    assert(0 != len); /* len may have come from m, and hence would be zero for a map */
    (void)get_mask_and_elts(nob, &elts_per_word);
    /* Now skip rows if necessary */
    for (i = 0; i < offset; i += 1) {
      endian_skip_row(inp, len);
    }
    /* Then multiply */
    for (j = 0; j < nor; j++) {
      row_init(rows3[j], len);
    }
    for (i = offset; i < noc; i += grease->level) {
      u32 size = (grease->level + i <= noc) ? grease->level : noc - i;
      u32 width = size * nob;
      u32 word_offset, bit_offset;
      word mask;
      int in_word;
      u32 shift = 0;
      u32 index = len, my_index = len;
      /* Read size rows from matrix 2 into rows 2 */
      /* This sets the initial rows */
      l = 1;
      for (j = 0; j < size; j++) {
        errno = 0;
        if (0 == endian_read_row(inp, grease->rows[l - 1], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: unable to read %s, terminating\n", name, m);
          return 0;
        }
        /*
         * Compute how much of matrix B we can skip
         * Only worth doing if we'll reuse the rows a bit
         */
        if (nor > 2 * grease->level) {
          /* Only do this if the row will be reused and there's some potential gain */
          if (NULL != indexes) {
            /* We already know where the rows start */
            my_index = indexes[i + j];
          } else {
            word *row = grease->rows[l - 1];
            word *end_row = row + index;
            while (0 == *row && row < end_row) {
              row++;
            }
            my_index = row - grease->rows[l - 1];
          }
          if (my_index < index) {
            index = my_index; /* Reduce if a row demands it */
          }
        } else {
          index = 0;
        }
        l *= prime;
      }
      element_access_init(nob, i, size, &word_offset, &bit_offset, &mask);
      in_word = bit_offset + width <= bits_in_word;
      if (0 == in_word) {
        shift = ((bits_in_word - bit_offset) / nob) * nob;
      }
      grease_init_rows(grease, prime);
      if (index < len) {
        /* We only do this if some non-zero rows exist */
        for (j = 0; j < nor; j++) {
          word *row1 = rows1[j];
          word elt = (in_word) ? get_elements_in_word_from_row(row1 + word_offset, bit_offset, mask) :
            get_elements_out_word_from_row(row1 + word_offset, shift, bit_offset, mask);
          if (0 != elt) {
            grease_row_inc(grease, len, rows3[j], contract(elt, prime, nob), index);
          }
        }
      }
    }
  }
  /* Move back in matrix 2 */
  if (0 != fseeko(inp, pos, SEEK_SET)) {
    fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m);
    return 0;
  }
  return 1;
}

int mul_from_store(word **rows1, word **rows3,
                   FILE *inp, int is_map, u32 noc, u32 len,
                   u32 nob, u32 nor, u32 noc_o, u32 prime,
                   grease grease, int verbose, const char *m, const char *name)
{
  return skip_mul_from_store(0, rows1, rows3, inp, is_map, noc, len,
                             nob, nor, noc_o, prime, grease, verbose, NULL, m, name);
}

/* Does not handle maps */
int mul_in_store(word **rows1, word **rows2, word **rows3,
                 u32 noc, u32 len,
                 u32 nob, u32 nor, u32 prime,
                 int preserve_rows, grease grease)
{
  u32 i, j, l;
  u32 level;
  word **grease_rows = NULL;
  u32 word_offset = 0, bit_offset = 0;
  u32 elts_per_word = bits_in_word / nob;
  u32 bits_per_word = elts_per_word * nob;
  u32 width, size, spare, end;
  word mask;
  word bit = 1;
  assert(NULL != rows1);
  assert(NULL != rows2);
  assert(NULL != rows3);
  assert(0 != noc);
  assert(0 != nob); /* nob refers to rows1 */
  /* len may have come from m2, and hence would be zero for a map */
  assert(0 != len);
  assert(NULL != grease);
  level = grease->level;
  size = level;
  width = level * nob;
  mask = (bit << width) - 1;
  spare = noc % level;
  end = noc - spare;
  if (preserve_rows) {
    /* Save the grease row pointers */
    grease_rows = matrix_malloc(level);
    l = 1;
    /* Replace the initial allocated grease rows with the rows of rows2 */
    for (j = 0; j < level; j++) {
      grease_rows[j] = grease->rows[l - 1];
      l *= prime;
    }
  }
  /* Initialise the output rows */
  for (j = 0; j < nor; j++) {
    row_init(rows3[j], len);
  }
  /* We will use our element instead */
  for (i = 0; i < end; i += level) {
    int in_word;
    u32 shift = bits_per_word - bit_offset;
    word **rows = grease->rows - 1;
    if (level + i > noc) {
      size = noc - i;
      width = size * nob;
      mask = (bit << width) - 1;
    }
    l = 1;
    /* Replace the initial allocated grease rows with the rows of rows2 */
    for (j = 0; j < size; j++) {
      rows[l] = rows2[i + j];
      l *= prime;
    }
    in_word = width <= shift;
    grease_init_rows(grease, prime);
    if (in_word) {
      for (j = 0; j < nor; j++) {
        word elt = get_elements_in_word_from_row(rows1[j] + word_offset, bit_offset, mask);
        if (0 != elt) {
          grease_row_inc(grease, len, rows3[j], contract(elt, prime, nob), 0);
        }
      }
    } else {
      for (j = 0; j < nor; j++) {
        word elt = get_elements_out_word_from_row(rows1[j] + word_offset, shift, bit_offset, mask);
        if (0 != elt) {
          grease_row_inc(grease, len, rows3[j], contract(elt, prime, nob), 0);
        }
      }
    }
    bit_offset += width;
    if (bit_offset >= bits_per_word) {
      word_offset++;
      bit_offset -= bits_per_word;
    }
  }
  if (0 != spare) {
    int in_word;
    u32 shift = bits_per_word - bit_offset;
    word **rows = grease->rows - 1;
    size = spare;
    width = size * nob;
    mask = (bit << width) - 1;
    l = 1;
    /* Replace the initial allocated grease rows with the rows of rows2 */
    for (j = 0; j < size; j++) {
      rows[l] = rows2[i + j];
      l *= prime;
    }
    in_word = width <= shift;
    grease_init_rows(grease, prime);
    for (j = 0; j < nor; j++) {
      word elt = (in_word) ? get_elements_in_word_from_row(rows1[j] + word_offset, bit_offset, mask) :
        get_elements_out_word_from_row(rows1[j] + word_offset, shift, bit_offset, mask);
      if (0 != elt) {
        grease_row_inc(grease, len, rows3[j], contract(elt, prime, nob), 0);
      }
    }
  }
  if (preserve_rows) {
    /* Restore the grease row pointers */
    l = 1;
    for (j = 0; j < level; j++) {
      grease->rows[l - 1] = grease_rows[j];
      l *= prime;
    }
    matrix_free(grease_rows);
  }
  return 1;
}
