/*
 * $Id: maketab.c,v 1.3 2005/07/24 09:32:45 jon Exp $
 *
 * Produce the addition and multiplication tables for a Galois field
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "primes.h"
#include "utils.h"

static const char *name = "maketab";

static void maketab_usage(void)
{
  fprintf(stderr, "%s: usage: %s <prime> <out file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *out;
  FILE *outp;
  unsigned int prime, i, j;
  prime_ops prime_operations;

  if (3 != argc) {
    maketab_usage();
    exit(1);
  }
  prime = strtoul(argv[1], NULL, 0);
  if ((!is_a_prime_power(prime)) || 1 == prime) {
    fprintf(stderr, "%s: bad prime power %u, terminating\n", name, prime);
    return 1;
  }
  out = argv[2];
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: unknown prime power %u, terminating\n", name, prime);
    return 1;
  }
  errno = 0;
  outp = fopen(out, "w");
  if (NULL == outp) {
    fprintf(stderr, "%s: failed to open %s for writing, terminating\n", name, out);
    return 1;
  }
  switch (prime) {
  case 9:
    fprintf(outp, "static word prod_table_%u[256] = {\n", prime);
    for (i = 0; i < 256; i++) {
      unsigned int prod = 0;
      unsigned int k = (i & 0xf0) >> 4;
      j = i & 0xf;
      if (0 != j && j < 9 && 0 != k && k < 9) {
        prod = (*prime_operations.mul)(j, k);
      }
      fprintf(outp, "%u", prod);
      if (15 != j) {
        fprintf(outp, ", ");
      } else if (255 != i) {
        fprintf(outp, ",\n");
      } else {
        fprintf(outp, "\n");
      }
    }
    fprintf(outp, "};\n");
    fprintf(outp, "static word add_table_%u[65536] = {\n", prime);
    for (i = 0; i < 256; i++) {
      for (j = 0; j < 256; j++) {
        unsigned int a = i & 0xf, b = (i & 0xf0) >> 4, c = j & 0xf, d = (j & 0xf0) >> 4, sum = 0;
        if (a < 9 && b < 9 && c < 9 && d < 9) {
          unsigned int sum1, sum2;
          sum1 = (*prime_operations.add)(a, c);
          sum2 = (*prime_operations.add)(b, d);
          sum = (sum2 << 4) | sum1;
        }
        fprintf(outp, "%u", sum);
        if (15 != (j % 16)) {
          fprintf(outp, ", ");
        } else if (255 != j || 255 != i) {
          fprintf(outp, ",\n");
        } else {
          fprintf(outp, "\n");
        }
      }
    }
    fprintf(outp, "};\n");
    fprintf(outp, "static word scale_table_%u[7][256] = {\n", prime);
    for (i = 2; i < 9; i++) {
      fprintf(outp, "  {\n");
      for (j = 0; j < 256; j++) {
        unsigned int k = j & 0xf, l = (j & 0xf0) >> 4, m = 0;
        if (0 == (j & 0xf)) {
          fprintf(outp, "  ");
        }
        if (k < 9 && l < 9) {
          m = (*prime_operations.mul)(i, k) | (((*prime_operations.mul)(i, l)) << 4);
        }
        fprintf(outp, "%u", m);
        if (15 != (j & 0xf)) {
          fprintf(outp, ", ");
        } else if (255 != j) {
          fprintf(outp, ",\n");
        } else {
          fprintf(outp, "\n");
        }
      }
      fprintf(outp, "  }");
      if (8 != i) {
        fprintf(outp, ",");
      }
      fprintf(outp, "\n");
    }
    fprintf(outp, "};\n");
    break;
  default:
    assert(assert_var_zero != 0);
    return 1;
  }
  fclose(outp);
  return 0;
}
