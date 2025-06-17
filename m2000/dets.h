/*
 * $Id: dets.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Functions to compute determinants
 *
 */

#ifndef included__dets
#define included__dets

#include "primes.h"
#include "rows.h"

extern word det2(prime_ops prime_operations,
                 word e11, word e12,
                 word e21, word e22);

extern word det2_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2);

extern word det3_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2,
                     u32 row_i3, u32 col_i3);

extern word det3(prime_ops prime_operations,
                 word e11, word e12, word e13,
                 word e21, word e22, word e23,
                 word e31, word e32, word e33);

extern word det4_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     row_ops *row_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2,
                     u32 row_i3, u32 col_i3,
                     u32 row_i4, u32 col_i4);

extern word det4(prime_ops prime_operations,
                 row_ops *row_operations,
                 word e11, word e12, word e13, word e14,
                 word e21, word e22, word e23, word e24,
                 word e31, word e32, word e33, word e34,
                 word e41, word e42, word e43, word e44);

extern word det5_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     row_ops *row_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2,
                     u32 row_i3, u32 col_i3,
                     u32 row_i4, u32 col_i4,
                     u32 row_i5, u32 col_i5);

extern word det5(prime_ops prime_operations,
                 row_ops *row_operations,
                 word e11, word e12, word e13, word e14, word e15,
                 word e21, word e22, word e23, word e24, word e25,
                 word e31, word e32, word e33, word e34, word e35,
                 word e41, word e42, word e43, word e44, word e45,
                 word e51, word e52, word e53, word e54, word e55);

extern word det6_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     row_ops *row_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2,
                     u32 row_i3, u32 col_i3,
                     u32 row_i4, u32 col_i4,
                     u32 row_i5, u32 col_i5,
                     u32 row_i6, u32 col_i6);

extern word det6(prime_ops prime_operations,
                 row_ops *row_operations,
                 word e11, word e12, word e13, word e14, word e15, word e16,
                 word e21, word e22, word e23, word e24, word e25, word e26,
                 word e31, word e32, word e33, word e34, word e35, word e36,
                 word e41, word e42, word e43, word e44, word e45, word e46,
                 word e51, word e52, word e53, word e54, word e55, word e56,
                 word e61, word e62, word e63, word e64, word e65, word e66);

extern word det7_ptr(word **rows, u32 nob, prime_ops prime_operations,
                     row_ops *row_operations,
                     u32 row_i1, u32 col_i1,
                     u32 row_i2, u32 col_i2,
                     u32 row_i3, u32 col_i3,
                     u32 row_i4, u32 col_i4,
                     u32 row_i5, u32 col_i5,
                     u32 row_i6, u32 col_i6,
                     u32 row_i7, u32 col_i7);

word det7(prime_ops prime_operations,
          row_ops *row_operations,
          word e11, word e12, word e13, word e14, word e15, word e16, word e17,
          word e21, word e22, word e23, word e24, word e25, word e26, word e27,
          word e31, word e32, word e33, word e34, word e35, word e36, word e37,
          word e41, word e42, word e43, word e44, word e45, word e46, word e47,
          word e51, word e52, word e53, word e54, word e55, word e56, word e57,
          word e61, word e62, word e63, word e64, word e65, word e66, word e67,
          word e71, word e72, word e73, word e74, word e75, word e76, word e77);

#endif
