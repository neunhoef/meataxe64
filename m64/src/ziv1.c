/*
      ziv1.c     meataxe-64 invert
      ======     J. G. Thackray 11.09.2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parse.h"
#include "field.h"
#include "io.h"
#include "funs.h"
#include "mfuns.h"
#include "util.h"
#include "utils.h"
#include "parse.h"

static const char prog[] = "ziv1";

int main(int argc, const char * const argv[])
{
  uint64_t rank, nor;
  uint64_t *inv;
  FIELD *f;
  uint64_t hdr[5];
  const char *tmp_root = tmp_name();
  uint64_t tmp_len = strlen(tmp_root);
  char *m = mk_tmp(prog, tmp_root, tmp_len);

  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (3 !=argc) {
    LogString(80,"usage ziv1 input inverse");
    exit(14);
  }
  EPeek(argv[1], hdr);
  if(hdr[0] == 1) { /* Ordinary matrix */
    if(hdr[2] != hdr[3]) {
      printf("Invert - matrix not square\n");
      exit(15);
    }
    rank = fRecurse_ECH(IV, 1, prog, tmp_root, argv[1], 0, "NULL", "NULL", m,
                        "NULL", "NULL");
    if(rank!=hdr[2]) {
      printf("Invert - Matrix is singular\n");
      remove(m);
      free(m);
      exit(13);
    }
    /* Now negate result */
    f = malloc(FIELDLEN);
    FieldASet(hdr[1], f);
    fScalarMul(m, 1, argv[2], 0, f->charc - 1);
    remove(m);
    free(m);
  } else {
    if (hdr[0] == 3) { /* invert for permutations */
      EFIL *e2;
      inv = perm_inv(argv[1], &nor);
      e2 = EWHdr(argv[2], hdr);
      EWData(e2, 8 * nor, (uint8_t *)inv);
      EWClose(e2);
      free(inv);
    } else {
      printf("Cannot invert objects of type %ld\n", hdr[0]);
      free(m);
    }
  }
  return 0;
}
