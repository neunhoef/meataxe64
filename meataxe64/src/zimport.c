/*
 * Import a matrix from meataxe 2000 to meataxe 64
 *
 * Parameters:
 * 1: the meataxe 2000 matrix (input)
 * 2: the meataxe 64 matrix (output)
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
#include "read.h"
#include "utils.h"
/* m64 headers */
#include "field.h"
#include "io.h"
#include "slab.h"

static const char *name = "zimport";

int main(int argc, char **argv)
{
  EFIL *out;
  FIELD *f;
  DSPACE ds;
  FILE *inp = NULL;
  uint64_t hdr[5];
  uint32_t nor, noc, len, nob, fdef, i, elts_per_word, k;
  const header *h;
  Dfmt *v;
  word mask, *row, val = 0;

  LogCmd(argc,argv);
  /******  First check the number of input arguments  */
  if (3 != argc) {
    LogString(80, "usage zimport <m1> <m2>");
    exit(14);
  }
  /* open m2000 matrix and read header*/
  endian_init();
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    if (NULL != inp) {
      fclose(inp);
      header_free(h);
    }
    return 0;
  }
  /* set up field structure for m64 matrix */
  fdef = header_get_prime(h);;
  hdr[1] = fdef;
  nor = header_get_nor(h);
  hdr[2] = nor;
  noc = header_get_noc(h);
  hdr[3] = noc;
  hdr[4] = 0;
  if ( 1== fdef) {
    word j;
    /* Importing a permutation */
    hdr[0] = 3;
    out = EWHdr(argv[2], hdr);
    for (i = 0; i < nor; i++) {
      if (0 == endian_read_word(&j, inp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read word in row %u from %s, terminating\n", name, i, argv[1]);
        fclose(inp);
        exit(1);
      }
      EWData(out, 8, (uint8_t *)&j);
    }
  } else {
    hdr[0] = 1;
    len = header_get_len(h);
    nob = header_get_nob(h);
    row = my_malloc(len * sizeof(*row));
    f = malloc(FIELDLEN);
    if (f == NULL) {
      LogString(81, "Can't malloc field structure");
      exit(8);
    }
    FieldSet(fdef, f);
    DSSet(f, noc, &ds);
    /* Allow room for a full word of output at the end */
    v = malloc(ds.nob + 7);
    /* open m64 output */
    out = EWHdr(argv[2], hdr);
    /* loop over rows of m2000 producing rows of m64 */
    mask = get_mask_and_elts(nob, &elts_per_word);
    for (i = 0; i < nor; i++) {
      int e;
      u32 j, word_offset = 0;
      errno = 0;
      e = endian_read_row(inp, row, len);
      if (0 == e) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row %u from %s, terminating\n", name, i, argv[1]);
        fclose(inp);
        exit(1);
      }
      /* Clear output row */
      switch (fdef) {
      case 2:
      case 4:
        /* Characteristic 2 case, just copy */
        EWData(out, ds.nob, (Dfmt *)row);
        break;
      default:
        memset(v ,0, ds.nob + 7);
        k = 0;
        for (j = 0; j < noc; j++) {
          /* Get a field element from m2000 and put in m64 */
          word elt;
          if (0 == k) {
            val = get_elements_in_word_from_row(row + word_offset, 0, -1);
            k = elts_per_word;
            word_offset++;
          }
          elt = val & mask;
          DPak(&ds, j, v, elt);
          val >>= nob; /* Next value */
          k--;
        }
        EWData(out, ds.nob, v);
        break;
      }
    }
  }
  /* close m2000 and m64 matrices */
  fclose(inp);
  EWClose(out);
  return(0);
}
