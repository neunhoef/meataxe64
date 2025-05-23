/*
 * Export a matrix to meataxe 2000 from meataxe 64
 *
 * Parameters:
 * 1: the meataxe 64 matrix (input)
 * 2: the meataxe 2000 matrix (output)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
/* m2000 headers */
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "write.h"
#include "utils.h"
/* m64 headers */
#include "field.h"
#include "io.h"
#include "slab.h"

static const char *name = "zexport";

int main(int argc, char **argv)
{
  EFIL *in;
  FIELD *f;
  DSPACE ds;
  FILE *outp = NULL;
  uint64_t hdr[5];
  uint32_t nor, noc, len, nob, fdef, i, elts_per_word;
  header *h;
  Dfmt *v;
  word *row, mask, val;

  LogCmd(argc,argv);
  /******  First check the number of input arguments  */
  if (3 != argc) {
    LogString(80, "usage zexport <m1> <m2>");
    exit(14);
  }
  /* open m64 matrix and read header*/
  in = ERHdr(argv[1], hdr);
  fdef = hdr[1];
  nor = hdr[2];
  noc = hdr[3];
  nob = bits_of(fdef);
  /* set up header structure for m2000 matrix */
  endian_init();
  h = header_create(fdef, nob, 1, noc, nor);
  /* open m2000 output */
  if (0 == open_and_write_binary_header(&outp, h, argv[2], name)) {
    if (NULL != outp) {
      fclose(outp);
      header_free(h);
    }
    return 0;
  }
  if (3 == hdr[0] && 1 == fdef) {
    /* This is a map, not a matrix */
    for (i = 0; i < nor; i++) {
      /* Get an input value */
      uint64_t j;
      ERData(in, 8, (uint8_t *)&j);
      errno = 0;
      if (0 == endian_write_word(j, outp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write output value %lu in row %u to %s\n", name, j - 1, i, argv[2]);
        fclose(outp);
        exit(1);
      }
    }
    fclose(outp);
    return 0;
  }
  f = malloc(FIELDLEN);
  if (f == NULL) {
    LogString(81, "Can't malloc field structure");
    exit(8);
  }
  FieldSet(fdef, f);
  DSSet(f, noc, &ds);
  v = malloc(ds.nob + 7);
  len = header_get_len(h);
  row = my_malloc(len * sizeof(*row));
  DSSet(f, noc, &ds);
  /* loop over rows of m64 producing rows of m2000 */
  mask = get_mask_and_elts(nob, &elts_per_word);
  NOT_USED(mask);
  for (i = 0; i < nor; i++) {
    int e;
    u32 j, k = 0, word_offset = 0;

    /* Read a row from m64 */
    ERData(in, ds.nob, v);
    switch (fdef) {
    case 2:
    case 4:
      /* Characteristic 2 case, just copy */
      e = endian_write_row(outp, (word *)v, len);
      break;
    default:
      /* Clear output row */
      memset(row ,0, len * sizeof(*row));
      errno = 0;
      val = 0;
      for (j = 0; j < noc; j++) {
        /* Get a field element from m64 and put in m2000 */
        FELT elt = DUnpak(&ds, j, v);
        val |= elt << (nob * k);
        k++;
        if (k >= elts_per_word) {
          row[word_offset] = val;
          val = 0;
          k = 0;
          word_offset++;
        }
      }
      if (k > 0) {
        row[word_offset] = val;
      }
      e = endian_write_row(outp, row, len);
      break;
    }
    if (0 == e) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row %u fto %s, terminating\n", name, i, argv[2]);
    }
  }
  /* close m2000 and m64 matrices */
  fclose(outp);
  ERClose(in);
  return(0);
}
