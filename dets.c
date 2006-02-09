/*
 * $Id: dets.c,v 1.7 2006/02/09 22:07:40 jon Exp $
 *
 * Functions to compute determinants
 *
 */

#include "dets.h"
#include "elements.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static word det(word **rows, prime_ops prime_operations,
                row_ops *row_operations, u32 nor)
{
  u32 i, j, k, prime, nob, elts_per_word;
  word det = 1;
  prime = prime_operations.prime;
  assert(is_a_prime_power(prime));
  assert(NULL != rows);
  nob = bits_of(prime);
  elts_per_word = bits_in_word / nob;
  for (i = 0; i < nor; i++) {
    word elt = first_non_zero(rows[i], nob, nor, &j);
    if (0 == elt) {
      return 0;
    } else {
      if (1 != elt) {
        det = (*prime_operations.mul)(det, elt);
        elt = (*prime_operations.invert)(elt);
      }
      if (1 != elt) {
        (*row_operations->scaler_in_place)(rows[i], nor, elt);
      }
      j /= elts_per_word;
      for (k = i+1; k < nor; k++) {
        elt = rows[k][j];
        if (0 != elt) {
          elt = (*prime_operations.negate)(elt);
          if (1 != elt) {
            (*row_operations->scaled_incer)(rows[i], rows[k], nor, elt);
          } else {
            (*row_operations->incer)(rows[i], rows[k], nor);
          }
        }
      }
    }
  }
  if (2 != prime) {
    /* Need to factor in the sign of the permutation */
    /* For the moment, we don't handle more than 10x10 */
    /* This is not a practial limitation */
    if (nor >= 10) {
      fprintf(stderr, "Cannot handle determinants greater than 10x10\n");
      exit(1);
    } else {
      u32 perm[10];
      int sign = 0;
      for (i = 0; i < nor; i++) {
        word elt = first_non_zero(rows[i], nob, nor, &j);
        NOT_USED(elt);
        perm[i] = j / elts_per_word;
      }
      for (i = 0; i + 1< nor; i++) {
        j = perm[i];
        if (i != j) {
          /* Post multiply by the cycle (i j) */
          assert(j > i);
          sign = 1 - sign;
          for (k = i + 1; k < nor; k++) {
            if (perm[k] == i) {
              perm[k] = j;
              break;
            }
          }
        }
      }
      assert(perm[nor - 1] == nor - 1);
      if (sign) {
        det = (*prime_operations.negate)(det);
      }
    }
  }
  return det;
}

word det2(prime_ops prime_operations,
          word e11, word e12,
          word e21, word e22)
{
  word e1, e2;
  e1 = (*prime_operations.mul)(e11, e22);
  e2 = (*prime_operations.negate)((*prime_operations.mul)(e12, e21));
  return (*prime_operations.add)(e1, e2);
}

word det2_ptr(word **rows, u32 nob, prime_ops prime_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2)
{
  word e11, e21, e12, e22, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  return det2(prime_operations, e11, e12, e21, e22);
}

word det3(prime_ops prime_operations,
          word e11, word e12, word e13,
          word e21, word e22, word e23,
          word e31, word e32, word e33)
{
  word e1, e2, e3, f1, f2, f3;
  f1 = det2(prime_operations, e22, e23, e32, e33);
  f2 = det2(prime_operations, e21, e23, e31, e33);
  f3 = det2(prime_operations, e21, e22, e31, e32);
  e1 = (*prime_operations.mul)(e11, f1);
  e2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  e3 = (*prime_operations.mul)(e13, f3);
  return (*prime_operations.add)((*prime_operations.add)(e1, e2), e3);
}

word det3_ptr(word **rows, u32 nob, prime_ops prime_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2,
              u32 row_i3, u32 col_i3)
{
  word e11, e12, e13, e21, e22, e23, e31, e32, e33, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  e13 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i1]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e23 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i2]);
  e31 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i3]);
  e32 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i3]);
  e33 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i3]);
  return det3(prime_operations, e11, e12, e13, e21, e22, e23, e31, e32, e33);
}

word det4(prime_ops prime_operations,
          row_ops *row_operations,
          word e11, word e12, word e13, word e14,
          word e21, word e22, word e23, word e24,
          word e31, word e32, word e33, word e34,
          word e41, word e42, word e43, word e44)
{
  word r1[4];
  word r2[4];
  word r3[4];
  word r4[4];
  word *rows[4];
  r1[0] = e11;
  r1[1] = e12;
  r1[2] = e13;
  r1[3] = e14;
  r2[0] = e21;
  r2[1] = e22;
  r2[2] = e23;
  r2[3] = e24;
  r3[0] = e31;
  r3[1] = e32;
  r3[2] = e33;
  r3[3] = e34;
  r4[0] = e41;
  r4[1] = e42;
  r4[2] = e43;
  r4[3] = e44;
  rows[0] = r1;
  rows[1] = r2;
  rows[2] = r3;
  rows[3] = r4;
  return det(rows, prime_operations, row_operations, 4);
}

word det4_ptr(word **rows, u32 nob, prime_ops prime_operations,
              row_ops *row_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2,
              u32 row_i3, u32 col_i3,
              u32 row_i4, u32 col_i4)
{
  word e11, e12, e13, e14, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  e13 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i1]);
  e14 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i1]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e23 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i2]);
  e24 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i2]);
  e31 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i3]);
  e32 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i3]);
  e33 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i3]);
  e34 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i3]);
  e41 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i4]);
  e42 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i4]);
  e43 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i4]);
  e44 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i4]);
  return det4(prime_operations, row_operations, e11, e12, e13, e14, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44);
}

word det5(prime_ops prime_operations,
          row_ops *row_operations,
          word e11, word e12, word e13, word e14, word e15,
          word e21, word e22, word e23, word e24, word e25,
          word e31, word e32, word e33, word e34, word e35,
          word e41, word e42, word e43, word e44, word e45,
          word e51, word e52, word e53, word e54, word e55)
{
  word r1[5];
  word r2[5];
  word r3[5];
  word r4[5];
  word r5[5];
  word *rows[5];
  r1[0] = e11;
  r1[1] = e12;
  r1[2] = e13;
  r1[3] = e14;
  r1[4] = e15;
  r2[0] = e21;
  r2[1] = e22;
  r2[2] = e23;
  r2[3] = e24;
  r2[4] = e25;
  r3[0] = e31;
  r3[1] = e32;
  r3[2] = e33;
  r3[3] = e34;
  r3[4] = e35;
  r4[0] = e41;
  r4[1] = e42;
  r4[2] = e43;
  r4[3] = e44;
  r4[4] = e45;
  r5[0] = e51;
  r5[1] = e52;
  r5[2] = e53;
  r5[3] = e54;
  r5[4] = e55;
  rows[0] = r1;
  rows[1] = r2;
  rows[2] = r3;
  rows[3] = r4;
  rows[4] = r5;
  return det(rows, prime_operations, row_operations, 5);
}

word det5_ptr(word **rows, u32 nob, prime_ops prime_operations,
              row_ops *row_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2,
              u32 row_i3, u32 col_i3,
              u32 row_i4, u32 col_i4,
              u32 row_i5, u32 col_i5)
{
  word e11, e12, e13, e14, e15, e21, e22, e23, e24, e25, e31, e32, e33, e34, e35, e41, e42, e43, e44, e45, e51, e52, e53, e54, e55, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  e13 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i1]);
  e14 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i1]);
  e15 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i1]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e23 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i2]);
  e24 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i2]);
  e25 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i2]);
  e31 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i3]);
  e32 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i3]);
  e33 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i3]);
  e34 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i3]);
  e35 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i3]);
  e41 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i4]);
  e42 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i4]);
  e43 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i4]);
  e44 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i4]);
  e45 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i4]);
  e51 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i5]);
  e52 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i5]);
  e53 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i5]);
  e54 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i5]);
  e55 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i5]);
  return det5(prime_operations, row_operations,
              e11, e12, e13, e14, e15,
              e21, e22, e23, e24, e25,
              e31, e32, e33, e34, e35,
              e41, e42, e43, e44, e45,
              e51, e52, e53, e54, e55);
}

word det6(prime_ops prime_operations,
          row_ops *row_operations,
          word e11, word e12, word e13, word e14, word e15, word e16,
          word e21, word e22, word e23, word e24, word e25, word e26,
          word e31, word e32, word e33, word e34, word e35, word e36,
          word e41, word e42, word e43, word e44, word e45, word e46,
          word e51, word e52, word e53, word e54, word e55, word e56,
          word e61, word e62, word e63, word e64, word e65, word e66
)
{
  word r1[6];
  word r2[6];
  word r3[6];
  word r4[6];
  word r5[6];
  word r6[6];
  word *rows[6];
  r1[0] = e11;
  r1[1] = e12;
  r1[2] = e13;
  r1[3] = e14;
  r1[4] = e15;
  r1[5] = e16;
  r2[0] = e21;
  r2[1] = e22;
  r2[2] = e23;
  r2[3] = e24;
  r2[4] = e25;
  r2[5] = e26;
  r3[0] = e31;
  r3[1] = e32;
  r3[2] = e33;
  r3[3] = e34;
  r3[4] = e35;
  r3[5] = e36;
  r4[0] = e41;
  r4[1] = e42;
  r4[2] = e43;
  r4[3] = e44;
  r4[4] = e45;
  r4[5] = e46;
  r5[0] = e51;
  r5[1] = e52;
  r5[2] = e53;
  r5[3] = e54;
  r5[4] = e55;
  r5[5] = e56;
  r6[0] = e61;
  r6[1] = e62;
  r6[2] = e63;
  r6[3] = e64;
  r6[4] = e65;
  r6[5] = e66;
  rows[0] = r1;
  rows[1] = r2;
  rows[2] = r3;
  rows[3] = r4;
  rows[4] = r5;
  rows[5] = r6;
  return det(rows, prime_operations, row_operations, 6);
}

word det6_ptr(word **rows, u32 nob, prime_ops prime_operations,
              row_ops *row_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2,
              u32 row_i3, u32 col_i3,
              u32 row_i4, u32 col_i4,
              u32 row_i5, u32 col_i5,
              u32 row_i6, u32 col_i6)
{
  word e11, e12, e13, e14, e15, e16, e21, e22, e23, e24, e25, e26, e31, e32, e33, e34, e35, e36, e41, e42, e43, e44, e45, e46, e51, e52, e53, e54, e55, e56, e61, e62, e63, e64, e65, e66, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  e13 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i1]);
  e14 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i1]);
  e15 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i1]);
  e16 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i1]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e23 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i2]);
  e24 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i2]);
  e25 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i2]);
  e26 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i2]);
  e31 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i3]);
  e32 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i3]);
  e33 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i3]);
  e34 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i3]);
  e35 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i3]);
  e36 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i3]);
  e41 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i4]);
  e42 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i4]);
  e43 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i4]);
  e44 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i4]);
  e45 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i4]);
  e46 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i4]);
  e51 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i5]);
  e52 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i5]);
  e53 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i5]);
  e54 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i5]);
  e55 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i5]);
  e56 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i5]);
  e61 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i6]);
  e62 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i6]);
  e63 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i6]);
  e64 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i6]);
  e65 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i6]);
  e66 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i6]);
  return det6(prime_operations, row_operations,
              e11, e12, e13, e14, e15, e16,
              e21, e22, e23, e24, e25, e26,
              e31, e32, e33, e34, e35, e36,
              e41, e42, e43, e44, e45, e46,
              e51, e52, e53, e54, e55, e56,
              e61, e62, e63, e64, e65, e66);
}

word det7(prime_ops prime_operations,
          row_ops *row_operations,
          word e11, word e12, word e13, word e14, word e15, word e16, word e17,
          word e21, word e22, word e23, word e24, word e25, word e26, word e27,
          word e31, word e32, word e33, word e34, word e35, word e36, word e37,
          word e41, word e42, word e43, word e44, word e45, word e46, word e47,
          word e51, word e52, word e53, word e54, word e55, word e56, word e57,
          word e61, word e62, word e63, word e64, word e65, word e66, word e67,
          word e71, word e72, word e73, word e74, word e75, word e76, word e77)
{
  word r1[7];
  word r2[7];
  word r3[7];
  word r4[7];
  word r5[7];
  word r6[7];
  word r7[7];
  word *rows[7];
  r1[0] = e11;
  r1[1] = e12;
  r1[2] = e13;
  r1[3] = e14;
  r1[4] = e15;
  r1[5] = e16;
  r1[6] = e17;
  r2[0] = e21;
  r2[1] = e22;
  r2[2] = e23;
  r2[3] = e24;
  r2[4] = e25;
  r2[5] = e26;
  r2[6] = e27;
  r3[0] = e31;
  r3[1] = e32;
  r3[2] = e33;
  r3[3] = e34;
  r3[4] = e35;
  r3[5] = e36;
  r3[6] = e37;
  r4[0] = e41;
  r4[1] = e42;
  r4[2] = e43;
  r4[3] = e44;
  r4[4] = e45;
  r4[5] = e46;
  r4[6] = e47;
  r5[0] = e51;
  r5[1] = e52;
  r5[2] = e53;
  r5[3] = e54;
  r5[4] = e55;
  r5[5] = e56;
  r5[6] = e57;
  r6[0] = e61;
  r6[1] = e62;
  r6[2] = e63;
  r6[3] = e64;
  r6[4] = e65;
  r6[5] = e66;
  r6[6] = e67;
  r7[0] = e71;
  r7[1] = e72;
  r7[2] = e73;
  r7[3] = e74;
  r7[4] = e75;
  r7[5] = e76;
  r7[6] = e77;
  rows[0] = r1;
  rows[1] = r2;
  rows[2] = r3;
  rows[3] = r4;
  rows[4] = r5;
  rows[5] = r6;
  rows[6] = r7;
  return det(rows, prime_operations, row_operations, 7);
}

word det7_ptr(word **rows, u32 nob, prime_ops prime_operations,
              row_ops *row_operations,
              u32 row_i1, u32 col_i1,
              u32 row_i2, u32 col_i2,
              u32 row_i3, u32 col_i3,
              u32 row_i4, u32 col_i4,
              u32 row_i5, u32 col_i5,
              u32 row_i6, u32 col_i6,
              u32 row_i7, u32 col_i7)
{
  word e11, e12, e13, e14, e15, e16, e17, e21, e22, e23, e24, e25, e26, e27, e31, e32, e33, e34, e35, e36, e37, e41, e42, e43, e44, e45, e46, e47, e51, e52, e53, e54, e55, e56, e57, e61, e62, e63, e64, e65, e66, e67, e71, e72, e73, e74, e75, e76, e77, mask;
  u32 elts_per_word;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  e11 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i1]);
  e12 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i1]);
  e13 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i1]);
  e14 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i1]);
  e15 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i1]);
  e16 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i1]);
  e17 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i1]);
  e21 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i2]);
  e22 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i2]);
  e23 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i2]);
  e24 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i2]);
  e25 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i2]);
  e26 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i2]);
  e27 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i2]);
  e31 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i3]);
  e32 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i3]);
  e33 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i3]);
  e34 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i3]);
  e35 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i3]);
  e36 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i3]);
  e37 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i3]);
  e41 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i4]);
  e42 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i4]);
  e43 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i4]);
  e44 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i4]);
  e45 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i4]);
  e46 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i4]);
  e47 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i4]);
  e51 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i5]);
  e52 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i5]);
  e53 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i5]);
  e54 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i5]);
  e55 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i5]);
  e56 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i5]);
  e57 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i5]);
  e61 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i6]);
  e62 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i6]);
  e63 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i6]);
  e64 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i6]);
  e65 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i6]);
  e66 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i6]);
  e67 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i6]);
  e71 = get_element_from_row_with_params(nob, col_i1, mask, elts_per_word, rows[row_i7]);
  e72 = get_element_from_row_with_params(nob, col_i2, mask, elts_per_word, rows[row_i7]);
  e73 = get_element_from_row_with_params(nob, col_i3, mask, elts_per_word, rows[row_i7]);
  e74 = get_element_from_row_with_params(nob, col_i4, mask, elts_per_word, rows[row_i7]);
  e75 = get_element_from_row_with_params(nob, col_i5, mask, elts_per_word, rows[row_i7]);
  e76 = get_element_from_row_with_params(nob, col_i6, mask, elts_per_word, rows[row_i7]);
  e77 = get_element_from_row_with_params(nob, col_i7, mask, elts_per_word, rows[row_i7]);
  return det7(prime_operations, row_operations,
              e11, e12, e13, e14, e15, e16, e17,
              e21, e22, e23, e24, e25, e26, e27,
              e31, e32, e33, e34, e35, e36, e37,
              e41, e42, e43, e44, e45, e46, e47,
              e51, e52, e53, e54, e55, e56, e57,
              e61, e62, e63, e64, e65, e66, e67,
              e71, e72, e73, e74, e75, e76, e77);
}
