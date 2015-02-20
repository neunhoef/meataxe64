/*
 * Function compute the orthogonal group sign
 *
 */

#include "sign.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "indexes.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "singular.h"
#include "utils.h"
#include "write.h"

#define shuffle_rows

static void shuffle(u32 from, u32 to, word **mat)
{
  u32 i;
  word *row = mat[to];
  assert(from <= to);
  for (i = to; i > from; i--) {
    mat[i] = mat[i - 1];
  }
  mat[from] = row;
}

int sign(const char *qform, const char *bform, const char *name)
{
  FILE *qinp = NULL, *binp = NULL;
  const header *h_inq, *h_inb;
  u32 prime, nob, nor, noc, len, n;
  word **mat; /* Our space. We lose vectors off the top of this */
  u32 *q_indexes, *b_indexes = NULL;
  word *sing_row1, *sing_row2;
  u32 *products, out_num;
  u32 start = 0; /* Pointer into mat */
  u32 elts_per_word;
  int res;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != bform);
  assert(NULL != qform);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&qinp, &h_inq, qform, name) ||
      0 == open_and_read_binary_header(&binp, &h_inb, bform, name)) {
    if (NULL != qinp) {
      fclose(qinp);
      header_free(h_inq);
    }
    return 1;
  }
  prime = header_get_prime(h_inb);
  if (1 == prime || 1 == header_get_prime(h_inq)) {
    fprintf(stderr, "%s: form cannot be a map, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  nob = header_get_nob(h_inb);
  nor = header_get_nor(h_inb);
  noc = header_get_noc(h_inb);
  len = header_get_len(h_inb);
  if (noc != nor) {
    fprintf(stderr, "%s: form must be square, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  header_free(h_inb);
  if (header_get_nob(h_inq) != nob ||
      header_get_noc(h_inq) != noc ||
      header_get_nor(h_inq) != nor ||
      header_get_len(h_inq) != len ||
      header_get_prime(h_inq) != prime) {
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
  }
  header_free(h_inq);
  if (nor % 2 != 0) {
    fclose(qinp);
    fclose(binp);
    return 0; /* We'll call odd dimension + */
  }
  n = memory_rows(len, 100);
  if (memory_rows(len, 900) < nor + 5 || n < prime) {
    fprintf(stderr, "%s: cannot allocate %u rows, terminating\n",
            name, nor + prime + 5);
    fclose(qinp);
    fclose(binp);
    exit(2);
  }
  (void)grease_level(prime, &grease, n);
  if (grease.level > 1) {
    grease.level = 1;
  }
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  (void)get_mask_and_elts(nob, &elts_per_word);
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(qinp);
    fclose(binp);
    return 1;
  }
  /* Now allocate the matrix for the identity plus workspace */
  mat = matrix_malloc(nor + 3);
  for (n = 0; n < nor + 3; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  sing_row1 = memory_pointer_offset(0, nor + 3, len);
  sing_row2 = memory_pointer_offset(0, nor + 4, len);
  /*
   * Now get the indexes for q and b
   */
  q_indexes = my_malloc(sizeof(u32) * noc);
  if (0 == make_indexes(qinp, mat[0], q_indexes, noc, len, name, qform)) {
    fclose(qinp);
    fclose(binp);
    return 1;
  }
  /*
   * Now set up the identity (in reverse order, part of a cunning plan)
   * This is so we know early on how much of the intial part of each
   * row is zero, and can feed this to singular_vector,
   * skip_mul_from_store, and the row product function
   */
  for (n = 0; n < nor; n++) {
    row_init(mat[n], len);
    put_element_to_row(nob, nor - 1 - n, mat[n], 1);
  }
  products = my_malloc(nor * sizeof(u32));
  /* Whilst there are 3 or more rows there is always a singular vector */
  /*
   * The loop below has the invariant that
   * the vector at position n has its first n columns zero
   * This is how we start. Choosing a singular vector can only
   * make some rows have a greater leading zero section, if we shuffle
   * we always choose the first not orthogonal vector, and add it
   * in to later vectors, so this cannot change the leading of leading 0
   */
  while (nor > 2) {
    int start_pos = -1;
    u32 prod_start = 0, prod_len = len;
    assert(nor >= 3);
#ifndef NDEBUG
    /*
     * Validate that nor - 3 will be ok for passing to singular_vector
     * ie that no row it receives has a non zero entry earlier than nor - 3
     * I think this may be insufficient if when cleaning to obtain
     * the new perpendicular subspace we clean with a vector with an
     * earlier first non-zero position
     */
    {
      u32 pos;
      word elt = first_non_zero(mat[start], nob, len, &pos);
      NOT_USED(elt);
      assert(0 != elt && nor >= 4);
      assert(pos + 3 >= nor);
      elt = first_non_zero(mat[start + 1], nob, len, &pos);
      assert(0 != elt);
      assert(pos + 3 >= nor);
      elt = first_non_zero(mat[start + 2], nob, len, &pos);
      assert(0 != elt);
      assert(pos + 3 >= nor);
    }
#endif
    res = singular_vector(&row_operations, mat + start, mat + noc, sing_row1, &out_num, qinp,
                          noc, 3, nob, len, prime, &grease, nor - 3, q_indexes, qform, name);
    if (0 != res) {
      fclose(binp);
      fclose(qinp);
      matrix_free(mat);
      free(products);
      fprintf(stderr, "%s: cannot find a singular vector, terminating\n", name);
      return 1;
    }
    assert(nor > 3);
    /* Move the row chosen by singular_vector to the top */
    shuffle(start, start + out_num, mat);
    /* Move on one row, ie ignore the singular vector we just found */
    start++;
    assert(start < noc);
    /*
     * Use skip_mul_from_store here, from offset nor - 3
     * ie first non-zero occurs at a column >= nor-3
     */
#ifndef NDEBUG
    /* Validate that nor - 3 is ok */
    {
      u32 pos;
      word elt = first_non_zero(sing_row1, nob, len, &pos);
      NOT_USED(elt);
      assert(0 != elt && nor >= 4);
      assert(pos + 3 >= nor);
    }
#endif
    /*
     * Compute singular vector v * bilinear form S
     * This is what we take products with to find w such that vSw != 0
     */
    if (0 == skip_mul_from_store(nor - 3, &sing_row1, &sing_row2, binp, 0, noc, len, nob, 1, noc, prime,
                                 &grease, 0, b_indexes, bform, name)) {
      fclose(binp);
      fclose(qinp);
      matrix_free(mat);
      free(products);
      return 1;
    }
    /*
     * Compute start_pos in words
     * We can use this to cut down the size of product
     */
    for (n = 0; n < len; n++) {
      if (0 != sing_row2[n]) {
        start_pos = n;
        break; /* At first non-zero element */
      }
    }
    assert(start_pos >= 0);
    /*
     * Compute the product of the chosen norm zero vector
     * with the remaining vectors. Note we use noc here
     * as nor is our decrementing space dimension
     */
    for (n = start; n < noc; n++) {
      u32 i = (noc - 1 - n) / elts_per_word;
#ifndef NDEBUG
      u32 j;
      for (j = 0; j < i; j++) {
        assert(0 == mat[n][j]);
      }
#endif
      /* Start later in the row if we can */
      if (i < (u32)start_pos) {
        i = start_pos;
      }
      /*
       * Compute vS(mat[n]), so we can find a non-orthogonal vector
       * And also so we know what to add
       * This is where all the time is consumed (just over 50%
       */
      products[n] = (*row_operations.product)(mat[n] + i, sing_row2 + i, len - i);
    }
    n = start;
    res = -1;
    /* TBD: combine this with the previous loop and eliminate products */
    while (n < noc) {
      if (0 != products[n]) {
        word elt = products[n];
        if (res < 0) {
          /*
           * First instance, clean the row and remember it
           * So vS(mat[res]) = 1
           * TBD: can we avoid early parts of the row that are zero?
           */
          if (1 != elt) {
            elt = (*prime_operations.invert)(elt);
            (*row_operations.scaler_in_place)(mat[n], len, elt);
          }
          res = n;
          prod_start = (noc - 1 - n) / elts_per_word;
          prod_len = len - prod_start;
        } else {
          /*
           * Subsequent instance, clean the row with the remembered row
           * TBD: it would make sense here to know where the first
           * non zero is in mat[res]. 24% of total time goes here
           */
          elt = (*prime_operations.negate)(elt);
          if (1 == elt) {
            (*row_operations.incer)(mat[res] + prod_start, mat[n] + prod_start, prod_len);
          } else {
            (*row_operations.scaled_incer)(mat[res] + prod_start, mat[n] + prod_start, prod_len, elt);
          }
        }
      } /* No action if the row is orthogonal */
      n++;
    }
    /* Now remove mat[res] as this isn't a null vector */
    assert(nor > 3);
    nor -= 2;
    shuffle(start, res, mat);
    start++;
    assert(start < noc);
    assert (start + nor == noc);
  }
  assert(nor == 2 && noc == nor + start);
  res = singular_vector(&row_operations, mat + start, mat + noc, sing_row1, &out_num, qinp,
                        noc, nor, nob, len, prime, &grease, 0, q_indexes, qform, name);
  matrix_free(mat);
  free(products);
  fclose(binp);
  fclose(qinp);
  return res;
}
