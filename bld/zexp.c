/*
 * $Id: zexp.c,v 1.1 2012/06/21 17:01:07 jon Exp $
 * Export meataxe 64 file format to meataxe 2000 (JT)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
 
int main(int argc, const char * const argv[])
{
  FILE *f_in, *f_out;
  FIELD *f;
  uint64 fdef, nor, noc;
  size_t lenf, count;
  int res;
  DSPACE ds;
  Dfmt v1;
  uint64 i;
  uint32 prime, nor_32, noc_32, len;

  /******  First check the number of input arguments  */
  if (argc != 3) {
    fprintf(stderr, "usage zexp <mtx64_file> <mtx2000_file>\n");
    exit(1);
  }

  f_in = RdHdr(argv[1], &fdef, &nor, &noc);
  lenf = LenField(fdef);
  f = malloc(lenf);
  if (f == NULL) {
    fprintf(stderr, "Can't malloc field structure\n");
    Close(f_in);
    exit(1);
  }
  res = FieldSet(fdef, f);
  NOT_USED(res);
  DSSet(f, noc, &ds);
  v1 = malloc(ds.nob);
  if (v1 == NULL) {
    fprintf(stderr, "Can't malloc a single vector\n");
    Close(f_in);
    exit(1);
  }
  /* Open output */
  f_out = fopen(argv[2], "wb");
  /* Write output header */
  prime = (uint32)fdef;
  nor_32 = (uint32)nor;
  noc_32 = (uint32)noc;
  len = ((noc_32 + 31) / 8) / sizeof(uint32);
  count = fwrite(&prime, sizeof(uint32), 1, f_out);
  if (1 != count) {
    fprintf(stderr, "Failed to write prime to %s\n", argv[2]);
    fclose(f_out);
    Close(f_in);
    exit(1);
  }
  count = fwrite(&nor_32, sizeof(uint32), 1, f_out);
  if (1 != count) {
    fprintf(stderr, "Failed to write nor to %s\n", argv[2]);
    fclose(f_out);
    Close(f_in);
    exit(1);
  }
  count = fwrite(&noc_32, sizeof(uint32), 1, f_out);
  if (1 != count) {
    fprintf(stderr, "Failed to write noc to %s\n", argv[2]);
    fclose(f_out);
    Close(f_in);
    exit(1);
  }
  /* Copy matrix to output */
  /* Finish */
  for (i = 0; i < nor; i++) {
    RdMatrix(f_in, &ds, 1, v1);
    /* Binary data at v1 */
    count = fwrite(v1, sizeof(uint32), len, f_out);
    if (len != count) {
      fprintf(stderr, "Failed to write %u words to %s\n", len, argv[2]);
      fclose(f_out);
      Close(f_in);
      exit(1);
    }
  }
  fclose(f_out);
  Close(f_in);
  return 0;
}
