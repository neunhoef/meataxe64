/*
 * Function compute the sign of a permutation
 *
 */

#include "psign.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "header.h"
#include "maps.h"
#include "read.h"
#include "utils.h"

int psign_value(word *perm, u32 nor)
{
  u32 start = 0;
  int sign = 1;
  int *done = malloc(sizeof(*done) * nor);
  /* Mark no values examined yet */
  start = 0;
  memset(done, 0, sizeof(*done) * nor);
  while (start < nor) {
    u32 cycle;
    while (start < nor && 0 != done[start]) {
      /* Search for next cycle start */
      start++;
    }
    /* Ok, now got a cycle we haven't done, or we've finished */
    if (start < nor) {
      done[start] = -1;
      cycle = perm[start];
      while (cycle != start) {
        /* Follow the cycle */
        sign *= -1; /* Invert the sign */
        done[cycle] = -1; /* Mark this one visited */
        cycle = perm[cycle]; /* On to the next */
      }
    }
  }
  return sign;
}

int psign(const char *perm, const char *name)
{
  FILE *pinp = NULL;
  const header *h_inp;
  u32 prime, nor, noc;
  int sign;
  word *perm_vals;
  assert(NULL != perm);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&pinp, &h_inp, perm, name)) {
    if (NULL != pinp) {
      fclose(pinp);
      header_free(h_inp);
    }
    return 1;
  }
  prime = header_get_prime(h_inp);
  if (1 != prime) {
    fprintf(stderr, "%s: perm must be a map, terminating\n", name);
    fclose(pinp);
    header_free(h_inp);
    return 1;
  }
  nor = header_get_nor(h_inp);
  noc = header_get_noc(h_inp);
  if (noc != nor) {
    fprintf(stderr, "%s: perm must be square, terminating\n", name);
    fclose(pinp);
    header_free(h_inp);
    return 1;
  }
  perm_vals = malloc_map(nor);
  if (0 == read_map(pinp, nor, perm_vals, name, perm)) {
    header_free(h_inp);
    map_free(perm_vals);
    fclose(pinp);
    return 1;
  }
  sign = psign_value(perm_vals, nor);
  header_free(h_inp);
  fclose(pinp);
  printf("%d\n", sign);
  return 0;
}
