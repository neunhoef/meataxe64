/*
 * $Id: ip.c,v 1.16 2002/10/14 19:11:51 jon Exp $
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
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  unsigned int prime, nob, nod, noc, nor;
  unsigned int i, j;
  unsigned int base_mask;
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
      unsigned int j;
      fscanf(inp, "%d", &j);
      if (0 == j || j > noc) {
        fprintf(stderr, "%s: %d (out of range 1 - %d) found as permutation image, terminating\n", name, j, noc);
        exit(1);
      }
      errno = 0;
      if (0 == endian_write_int(j - 1, outp)) {
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
      unsigned int a = 0;
      unsigned int k = 0;
      for (j = 0; j < noc; j++) {
        unsigned int e;
        if (get_element_from_text(inp, nod, prime, &e)) {
          a |= e << (k * nob);
          k++;
          if ((k + 1) * nob > bits_in_unsigned_int) {
            errno = 0;
            if (0 == endian_write_int(a, outp)) {
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
      if (0 != k && 0 == endian_write_int(a, outp)) {
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
