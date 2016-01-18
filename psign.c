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
#include "read.h"
#include "utils.h"

int psign(const char *perm, const char *name)
{
  FILE *pinp = NULL;
  const header *h_inp;
  u32 prime, nob, nor, noc, len;
  int res = 0;
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
  nob = header_get_nob(h_inp);
  nor = header_get_nor(h_inp);
  noc = header_get_noc(h_inp);
  len = header_get_len(h_inp);
  if (noc != nor) {
    fprintf(stderr, "%s: perm must be square, terminating\n", name);
    fclose(pinp);
    header_free(h_inp);
    return 1;
  }
  NOT_USED(len);
  NOT_USED(nob);
  return res;
}
