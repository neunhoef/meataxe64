/*
 * $Id: mmat.c,v 1.6 2002/01/06 16:35:48 jon Exp $
 *
 * Monster program
 * Based on Version 1 - 30th November 1998 by R.A.Parker and R.A.Wilson
 *
 * This program is supposed to write out a matrix in the Monster.
 * At present, the matrix required is hard-wired into the program.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mop.h"
#include "mtx.h"

static void mkvec(long pos, unsigned char *vecin)
{
  unsigned int i;
  assert(NULL != vecin);
  memset(vectemp, 0, 24611);
  FTOV(vectemp, pos/2, (pos+1)%2+1);

  memset(vecin, 0, VECLEN);
  ptr1=0;
  ptr2=0;
  for (i=0; i<90; i++)     FGAP(vectemp, vecin, 729, 3);
  FGAP(vectemp, vecin, 21870, 2);
  for (i=0; i<66; i++)     FGAP(vectemp, vecin, 162, 2);
  FGAP(vectemp, vecin, 198, 2);
  FGAP(vectemp, vecin, 71, 1);
}

static void wtrow(unsigned int row_num, unsigned char *vecout)
{
  unsigned int i;
  assert(NULL != vecout);
  memset(vectemp, 0, 24611);
  ptr1=0;
  ptr2=0;
  for (i=0; i<90; i++)     FUNGAP(vecout, vectemp, 729, 3);
  FUNGAP(vecout, vectemp, 21870, 2);
  for (i=0; i<66; i++)     FUNGAP(vecout, vectemp, 162, 2);
  FUNGAP(vecout, vectemp, 198, 2);
  FUNGAP(vecout, vectemp, 71, 1);
  put_row(row_num, 196882, 20000, vectemp);
}

int main(int argc, char *argv[])
{
  unsigned int i;
  suzel H, G1, G2, G3, G4;
  suzel z1, z2, z3, z4, z5, z6, z7, z8, z9;
  int type = -1;
  if (argc != 2) {
    fprintf(stderr, "Incorrect number of arguments to monster\n");
    exit(1);
  }
  if (strcmp(argv[1], "a") != 0 && strcmp(argv[1], "b") != 0) {
    fprintf(stderr, "Incorrect parameters to monster, should be 'a' or 'b'\n");
    exit(1);
  }
  type = (strcmp(argv[1], "a") == 0) ? 1 : 2;
  init();

  rdall();
  malsuz(&H);
  rdsuz1(H, "H.m");
  malsuz(&G1);
  malsuz(&G2);
  malsuz(&G3);
  malsuz(&G4);

  malsuz(&z1);
  malsuz(&z2);
  malsuz(&z3);
  malsuz(&z4);
  malsuz(&z5);
  malsuz(&z6);
  malsuz(&z7);
  malsuz(&z8);
  malsuz(&z9);

  cpsuz(A, z1);
  cpsuz(B, z2);

  suzmult(z1, z2, z3);
  PRINT=1;
  (void)suzor(z3);
  suzmult(z3, z3, z4);
  suzmult(z4, z4, z5);
  suzmult(z5, z5, z6);
  suzmult(z6, z6, z7);
  suzmult(z6, z7, z8);
  suzmult(z8, H, z9);
  suzmult(z9, z4, G1);
  (void)suzor(G1);

  suzmult(z3, z2, z4);
  (void)suzor(z4);
  suzmult(z4, z3, z5);
  suzmult(z5, z8, G2);

  suzmult(z3, z4, z5);
  suzmult(z3, z5, z6);
  suzmult(z6, z3, z7);
  suzmult(z7, z7, z8);
  suzmult(z7, z8, z9);
  suzmult(z9, z9, z8);
  suzmult(z7, z8, G3);
  (void)suzor(G3);

  suzmult(z4, z4, z5);
  suzmult(z4, z5, z6);
  suzmult(z6, z6, z7);
  suzmult(z7, z7, z6);
  suzmult(z3, z6, G4);

  suzmult(G2, G4, z9);
  (void)suzor(z9);

  for(i=0; i<196882; i++) {
    mkvec(i, vec1);
    switch (type) {
    case 1:
      vecsuz(vec1, G1, vec2);
      break;
    case 2:
      vecT(vec1, vec2);
      vecsuz(vec2, G2, vec1);
      vecT(vec1, vec2);
      vecsuz(vec2, G3, vec1);
      vecT(vec1, vec2);
      vecsuz(vec2, G4, vec1);
      vecT(vec1, vec2);
      break;
    default:
      fprintf(stderr, "Error in monster, unexpected generator type %d\n", type);
      exit(1);
    }
    if(i%1000==0) printf("%dth row done\n", i);
    wtrow(i, vec2);
  }
  return 0;
}

