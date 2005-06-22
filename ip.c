/*
 * $Id: ip.c,v 1.18 2005/06/22 21:52:53 jon Exp $
 *
 * Read a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zcv";

static void ip_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  u32 prime, nob, nod, noc, nor;
  u32 i, j;
  word base_mask;
  const header *h;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    ip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  errno = 0;
  inp = fopen(in, "r");
  if (NULL == inp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  endian_init();
  if (0 == read_text_header(inp, &h, in, name)) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  if ( 1 == prime) {
    header_set_noc((header *)h, nor);
  }
  header_set_prime((header *)h, prime);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    header_free(h);
    exit(1);
  }
  if (1 == prime) {
    /* Input of a map or permutation */
    header_free(h);
    noc = nor;
    for (i = 0; i < nor; i++) {
      u32 j;
      fscanf(inp, "%d", &j);
      if (0 == j || j > noc) {
        fprintf(stderr, "%s: %d (out of range 1 - %d) found as permutation image, terminating\n", name, j, noc);
        exit(1);
      }
      errno = 0;
      if (0 == endian_write_word(j - 1, outp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write output value %d in row %d to %s\n", name, j - 1, i, out);
        fclose(inp);
        fclose(outp);
        exit(1);
      }
    }
  } else {
    noc = header_get_noc(h);
    nob = header_get_nob(h);
    nod = header_get_nod(h);
    header_free(h);
    base_mask = (1 << nob) - 1;
    for (i = 0; i < nor; i++) {
      word a = 0;
      u32 k = 0;
      for (j = 0; j < noc; j++) {
        word e;
        if (get_element_from_text(inp, nod, prime, &e)) {
          a |= e << (k * nob);
          k++;
          if ((k + 1) * nob > bits_in_word) {
            errno = 0;
            if (0 == endian_write_word(a, outp)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "Failed to write element to %s at (%d, %d)\n",
                      out, i, j);
              fclose(inp);
              fclose(outp);
              exit(1);
            }
            k = 0;
            a = 0;
          }
        } else {
          fprintf(stderr, "Failed to read element from %s at (%d, %d)\n",
                  in, i, j);
          fclose(inp);
          fclose(outp);
          exit(1);
        }
      }
      errno = 0;
      if (0 != k && 0 == endian_write_word(a, outp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "Failed to write element to %s at (%d, %d)\n",
                out, i, j);
        fclose(inp);
        fclose(outp);
        exit(1);
      }
    }
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
