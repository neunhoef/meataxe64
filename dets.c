/*
 * $Id: dets.c,v 1.1 2002/02/27 19:06:17 jon Exp $
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

#define ROW_DETS 1

#if ROW_DETS
static unsigned int det(unsigned int **rows, prime_ops prime_operations,
                        unsigned int nor)
{
  row_ops row_operations;
  unsigned int i, j, k, det = 1, prime, nob;
  prime = prime_operations.prime;
  assert(is_a_prime_power(prime));
  assert(NULL != rows);
  nob = bits_of(prime);
  rows_init(prime, &row_operations);
  for (i = 0; i < nor; i++) {
    unsigned int elt = first_non_zero(rows[i], nob, nor, &j);
    if (0 == elt) {
      return 0;
    } else {
      det = (*prime_operations.mul)(det, elt);
      elt = (*prime_operations.negate)(elt);
      if (1 != elt) {
        (*row_operations.scaler_in_place)(rows[i], nor, elt);
      }
      for (k = i+1; k < nor; k++) {
        elt = get_element_from_row(nob, j, rows[k]);
        if (0 != elt) {
          elt = (*prime_operations.negate)(elt);
          if (1 != elt) {
            (*row_operations.scaled_incer)(rows[i], rows[k], nor, elt);
          } else {
            (*row_operations.incer)(rows[i], rows[k], nor);
          }
        }
      }
    }
  }
  return det;
}
#endif

unsigned int det2(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12,
                  unsigned int e21, unsigned int e22)
{
  unsigned int e1, e2;
  e1 = (*prime_operations.mul)(e11, e22);
  e2 = (*prime_operations.negate)((*prime_operations.mul)(e12, e21));
  return (*prime_operations.add)(e1, e2);
}

unsigned int det2_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                  unsigned int row_i1, unsigned int col_i1,
                  unsigned int row_i2, unsigned int col_i2)
{
  unsigned int e11, e21, e12, e22;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  return det2(prime_operations, e11, e12, e21, e22);
}

unsigned int det3(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13,
                  unsigned int e21, unsigned int e22, unsigned int e23,
                  unsigned int e31, unsigned int e32, unsigned int e33)
{
  unsigned int e1, e2, e3, f1, f2, f3;
  f1 = det2(prime_operations, e22, e23, e32, e33);
  f2 = det2(prime_operations, e21, e23, e31, e33);
  f3 = det2(prime_operations, e21, e22, e31, e32);
  e1 = (*prime_operations.mul)(e11, f1);
  e2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  e3 = (*prime_operations.mul)(e13, f3);
  return (*prime_operations.add)((*prime_operations.add)(e1, e2), e3);
}

unsigned int det3_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                      unsigned int row_i1, unsigned int col_i1,
                      unsigned int row_i2, unsigned int col_i2,
                      unsigned int row_i3, unsigned int col_i3)
{
  unsigned int e11, e12, e13, e21, e22, e23, e31, e32, e33;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  e13 = get_element_from_row(nob, col_i3, rows[row_i1]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e23 = get_element_from_row(nob, col_i3, rows[row_i2]);
  e31 = get_element_from_row(nob, col_i1, rows[row_i3]);
  e32 = get_element_from_row(nob, col_i2, rows[row_i3]);
  e33 = get_element_from_row(nob, col_i3, rows[row_i3]);
  return det3(prime_operations, e11, e12, e13, e21, e22, e23, e31, e32, e33);
}

unsigned int det4(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14,
                  unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24,
                  unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34,
                  unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44)
{
#if ROW_DETS
  unsigned int r1[4];
  unsigned int r2[4];
  unsigned int r3[4];
  unsigned int r4[4];
  unsigned int *rows[4];
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
  return det(rows, prime_operations, 4);
#else
  unsigned int e1, e2, e3, e4, f1, f2, f3, f4;
  f1 = det3(prime_operations, e22, e23, e24, e32, e33, e34, e42, e43, e44);
  f2 = det3(prime_operations, e21, e23, e24, e31, e33, e34, e41, e43, e44);
  f3 = det3(prime_operations, e21, e22, e24, e31, e32, e34, e41, e42, e44);
  f4 = det3(prime_operations, e21, e22, e23, e31, e32, e33, e41, e42, e43);
  e1 = (*prime_operations.mul)(e11, f1);
  e2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  e3 = (*prime_operations.mul)(e13, f3);
  e4 = (*prime_operations.negate)((*prime_operations.mul)(e14, f4));
  return (*prime_operations.add)((*prime_operations.add)((*prime_operations.add)(e1, e2), e3), e4);
#endif
}

unsigned int det4_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                      unsigned int row_i1, unsigned int col_i1,
                      unsigned int row_i2, unsigned int col_i2,
                      unsigned int row_i3, unsigned int col_i3,
                      unsigned int row_i4, unsigned int col_i4)
{
  unsigned int e11, e12, e13, e14, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  e13 = get_element_from_row(nob, col_i3, rows[row_i1]);
  e14 = get_element_from_row(nob, col_i4, rows[row_i1]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e23 = get_element_from_row(nob, col_i3, rows[row_i2]);
  e24 = get_element_from_row(nob, col_i4, rows[row_i2]);
  e31 = get_element_from_row(nob, col_i1, rows[row_i3]);
  e32 = get_element_from_row(nob, col_i2, rows[row_i3]);
  e33 = get_element_from_row(nob, col_i3, rows[row_i3]);
  e34 = get_element_from_row(nob, col_i4, rows[row_i3]);
  e41 = get_element_from_row(nob, col_i1, rows[row_i4]);
  e42 = get_element_from_row(nob, col_i2, rows[row_i4]);
  e43 = get_element_from_row(nob, col_i3, rows[row_i4]);
  e44 = get_element_from_row(nob, col_i4, rows[row_i4]);
  return det4(prime_operations, e11, e12, e13, e14, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44);
}

unsigned int det5(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15,
                  unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25,
                  unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35,
                  unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45,
                  unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55)
{
#if ROW_DETS
  unsigned int r1[5];
  unsigned int r2[5];
  unsigned int r3[5];
  unsigned int r4[5];
  unsigned int r5[5];
  unsigned int *rows[5];
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
  return det(rows, prime_operations, 5);
#else
  unsigned int f1, f2, f3, f4, f5;
  f1 = det4(prime_operations, e22, e23, e24, e25, e32, e33, e34, e35, e42, e43, e44, e45, e52, e53, e54, e55);
  f2 = det4(prime_operations, e21, e23, e24, e25, e31, e33, e34, e35, e41, e43, e44, e45, e51, e53, e54, e55);
  f3 = det4(prime_operations, e21, e22, e24, e25, e31, e32, e34, e35, e41, e42, e44, e45, e51, e52, e54, e55);
  f4 = det4(prime_operations, e21, e22, e23, e25, e31, e32, e33, e35, e41, e42, e43, e45, e51, e52, e53, e55);
  f5 = det4(prime_operations, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44, e51, e52, e53, e54);
  f1 = (*prime_operations.mul)(e11, f1);
  f2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  f3 = (*prime_operations.mul)(e13, f3);
  f4 = (*prime_operations.negate)((*prime_operations.mul)(e14, f4));
  f5 = (*prime_operations.mul)(e15, f5);
  return (*prime_operations.add)((*prime_operations.add)((*prime_operations.add)((*prime_operations.add)(f1, f2), f3), f4), f5);
#endif
}

unsigned int det5_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                      unsigned int row_i1, unsigned int col_i1,
                      unsigned int row_i2, unsigned int col_i2,
                      unsigned int row_i3, unsigned int col_i3,
                      unsigned int row_i4, unsigned int col_i4,
                      unsigned int row_i5, unsigned int col_i5)
{
  unsigned int e11, e12, e13, e14, e15, e21, e22, e23, e24, e25, e31, e32, e33, e34, e35, e41, e42, e43, e44, e45, e51, e52, e53, e54, e55;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  e13 = get_element_from_row(nob, col_i3, rows[row_i1]);
  e14 = get_element_from_row(nob, col_i4, rows[row_i1]);
  e15 = get_element_from_row(nob, col_i5, rows[row_i1]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e23 = get_element_from_row(nob, col_i3, rows[row_i2]);
  e24 = get_element_from_row(nob, col_i4, rows[row_i2]);
  e25 = get_element_from_row(nob, col_i5, rows[row_i2]);
  e31 = get_element_from_row(nob, col_i1, rows[row_i3]);
  e32 = get_element_from_row(nob, col_i2, rows[row_i3]);
  e33 = get_element_from_row(nob, col_i3, rows[row_i3]);
  e34 = get_element_from_row(nob, col_i4, rows[row_i3]);
  e35 = get_element_from_row(nob, col_i5, rows[row_i3]);
  e41 = get_element_from_row(nob, col_i1, rows[row_i4]);
  e42 = get_element_from_row(nob, col_i2, rows[row_i4]);
  e43 = get_element_from_row(nob, col_i3, rows[row_i4]);
  e44 = get_element_from_row(nob, col_i4, rows[row_i4]);
  e45 = get_element_from_row(nob, col_i5, rows[row_i4]);
  e51 = get_element_from_row(nob, col_i1, rows[row_i5]);
  e52 = get_element_from_row(nob, col_i2, rows[row_i5]);
  e53 = get_element_from_row(nob, col_i3, rows[row_i5]);
  e54 = get_element_from_row(nob, col_i4, rows[row_i5]);
  e55 = get_element_from_row(nob, col_i5, rows[row_i5]);
  return det5(prime_operations, e11, e12, e13, e14, e15,
              e21, e22, e23, e24, e25,
              e31, e32, e33, e34, e35,
              e41, e42, e43, e44, e45,
              e51, e52, e53, e54, e55);
}

unsigned int det6(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15, unsigned int e16,
                  unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25, unsigned int e26,
                  unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35, unsigned int e36,
                  unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45, unsigned int e46,
                  unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55, unsigned int e56,
                  unsigned int e61, unsigned int e62, unsigned int e63, unsigned int e64, unsigned int e65, unsigned int e66
)
{
#if ROW_DETS
  unsigned int r1[5];
  unsigned int r2[5];
  unsigned int r3[5];
  unsigned int r4[5];
  unsigned int r5[5];
  unsigned int r6[6];
  unsigned int *rows[6];
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
  return det(rows, prime_operations, 6);
#else
  unsigned int f1, f2, f3, f4, f5, f6;
  f1 = det5(prime_operations, e22, e23, e24, e25, e26, e32, e33, e34, e35, e36, e42, e43, e44, e45, e46, e52, e53, e54, e55, e56, e62, e63, e64, e65, e66);
  f2 = det5(prime_operations, e21, e23, e24, e25, e26, e31, e33, e34, e35, e36, e41, e43, e44, e45, e46, e51, e53, e54, e55, e56, e61, e63, e64, e65, e66);
  f3 = det5(prime_operations, e21, e22, e24, e25, e26, e31, e32, e34, e35, e36, e41, e42, e44, e45, e46, e51, e52, e54, e55, e56, e61, e62, e64, e65, e66);
  f4 = det5(prime_operations, e21, e22, e23, e25, e26, e31, e32, e33, e35, e36, e41, e42, e43, e45, e46, e51, e52, e53, e55, e56, e61, e62, e63, e65, e66);
  f5 = det5(prime_operations, e21, e22, e23, e24, e26, e31, e32, e33, e34, e36, e41, e42, e43, e44, e46, e51, e52, e53, e54, e56, e61, e62, e63, e64, e66);
  f6 = det5(prime_operations, e21, e22, e23, e24, e25, e31, e32, e33, e34, e35, e41, e42, e43, e44, e45, e51, e52, e53, e54, e55, e61, e62, e63, e64, e65);
  f1 = (*prime_operations.mul)(e11, f1);
  f2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  f3 = (*prime_operations.mul)(e13, f3);
  f4 = (*prime_operations.negate)((*prime_operations.mul)(e14, f4));
  f5 = (*prime_operations.mul)(e15, f5);
  f6 = (*prime_operations.negate)((*prime_operations.mul)(e16, f6));
  return (*prime_operations.add)((*prime_operations.add)((*prime_operations.add)((*prime_operations.add)((*prime_operations.add)(f1, f2), f3), f4), f5), f6);
#endif
}

unsigned int det6_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                      unsigned int row_i1, unsigned int col_i1,
                      unsigned int row_i2, unsigned int col_i2,
                      unsigned int row_i3, unsigned int col_i3,
                      unsigned int row_i4, unsigned int col_i4,
                      unsigned int row_i5, unsigned int col_i5,
                      unsigned int row_i6, unsigned int col_i6)
{
  unsigned int e11, e12, e13, e14, e15, e16, e21, e22, e23, e24, e25, e26, e31, e32, e33, e34, e35, e36, e41, e42, e43, e44, e45, e46, e51, e52, e53, e54, e55, e56, e61, e62, e63, e64, e65, e66;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  e13 = get_element_from_row(nob, col_i3, rows[row_i1]);
  e14 = get_element_from_row(nob, col_i4, rows[row_i1]);
  e15 = get_element_from_row(nob, col_i5, rows[row_i1]);
  e16 = get_element_from_row(nob, col_i6, rows[row_i1]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e23 = get_element_from_row(nob, col_i3, rows[row_i2]);
  e24 = get_element_from_row(nob, col_i4, rows[row_i2]);
  e25 = get_element_from_row(nob, col_i5, rows[row_i2]);
  e26 = get_element_from_row(nob, col_i6, rows[row_i2]);
  e31 = get_element_from_row(nob, col_i1, rows[row_i3]);
  e32 = get_element_from_row(nob, col_i2, rows[row_i3]);
  e33 = get_element_from_row(nob, col_i3, rows[row_i3]);
  e34 = get_element_from_row(nob, col_i4, rows[row_i3]);
  e35 = get_element_from_row(nob, col_i5, rows[row_i3]);
  e36 = get_element_from_row(nob, col_i6, rows[row_i3]);
  e41 = get_element_from_row(nob, col_i1, rows[row_i4]);
  e42 = get_element_from_row(nob, col_i2, rows[row_i4]);
  e43 = get_element_from_row(nob, col_i3, rows[row_i4]);
  e44 = get_element_from_row(nob, col_i4, rows[row_i4]);
  e45 = get_element_from_row(nob, col_i5, rows[row_i4]);
  e46 = get_element_from_row(nob, col_i6, rows[row_i4]);
  e51 = get_element_from_row(nob, col_i1, rows[row_i5]);
  e52 = get_element_from_row(nob, col_i2, rows[row_i5]);
  e53 = get_element_from_row(nob, col_i3, rows[row_i5]);
  e54 = get_element_from_row(nob, col_i4, rows[row_i5]);
  e55 = get_element_from_row(nob, col_i5, rows[row_i5]);
  e56 = get_element_from_row(nob, col_i6, rows[row_i5]);
  e61 = get_element_from_row(nob, col_i1, rows[row_i6]);
  e62 = get_element_from_row(nob, col_i2, rows[row_i6]);
  e63 = get_element_from_row(nob, col_i3, rows[row_i6]);
  e64 = get_element_from_row(nob, col_i4, rows[row_i6]);
  e65 = get_element_from_row(nob, col_i5, rows[row_i6]);
  e66 = get_element_from_row(nob, col_i6, rows[row_i6]);
  return det6(prime_operations, e11, e12, e13, e14, e15, e16,
              e21, e22, e23, e24, e25, e26,
              e31, e32, e33, e34, e35, e36,
              e41, e42, e43, e44, e45, e46,
              e51, e52, e53, e54, e55, e56,
              e61, e62, e63, e64, e65, e66);
}

unsigned int det7(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15, unsigned int e16, unsigned int e17,
                  unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25, unsigned int e26, unsigned int e27,
                  unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35, unsigned int e36, unsigned int e37,
                  unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45, unsigned int e46, unsigned int e47,
                  unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55, unsigned int e56, unsigned int e57,
                  unsigned int e61, unsigned int e62, unsigned int e63, unsigned int e64, unsigned int e65, unsigned int e66, unsigned int e67,
                  unsigned int e71, unsigned int e72, unsigned int e73, unsigned int e74, unsigned int e75, unsigned int e76, unsigned int e77)
{
#if ROW_DETS
  unsigned int r1[5];
  unsigned int r2[5];
  unsigned int r3[5];
  unsigned int r4[5];
  unsigned int r5[5];
  unsigned int r6[6];
  unsigned int r7[7];
  unsigned int *rows[7];
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
  return det(rows, prime_operations, 7);
#else
  unsigned int f1, f2, f3, f4, f5, f6;
  f1 = det5(prime_operations, e22, e23, e24, e25, e26, e32, e33, e34, e35, e36, e42, e43, e44, e45, e46, e52, e53, e54, e55, e56, e62, e63, e64, e65, e66);
  f2 = det5(prime_operations, e21, e23, e24, e25, e26, e31, e33, e34, e35, e36, e41, e43, e44, e45, e46, e51, e53, e54, e55, e56, e61, e63, e64, e65, e66);
  f3 = det5(prime_operations, e21, e22, e24, e25, e26, e31, e32, e34, e35, e36, e41, e42, e44, e45, e46, e51, e52, e54, e55, e56, e61, e62, e64, e65, e66);
  f4 = det5(prime_operations, e21, e22, e23, e25, e26, e31, e32, e33, e35, e36, e41, e42, e43, e45, e46, e51, e52, e53, e55, e56, e61, e62, e63, e65, e66);
  f5 = det5(prime_operations, e21, e22, e23, e24, e26, e31, e32, e33, e34, e36, e41, e42, e43, e44, e46, e51, e52, e53, e54, e56, e61, e62, e63, e64, e66);
  f6 = det5(prime_operations, e21, e22, e23, e24, e25, e31, e32, e33, e34, e35, e41, e42, e43, e44, e45, e51, e52, e53, e54, e55, e61, e62, e63, e64, e65);
  f1 = (*prime_operations.mul)(e11, f1);
  f2 = (*prime_operations.negate)((*prime_operations.mul)(e12, f2));
  f3 = (*prime_operations.mul)(e13, f3);
  f4 = (*prime_operations.negate)((*prime_operations.mul)(e14, f4));
  f5 = (*prime_operations.mul)(e15, f5);
  f6 = (*prime_operations.negate)((*prime_operations.mul)(e16, f6));
  return (*prime_operations.add)((*prime_operations.add)((*prime_operations.add)((*prime_operations.add)((*prime_operations.add)(f1, f2), f3), f4), f5), f6);
#endif
}

unsigned int det7_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                      unsigned int row_i1, unsigned int col_i1,
                      unsigned int row_i2, unsigned int col_i2,
                      unsigned int row_i3, unsigned int col_i3,
                      unsigned int row_i4, unsigned int col_i4,
                      unsigned int row_i5, unsigned int col_i5,
                      unsigned int row_i6, unsigned int col_i6,
                      unsigned int row_i7, unsigned int col_i7)
{
  unsigned int e11, e12, e13, e14, e15, e16, e17, e21, e22, e23, e24, e25, e26, e27, e31, e32, e33, e34, e35, e36, e37, e41, e42, e43, e44, e45, e46, e47, e51, e52, e53, e54, e55, e56, e57, e61, e62, e63, e64, e65, e66, e67, e71, e72, e73, e74, e75, e76, e77;
  assert(NULL != rows);
  e11 = get_element_from_row(nob, col_i1, rows[row_i1]);
  e12 = get_element_from_row(nob, col_i2, rows[row_i1]);
  e13 = get_element_from_row(nob, col_i3, rows[row_i1]);
  e14 = get_element_from_row(nob, col_i4, rows[row_i1]);
  e15 = get_element_from_row(nob, col_i5, rows[row_i1]);
  e16 = get_element_from_row(nob, col_i6, rows[row_i1]);
  e17 = get_element_from_row(nob, col_i7, rows[row_i1]);
  e21 = get_element_from_row(nob, col_i1, rows[row_i2]);
  e22 = get_element_from_row(nob, col_i2, rows[row_i2]);
  e23 = get_element_from_row(nob, col_i3, rows[row_i2]);
  e24 = get_element_from_row(nob, col_i4, rows[row_i2]);
  e25 = get_element_from_row(nob, col_i5, rows[row_i2]);
  e26 = get_element_from_row(nob, col_i6, rows[row_i2]);
  e27 = get_element_from_row(nob, col_i7, rows[row_i2]);
  e31 = get_element_from_row(nob, col_i1, rows[row_i3]);
  e32 = get_element_from_row(nob, col_i2, rows[row_i3]);
  e33 = get_element_from_row(nob, col_i3, rows[row_i3]);
  e34 = get_element_from_row(nob, col_i4, rows[row_i3]);
  e35 = get_element_from_row(nob, col_i5, rows[row_i3]);
  e36 = get_element_from_row(nob, col_i6, rows[row_i3]);
  e37 = get_element_from_row(nob, col_i7, rows[row_i3]);
  e41 = get_element_from_row(nob, col_i1, rows[row_i4]);
  e42 = get_element_from_row(nob, col_i2, rows[row_i4]);
  e43 = get_element_from_row(nob, col_i3, rows[row_i4]);
  e44 = get_element_from_row(nob, col_i4, rows[row_i4]);
  e45 = get_element_from_row(nob, col_i5, rows[row_i4]);
  e46 = get_element_from_row(nob, col_i6, rows[row_i4]);
  e47 = get_element_from_row(nob, col_i7, rows[row_i4]);
  e51 = get_element_from_row(nob, col_i1, rows[row_i5]);
  e52 = get_element_from_row(nob, col_i2, rows[row_i5]);
  e53 = get_element_from_row(nob, col_i3, rows[row_i5]);
  e54 = get_element_from_row(nob, col_i4, rows[row_i5]);
  e55 = get_element_from_row(nob, col_i5, rows[row_i5]);
  e56 = get_element_from_row(nob, col_i6, rows[row_i5]);
  e57 = get_element_from_row(nob, col_i7, rows[row_i5]);
  e61 = get_element_from_row(nob, col_i1, rows[row_i6]);
  e62 = get_element_from_row(nob, col_i2, rows[row_i6]);
  e63 = get_element_from_row(nob, col_i3, rows[row_i6]);
  e64 = get_element_from_row(nob, col_i4, rows[row_i6]);
  e65 = get_element_from_row(nob, col_i5, rows[row_i6]);
  e66 = get_element_from_row(nob, col_i6, rows[row_i6]);
  e67 = get_element_from_row(nob, col_i7, rows[row_i6]);
  e71 = get_element_from_row(nob, col_i1, rows[row_i7]);
  e72 = get_element_from_row(nob, col_i2, rows[row_i7]);
  e73 = get_element_from_row(nob, col_i3, rows[row_i7]);
  e74 = get_element_from_row(nob, col_i4, rows[row_i7]);
  e75 = get_element_from_row(nob, col_i5, rows[row_i7]);
  e76 = get_element_from_row(nob, col_i6, rows[row_i7]);
  e77 = get_element_from_row(nob, col_i7, rows[row_i7]);
  return det7(prime_operations, e11, e12, e13, e14, e15, e16, e17,
              e21, e22, e23, e24, e25, e26, e27,
              e31, e32, e33, e34, e35, e36, e37,
              e41, e42, e43, e44, e45, e46, e47,
              e51, e52, e53, e54, e55, e56, e57,
              e61, e62, e63, e64, e65, e66, e67,
              e71, e72, e73, e74, e75, e76, e77);
}
