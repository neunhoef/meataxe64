/*
      zqa.c     meataxe-64 quotient action
      =====     J. G. Thackray   10.11.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "mfuns.h"
#include "io.h"
#include "slab.h"
#include "funs.h"
#include "bitstring.h"
#include "util.h"
#include "utils.h"

static const char prog_name[] = "zqa";

static char *fun_tmp;

#define FUN_TMP "_funs"
/* Get the non selected rows from a generator */
static void fRowExtract(const char *bs, const char *in, const char *nsel)
{
  /*
   * Open the file, open the bs
   * check file has rows = total bits in bitstring
   * Loop read a row, if bit clear output else ignore
   * Pool
   */
  EFIL *ebs, *ei, *eo; /* bitstring, in, out */
  header hdrbs, hdrio;
  FIELD *f;
  DSPACE ds; /* Matrix in */
  Dfmt *mi; /* Matrix out */
  uint64_t nor, noc, j, fdef, size;
  uint64_t *bst;
  ebs = ERHdr(bs, hdrbs.hdr);
  ei = ERHdr(in, hdrio.hdr);
  nor = hdrbs.named.nor; /* Total bits */
  noc = hdrio.named.noc; /* Elements per row */
  fdef = hdrio.named.fdef;
  if (nor != hdrio.named.nor || noc != nor) {
    /* Should be same as for generator */
    fprintf(stderr, "%s: %s, %s different number of rows or columns, exiting\n", prog_name, bs, in);
    exit(1);
  }
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  hdrio.named.nor = nor - hdrbs.named.noc; /* Only non selected rows */
  /* Create the output header */
  eo = EWHdr(nsel, hdrio.hdr);
  DSSet(f, noc, &ds); /* input/output space */
  mi = malloc(ds.nob);
  /* Read the bitstring */
  size = 8 * (2 + (nor + 63) / 64);
  bst = malloc(size);
  ERData(ebs, size, (uint8_t *)bst);
  for (j = 0; j < nor; j++) {
    ERData(ei, ds.nob, mi);
    if (!BSBitRead(bst, j)) {
      EWData(eo, ds.nob, mi);
    }
  }
  EWClose1(eo, 0);
  ERClose1(ei, 0);
  ERClose1(ebs, 0);
  free(mi);
  free(bst);
  free(f);
}

int main(int argc, const char *argv[])
{
  /* Set up tmp stem tmp_pid*/
  const char *tmp_root = tmp_name();
  /* How long the temporary filename root is */
  size_t tmp_len = strlen(tmp_root);
  /* The non selected rows */
  char *seln = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Selected columns of non selected rows */
  char *selc = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Non selected columns of non selected rows */
  char *selcn = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Multiply result */
  char *selm = mk_tmp(prog_name, tmp_root, tmp_len);
  /* The root of the subspace */
  const char *sub_root;
  unsigned int sub_root_len;
  char *sub_bs;
  char *sub_rem;
  header hdr, hdr_gen;
  int is_perm;

  CLogCmd(argc, argv);
  /* Check command line <vecs> <output stem> [<gen>*] */
  /* Must be exactly 3 args */
  if (argc != 4) {
    LogString(80,"usage zqa <subspace stem> <generator> <output>");
    exit(21);
  }
  sub_root = argv[1];
  sub_root_len = strlen(sub_root);
  sub_bs = malloc(sub_root_len + 4);
  sub_rem = malloc(sub_root_len + 5);
  strcpy(sub_bs, sub_root);
  strcat(sub_bs, ".bs");
  strcpy(sub_rem, sub_root);
  strcat(sub_rem, ".rem");
  /* Temporary root for functions */
  fun_tmp = malloc(tmp_len + sizeof(FUN_TMP) + 1);
  strcpy(fun_tmp, tmp_root);
  strcat(fun_tmp, FUN_TMP);
  /*
   * Algorithm
   * zrx sub.bs gen junk seln (probably do this by steam)
   * zcx sub.bs seln selc selcn
   * zmu selc sub.rem selm
   * zad selm selcn out
   */
  /*
   * For permutations the above doesn't work
   * as we can't row or column select on the generator
   * So we have to do the multiplication first
   */
  EPeek(argv[2], hdr_gen.hdr);
  is_perm = 1 == hdr_gen.named.fdef;

  if (is_perm) {
    int res;
    /*
     * Let the subspace have dimension r and codimension s
     * Then rem is r x s. We extend by adding an s x s identity
     * We can now multiply by the generator on the left
     * and take the non-selected rows of the result
     * This gives us C*rem+N, where C is the selected columns
     * and N is the non-selected columns, both of the
     * notional non-selected rows of the permutation
     */
    /* */
    EPeek(sub_rem, hdr.hdr);
    res = ident(hdr.named.fdef, hdr.named.noc, hdr.named.noc, 1, seln);
    NOT_USED(res);
    /* Use guts of zcn (split out) to create selc */
    fMultiply(fun_tmp, argv[2], 0, selc, 0, selm, 0);
    /* Now take the non-selected part of selm */
    fRowExtract(sub_bs, seln, argv[3]);
  } else {
    /* Row select non-selected rows from gen (argv[2]) */
    fRowExtract(sub_bs, argv[2], seln);
    fColumnExtract(sub_bs, 0, seln, 0, selc, 0, selcn, 0);
    /* Clean using the pivots */
    fMultiply(fun_tmp, selc, 0, sub_rem, 0, selm, 0);
    /* Add result of clean into non-selected part */
    fAdd(selm, 0, selcn, 0, argv[3], 0);
  }
  remove(seln);
  remove(selc);
  remove(selcn);
  remove(selm);
  free(seln);
  free(selc);
  free(selcn);
  free(selm);
  return 0;
}
