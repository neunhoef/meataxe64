/*
         mfuns.c  -   matrix functions implementations
         =======      J.G.Thackray 13.10.2017
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"
#include "mfuns.h"
#include "funs.h"
#include "io.h"
#include "bitstring.h"
#include "slab.h"
#include "utils.h"

void make_plain(const char *zero_bs, const char *nref_bs, const char *in, const char *out, uint64_t fdef)
{
  /* A very naive implementation */
  EFIL *ezbs, *embs, *ei, *eo; /* zero bitstring, minus -1 bitstring, in, out */
  FELT min1;
  header hdrzbs, hdrmbs, hdrio;
  /*uint64_t hdrzbs[5], hdrmbs[5], hdrio[5];*/
  uint64_t nor, noci1, noci2, noci3, noco, sizz, sizm, i, j, k, l, m;
  uint64_t zeroes, ones; /* How many zeroes, ones in each row */
  FIELD *f;
  FELT felt;
  DSPACE dsi, dso; /* Matrix in and out descriptors */
  Dfmt *mo, *mi; /* Matrix in and out storage*/
  DSPACE cbs; /* Clipboard descriptor */
  Dfmt *cb; /* Clipboard storage */
  uint64_t *bstz = NULL, *bstm;
  int use_ei;

#if 0
  /* Print the various parts */
  if (NULL != zero_bs) {
    printf("make_plain: parameter zero_bs %s\n", zero_bs);
    zut_fn(zero_bs);
  }
  printf("make_plain: parameter nref_bs %s\n", nref_bs);
  zut_fn(nref_bs);
#endif
  ezbs = (NULL != zero_bs) ? ERHdr(zero_bs, hdrzbs.hdr) : NULL;
  embs = ERHdr(nref_bs, hdrmbs.hdr);
  noci3 = hdrmbs.named.nor; /* Total bits in nref bitstring */
  noci1 = hdrmbs.named.noc; /* Set bits in nref bitstring */
  use_ei = noci1 != noci3;
  if (use_ei) {
    ei = ERHdr(in, hdrio.hdr);    // remnant   = 1 fdef nor noc 0
    nor = hdrio.named.nor; /* Rows of input */
  } else {
    ei = NULL;
    nor = noci1;
    hdrio.named.noc = 0;
    hdrio.named.fdef = fdef;
    hdrio.named.nor = nor;
  }
  ones = noci1; /* As many ones as set bits */
  if (NULL != ezbs) {
    /* Some leading zeroes */
    noci2 = hdrzbs.named.noc; /* Set bits in zero bitstring */
    noco = hdrzbs.named.nor; /* Total bits in zero bitstring, ie amount to output */
    zeroes = noci2;
  } else {
    /* No leading zeroes */
    noci2 = 0;
    noco = hdrmbs.named.nor;
    zeroes = 0;
  }
  /* Compatibility check */
  if (noci1 != nor /* Number of pivots == number of rows */ || 
      hdrio.named.noc + noci1 + noci2 != noco /* Total columns check */) {
    LogString(80,"Inputs incompatible");
    exit(23);
  }
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  min1 = FieldNeg(f, 1);
  hdrio.named.noc = noco;
  /* Create the output header */
  eo = EWHdr(out, hdrio.hdr);  
  DSSet(f, noci3 - nor, &dsi); /* input space */
  DSSet(f, noco, &dso); /* output space */
  DSSet(f, noco, &cbs); /* clipboard space */
  /* Allocate space for a row of input, and a row of output */
  mi = malloc(dsi.nob);
  mo = malloc(dso.nob);
  cb = malloc(cbs.nob);
  /* Read the bitstring(s) */
  if (NULL != ezbs) {
    sizz = 8 * (2 + (noco + 63) / 64);
    bstz = malloc(sizz);
    ERData(ezbs, sizz, (uint8_t *)bstz);
  }
  sizm = 8 * (2 + (hdrmbs.named.nor + 63) / 64);
  bstm = malloc(sizm);
  ERData(embs, sizm, (uint8_t *)bstm);
  /* Now read through in row by row, inserting minus -1s and zeros */
  for (i = 0; i < nor; i++) {
    if (use_ei) { /* If not, ei is NULL */
      ERData(ei, dsi.nob, mi);
    }
    /* Clear output row */
    memset(mo, 0, dso.nob);
    /* TBD: put in -1s and contents of in. DCut and DPaste */
    k = 0; /* Bit position in riffle of -1s and input */
    l = 0; /* Bit position in -1s. Also counts total from bs in this row */
    m = 0; /* No zeroes output yet */
    for (j = 0; j < noco; j++) {
      if (NULL != ezbs && m < zeroes && BSBitRead(bstz, j)) {
        /* No action, we're putting in a zero */
        m++; /* one more zero out this row */
      } else {
        /* Putting in a -1 or something from the input */
        if (l < ones && BSBitRead(bstm, k)) {
          /* Put in a -1 if we're at the correct row */
          if (l == i) {
            /* Correct row */
            DPak(&dso, j, mo, min1);
          }
          /* Else leave as zero */
          l++; /* Next column for -1s */
        } else {
          if (m == zeroes && l == ones) {
            DCut(&dsi, 1, k-l, mi, &cbs, cb);
            DPaste(&cbs, cb, 1, j, &dso, mo);
            break; /* That's it, all done */
          }
          /* else do individually, pick the value out of input */
          felt = DUnpak(&dsi, k - l, mi);
          DPak(&dso, j, mo, felt);
        }
        k++; /* Next column in overall input */
      }
    }
    EWData(eo, dso.nob, mo);
  }
  /* Close files */
  if (use_ei) {
    ERClose1(ei, 1);
  }
  EWClose1(eo, 1);
  ERClose1(embs, 1);
  if (NULL != ezbs) {
    ERClose1(ezbs, 1);
  }
  /* Deallocate crap */
  free(mi);
  free(mo);
  free(f);
  free(bstm);
  free(cb);
  if (NULL != ezbs) {
    free(bstz);
  }
}

int ident(uint64_t fdef, uint64_t nor, uint64_t noc, uint64_t elt,
          const char *out)
{
  uint64_t hdr[5];
  EFIL *e;
  DSPACE ds;
  uint64_t i;
  int is_perm;

  hdr[1] = fdef;
  hdr[2] = nor;
  hdr[3] = noc;
  hdr[4] = 0;
  is_perm = 1 == fdef;
  if (is_perm) {
    hdr[0] = 3;
    e = EWHdr(out, hdr);
    for (i = 0; i < nor; i++) {
      EWData(e, 8, (uint8_t *)&i);
    }
  } else {
    FIELD *f;
    Dfmt *v1;
    hdr[0] = 1;
    e = EWHdr(out, hdr);
    f = malloc(FIELDLEN);
    if (f == NULL) {
      LogString(81, "Can't malloc field structure");
      exit(8);
    }
    FieldSet(fdef, f);
    DSSet(f, noc, &ds);
    v1 = malloc(ds.nob);
    if (v1 == NULL) {
      LogString(81,"Can't malloc a single vector");
      exit(9);
    }
    /******  for each row of the matrix  */
    for (i = 0; i < nor; i++) {
      memset(v1, 0, ds.nob);
      DPak(&ds, i, v1, elt);
      EWData(e, ds.nob, v1);
    }
    free(v1);
    free(f);
  }
  EWClose(e);
  return 1;
}

/* Triaged multiply */
void triage_multiply(const char *zbs, const char *sbs,
                     const char *rem, const char *in, const char *out,
                     const char *tmp_vars[], const char *fun_tmp)
{
  /* Triage in into 0 and 1 using zbs and sbs */
  fRowTriage(zbs, sbs, in, tmp_vars[0], tmp_vars[1]);
  /* Negate 0 into 2 */
  fNegate(tmp_vars[0], tmp_vars[2]);
  /* multiply rem by 1 into 3 */
  fMultiply(fun_tmp, rem, 1, tmp_vars[1], 1, tmp_vars[3], 1);
  /* add 2 and 3 into out */
  fAdd(tmp_vars[2], 1, tmp_vars[3], 1, out, 1);
}

/* Slicing and splicing */
void slice(const char *input, unsigned int slices, const char *output_stem)
{
  uint64_t fdef, nor, noc, i, j, rch[100]; 
  EFIL *inp; /* Input */
  EFIL *oup; /* Output */
  header hdr;
  FIELD *f;
  DSPACE ds;
  Dfmt *v1;
  char fn[200];
  const char *pt;
  int lfn;

  if (slices >= 100) {
    fprintf(stderr, "%u slices is too many (>= 100), terminating\n", slices);
    exit(1);
  }
  inp = ERHdr(input, hdr.hdr);
  fdef = hdr.named.fdef;
  nor = hdr.named.nor;
  noc = hdr.named.noc;
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  DSSet(f, noc, &ds);
  j = nor / slices;
  for (i = 0; i < slices; i++) {
    rch[i] = j;
  }
  j = nor - j * slices;
  for (i = 0; i < j; i++) {
    rch[i]++;
  }
  lfn = 0;
  pt = output_stem;
  /* Create the root of the temporary file name */
  while ((*pt) != 0) {
    fn[lfn++] = *(pt++);
  }
  fn[lfn+2] = 0;

  v1 = malloc(ds.nob);
  for(i = 0; i < slices; i++) {
    /* Set the temporary file name */
    fn[lfn] = '0' + i / 10;
    fn[lfn+1] = '0' + i % 10;
    hdr.named.nor = rch[i];
    oup = EWHdr(fn, hdr.hdr);
    for(j = 0; j < rch[i]; j++) {
      ERData(inp, ds.nob, v1);
      EWData(oup, ds.nob, v1);
    }
    EWClose(oup);
  }
  ERClose(inp);
  free(f);
  free(v1);
}

void splice(const char *input_stem, unsigned int slices, const char *output)
{
  uint64_t fdef, nor, noc, norout, nor1, i, j;
  EFIL *inp; /* Input */
  EFIL *oup; /* Output */
  header hdr, hdr1;
  FIELD *f;
  DSPACE ds;
  Dfmt *v1;
  char fn[200];
  const char *pt;
  int lfn;

  if (slices >= 100) {
    fprintf(stderr, "%u slices is too many (>= 100), terminating\n", slices);
    exit(1);
  }
  lfn = 0;
  pt = input_stem;
  /* Create the root of the temporary file name */
  while ((*pt) != 0) {
    fn[lfn++] = *(pt++);
  }
  fn[lfn+2] = 0;
  fn[lfn] = '0';
  fn[lfn+1] = '0';

  EPeek(fn, hdr.hdr);
  fdef = hdr.named.fdef;
  nor = hdr.named.nor;
  norout = nor;
  noc = hdr.named.noc;
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  DSSet(f, noc, &ds);

  for(i = 1; i < slices; i++) {
    fn[lfn] = '0' + i / 10;
    fn[lfn+1] = '0' + i % 10;
    EPeek(fn, hdr1.hdr);
    if ((fdef != hdr1.named.fdef) || (noc != hdr1.named.noc)) {
      printf("Matrices incompatible\n");
      exit(7);
    }
    norout += hdr1.named.nor;
  }
  hdr.named.nor = norout;
  oup = EWHdr(output, hdr.hdr);

  v1 = malloc(ds.nob);
  for(i = 0; i < slices; i++) {
    /* Set the temporary file name */
    fn[lfn] = '0' + i / 10;
    fn[lfn+1] = '0' + i % 10;
    inp = ERHdr(fn, hdr1.hdr);
    nor1 = hdr1.named.nor;
    for(j = 0; j < nor1; j++) {
      ERData(inp, ds.nob, v1);
      EWData(oup, ds.nob, v1);
    }
    ERClose(inp);
  }
  EWClose(oup);
  free(f);
  free(v1);
}

void cat(const char *files[], const char *out, unsigned int count)
{
  unsigned int i;
  uint64_t fdef, noc, maxnor, norout;
  uint64_t hdr[5];
  DSPACE ds;
  EFIL *e1,*e2;
  FIELD *f;
  Dfmt *m;

  EPeek(files[0], hdr);
  fdef = hdr[1];
  noc = hdr[3];
  norout = hdr[2];
  maxnor = hdr[2];
  for (i = 1; i < count; i++) {
    EPeek(files[i], hdr);
    if ((fdef != hdr[1]) || (noc != hdr[3])) {
      printf("Matrices incompatible\n");
      exit(7);
    }
    norout += hdr[2];
    if (maxnor < hdr[2]) {
      maxnor = hdr[2];
    }
  }
  hdr[2] = norout;
  e2 = EWHdr(out, hdr);
  f = malloc(FIELDLEN);
  if (NULL == f) {
    LogString(81,"Can't malloc field structure");
    exit(22);
  }
  FieldASet(fdef, f);
  DSSet(f, noc, &ds);
  m = malloc(ds.nob * maxnor);
  for (i = 0; i < count; i++) {
    e1 = ERHdr(files[i], hdr);
    ERData(e1, ds.nob * hdr[2], m);
    EWData(e2, ds.nob * hdr[2], m);
    ERClose(e1);
  }
  EWClose(e2);
  free(m);
  free(f);
}

uint64_t *perm_inv(const char *perm, uint64_t *nor)
{
  uint64_t hdr[5];
  uint64_t noc, i, k;
  uint64_t *inv;
  EFIL *e1 = ERHdr(perm, hdr);
  *nor = hdr[2];
  noc = hdr[3];
  if (*nor != noc) {
    printf("Invert - map not square\n");
    exit(15);
  }
  inv = malloc(8 * noc);
  memset(inv, 0xff, 8 * noc);
  for (i = 0; i < noc; i++) {
    ERData(e1, 8, (uint8_t *)&k);
    if (0xffffffffffffffff == inv[k]) {
      inv[k] = i;
    } else {
      printf("Invert - map not invertible\n");
      exit(15);
    }
  }
  ERClose(e1);
  return inv;
}
