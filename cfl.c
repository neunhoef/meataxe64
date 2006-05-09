/*
 * $Id: cfl.c,v 1.1 2006/05/09 22:04:10 jon Exp $
 *
 * Create a filter list
 *
 */

#include "cfl.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int create_filter_list(u32 prime, u32 count, const char *out, const char *name)
{
  u32 nob, nor, len, i;
  header *h;
  FILE *outp;
  word *row;
  prime_ops prime_operations;
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %" U32_F " is not a prime power, terminating\n", name, prime);
    return 1;
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: primes_init failed, terminating\n", name);
    return 1;
  }
  if (0 == count) {
    fprintf(stderr, "%s: no words requested, terminating\n", name);
    return 1;
  }
  if (0 == int_pow(prime, count, &nor)) {
    fprintf(stderr, "%s: %" U32_F " to the power %" U32_F " is too large, terminating\n", name, prime, count);
    return 1;
  }
  nor--;
  nob = bits_of(prime);
  /* Create header */
  h = header_create(prime, nob, digits_of(prime), count, nor);
  /* Open output */
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    header_free(h);
    return 1;
  }
  len = header_get_len(h);
  header_free(h);
  if (0 == memory_rows(len, 1000)) {
    fprintf(stderr, "%s: insufficient memory, terminating\n", name);
    fclose(outp);
    return 2;
  }
  /* Create row */
  row = memory_pointer(0);
  /* Init row */
  row_init(row, len);
  for (;;) {
    word elt = 0;
    for (i = 0; i < count; i++) {
      elt = get_element_from_row(nob, i, row);
      /* Add one to row and check for overflow */
      elt++;
      if (prime == elt) {
        elt = 0; /* Wrap */
      }
      put_element_to_row(nob, i, row, elt);
      if (0 != elt) {
        break;
      }
    }
    if (0 != elt) {
      errno = 0;
      if (0 == endian_write_row(outp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fclose(outp);
        return 1;
      }
    } else {
      break;
    }
  }
  fclose(outp);
  return 0;
}
