/*
 * $Id: dets.h,v 1.1 2002/02/27 19:06:17 jon Exp $
 *
 * Functions to compute determinants
 *
 */

#ifndef included__dets
#define included__dets

#include "primes.h"

extern unsigned int det2(prime_ops prime_operations,
                         unsigned int e11, unsigned int e12,
                         unsigned int e21, unsigned int e22);

extern unsigned int det2_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2);

extern unsigned int det3_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2,
                             unsigned int row_i3, unsigned int col_i3);

extern unsigned int det3(prime_ops prime_operations,
                         unsigned int e11, unsigned int e12, unsigned int e13,
                         unsigned int e21, unsigned int e22, unsigned int e23,
                         unsigned int e31, unsigned int e32, unsigned int e33);

extern unsigned int det4_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2,
                             unsigned int row_i3, unsigned int col_i3,
                             unsigned int row_i4, unsigned int col_i4);

extern unsigned int det4(prime_ops prime_operations,
                         unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14,
                         unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24,
                         unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34,
                         unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44);

extern unsigned int det5_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2,
                             unsigned int row_i3, unsigned int col_i3,
                             unsigned int row_i4, unsigned int col_i4,
                             unsigned int row_i5, unsigned int col_i5);

extern unsigned int det5(prime_ops prime_operations,
                         unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15,
                         unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25,
                         unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35,
                         unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45,
                         unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55);

extern unsigned int det6_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2,
                             unsigned int row_i3, unsigned int col_i3,
                             unsigned int row_i4, unsigned int col_i4,
                             unsigned int row_i5, unsigned int col_i5,
                             unsigned int row_i6, unsigned int col_i6);

extern unsigned int det6(prime_ops prime_operations,
                         unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15, unsigned int e16,
                         unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25, unsigned int e26,
                         unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35, unsigned int e36,
                         unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45, unsigned int e46,
                         unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55, unsigned int e56,
                         unsigned int e61, unsigned int e62, unsigned int e63, unsigned int e64, unsigned int e65, unsigned int e66);

extern unsigned int det7_ptr(unsigned int **rows, unsigned int nob, prime_ops prime_operations,
                             unsigned int row_i1, unsigned int col_i1,
                             unsigned int row_i2, unsigned int col_i2,
                             unsigned int row_i3, unsigned int col_i3,
                             unsigned int row_i4, unsigned int col_i4,
                             unsigned int row_i5, unsigned int col_i5,
                             unsigned int row_i6, unsigned int col_i6,
                             unsigned int row_i7, unsigned int col_i7);

unsigned int det7(prime_ops prime_operations,
                  unsigned int e11, unsigned int e12, unsigned int e13, unsigned int e14, unsigned int e15, unsigned int e16, unsigned int e17,
                  unsigned int e21, unsigned int e22, unsigned int e23, unsigned int e24, unsigned int e25, unsigned int e26, unsigned int e27,
                  unsigned int e31, unsigned int e32, unsigned int e33, unsigned int e34, unsigned int e35, unsigned int e36, unsigned int e37,
                  unsigned int e41, unsigned int e42, unsigned int e43, unsigned int e44, unsigned int e45, unsigned int e46, unsigned int e47,
                  unsigned int e51, unsigned int e52, unsigned int e53, unsigned int e54, unsigned int e55, unsigned int e56, unsigned int e57,
                  unsigned int e61, unsigned int e62, unsigned int e63, unsigned int e64, unsigned int e65, unsigned int e66, unsigned int e67,
                  unsigned int e71, unsigned int e72, unsigned int e73, unsigned int e74, unsigned int e75, unsigned int e76, unsigned int e77);

#endif
