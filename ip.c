/*
 * $Id: ip.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Read a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "header.h"
#include "utils.h"
#include "read.h"
#include "write.h"
#include "elements.h"
#include "endian.h"

static void ip_usage(void)
{
  fprintf(stderr, "ip: usage: ip <in_file> <out_file>\n");
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  unsigned int prime, nob, noc, nor;
  unsigned int i, j;
  unsigned int base_mask;
  unsigned int bits_in_unsigned_int =
    CHAR_BIT * sizeof(unsigned int);
  header h;

  endian_init();
  if (3 != argc) {
    ip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  inp = fopen(in, "r");
  if (NULL == inp) {
    fprintf(stderr, "ip: cannot open %s\n, terminating", in);
    exit(1);
  }
  outp = fopen(out, "w");
  if (NULL == outp) {
    fprintf(stderr, "ip: cannot open %s\n, terminating", out);
    fclose(inp);
    exit(1);
  }
  if (0 == read_text_header(inp, &h, in)) {
    exit(1);
  }
  write_binary_header(outp, h, out);
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  base_mask = (1 << nob) - 1;
  for (i = 0; i < nor; i++) {
    unsigned int a = 0;
    unsigned int k = 0;
    for (j = 0; j < noc; j++) {
      unsigned int e;
      if (get_element(inp, nob, prime, &e)) {
        a |= e << (k * nob);
        k++;
        if ((k + 1) * nob > bits_in_unsigned_int) {
          if (0 == endian_write_int(a, outp)) {
            fprintf(stderr, "Failed to write element to %s at (%d, %d)\n",
                    out, i, j);
            exit(1);
          }
          k = 0;
          a = 0;
        }
      } else {
        fprintf(stderr, "Failed to read element from %s at (%d, %d)\n",
                in, i, j);
        exit(1);
      }
    }
    if (0 != k && 0 == endian_write_int(a, outp)) {
      fprintf(stderr, "Failed to write element to %s at (%d, %d)\n",
              out, i, j);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  printf("ip: all ok\n");
  return 1;
}
