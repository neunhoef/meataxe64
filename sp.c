/*
 * $Id: sp.c,v 1.1 2001/11/21 01:06:29 jon Exp $
 *
 * Function to spin some vectors under two generators
 *
 */

#include "sp.h"
#include "header.h"
#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;
  long pos;
  unsigned int nor;
  gen next;
};

static void cleanup(FILE *f1, FILE *f2, FILE *f3, FILE *f4)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
  if (NULL != f4)
    fclose(f4);
}

void spin(const char *in, const char *out, const char *a,
          const char *b, const char *name)
{
  FILE *inp, *outp, *f_a, *f_b;
  const header *h_in, *h_a, *h_b;
  header *h_out;
  unsigned int prime, nob, noc1, nor1, noc2, nor2, len1, len2;
  struct gen_struct gen_a, gen_b;
  gen_a.next = &gen_b;
  gen_b.next = &gen_a;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != a);
  assert(NULL != b);
  assert(NULL != name);
  inp = fopen(in, "rb");
  outp = fopen(out, "rb");
  f_a = fopen(a, "rb");
  f_b = fopen(b, "rb");
  if (NULL == inp || NULL == outp || NULL == f_a || NULL == f_b) {
    fprintf(stderr, "%s: failed to open one of %s, %s, %s, %s, terminating\n",
            name, in, out, a, b);
    cleanup(inp, outp, f_a, f_b);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h_in, name) ||
      0 == read_binary_header(f_a, &h_a, name) ||
      0 == read_binary_header(f_b, &h_b, name)) {
    fprintf(stderr, "%s: failed to read header from one of %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, outp, f_a, f_b);
    exit(1);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  noc1 = header_get_noc(h_in);
  nor1 = header_get_nor(h_in);
  len1 = header_get_len(h_in);
  noc2 = header_get_noc(h_a);
  nor2 = header_get_nor(h_a);
  len2 = header_get_len(h_a);
  if (noc2 != header_get_noc(h_b) ||
      nor2 != header_get_nor(h_b) ||
      prime != header_get_prime(h_a) ||
      prime != header_get_prime(h_b) ||
      nob != header_get_nob(h_a) ||
      nob != header_get_nob(h_b) ||
      noc1 != nor2) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, outp, f_a, f_b);
    exit(1);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in), noc1, nor1);
  /* Now allocate store and read input vectors */
  /* Now loop until done or out of memory
   * multiply rows gen->nor to max_rows into max_rows onwards
   * set gen->nor = max_rows
   * clean max_rows onwards with 0 - max_rows
   * full echelise max_rows onwards
   * back clean with max_rows onwards
   * shuffle up max_rows onwards
   * if no rows found, and none found last time, set done
   * set gen = gen->next
   * loop
   */
}
