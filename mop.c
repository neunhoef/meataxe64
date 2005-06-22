/*
 * $Id: mop.c,v 1.21 2005/06/22 21:52:53 jon Exp $
 *
 * Monster operations for meataxe
 *
 * Derived from mop.h version 1.6  --  30.11.98 by R.A.Parker and R.A.Wilson
 *
 */

#include "mop.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "rows.h"

unsigned char vectemp[VECLEN];
unsigned int ptr1, ptr2;
int PRINT;
unsigned char vec1[VECLEN], vec2[VECLEN];
static char suz1head[8], suz2head[8];
static unsigned char vorvec[VECLEN];
static unsigned char s90head[12];
static unsigned char s729head[12];
static unsigned char s142head[12];
static char w[256], ww[256], bar[256];
static unsigned char vechead[12], mathead[12];
static int s3[7][7];
static char v3[4][7];
static unsigned int suztab[32761];
static int Tperm[87752];
static word *T324a, *T324b, *T538;
static word *t729, *tw729, *tww729;
static word *vwork;
static unsigned char Tbact[87752];
static row_ops operations;

static suzel C, E, suzwork;

suzel A, B;

static unsigned char FFRV(const unsigned char *a, unsigned int b)
{
  unsigned char c;
  assert(NULL != a);
  c = a[b/4];
  return (c>>(2*(3-(b%4))))&3;
}
static unsigned char FFRV2(const unsigned char *a, unsigned int b)
{
  unsigned char c;
  assert(NULL != a);
  c =  a[(b>>3)];
  return ( c >> ( 7 - (b&7) ) ) & 1;
}

void FTOV(unsigned char *a, unsigned int b, unsigned char c)
{
  unsigned char f;
  unsigned int d, e;
  assert(NULL != a);
  d = b/4;
  e = 3-(b%4);
  f = a[d];
  f = f|(c<<(e<<1));
  a[d] = f;
  return;
}

void FGAP(const unsigned char *d, unsigned char *e, unsigned int f, unsigned int g)
{
  unsigned int h;
  assert(NULL != d);
  assert(NULL != e);
  for (h = 0; h < f; h++)
    FTOV(e, ptr2++, FFRV(d, ptr1++));
  ptr2 += g;
}

void FUNGAP(const unsigned char *d, unsigned char *e, unsigned int f, unsigned int g)
{
  unsigned int h;
  assert(NULL != d);
  assert(NULL != e);
  for (h = 0; h < f; h++)
    FTOV(e, ptr2++, FFRV(d, ptr1++));
  ptr1 += g;
}

void init(void)
{
  unsigned char uc;
  unsigned int i, j, k, l;
  strncpy(suz1head, "MONSUZ01", 8);
  strncpy(suz2head, "MONSUZ02", 8);
  vechead[0] = 2;
  vechead[1] = 0;
  vechead[2] = 0;
  vechead[3] = 0;
  vechead[4] = 1;
  vechead[5] = 0;
  vechead[6] = 0;
  vechead[7] = 0;
  vechead[8] = 18;
  vechead[9] = 1;
  vechead[10] = 3;
  vechead[11] = 0;
/*  GF2,  1 row,  196882 (=3 1 18 base 256) columns */
  mathead[0] = 2;
  mathead[1] = 0;
  mathead[2] = 0;
  mathead[3] = 0;
  mathead[4] = 18;
  mathead[5] = 1;
  mathead[6] = 3;
  mathead[7] = 0;
  mathead[8] = 18;
  mathead[9] = 1;
  mathead[10] = 3;
  mathead[11] = 0;
/*  GF2,  196882 rows,  196882 (=3 1 18 base 256) columns */

  s90head[0] = 4;
  s90head[1] = 0;
  s90head[2] = 0;
  s90head[3] = 0;
  s90head[4] = 90;
  s90head[5] = 0;
  s90head[6] = 0;
  s90head[7] = 0;
  s90head[8] = 90;
  s90head[9] = 0;
  s90head[10] = 0;
  s90head[11] = 0;

  s729head[0] = 4;
  s729head[1] = 0;
  s729head[2] = 0;
  s729head[3] = 0;
  s729head[4] = 217;
  s729head[5] = 2;
  s729head[6] = 0;
  s729head[7] = 0;
  s729head[8] = 217;
  s729head[9] = 2;
  s729head[10] = 0;
  s729head[11] = 0;

  s142head[0] = 2;
  s142head[1] = 0;
  s142head[2] = 0;
  s142head[3] = 0;
  s142head[4] = 142;
  s142head[5] = 0;
  s142head[6] = 0;
  s142head[7] = 0;
  s142head[8] = 142;
  s142head[9] = 0;
  s142head[10] = 0;
  s142head[11] = 0;

  for(i = 1; i < 7; i++) {
    s3[i][1] = i;
    s3[1][i] = i;
    s3[i][i] = 1;
  }
  s3[2][2] = 3;
  s3[3][3] = 2;
  s3[2][3] = 1;
  s3[3][2] = 1;
  s3[2][4] = 5;
  s3[2][5] = 6;
  s3[2][6] = 4;
  s3[3][4] = 6;
  s3[3][5] = 4;
  s3[3][6] = 5;
  s3[4][2] = 6;
  s3[5][2] = 4;
  s3[6][2] = 5;
  s3[4][3] = 5;
  s3[5][3] = 6;
  s3[6][3] = 4;
  s3[4][5] = 3;
  s3[5][4] = 2;
  s3[4][6] = 2;
  s3[6][4] = 3;
  s3[5][6] = 3;
  s3[6][5] = 2;
  memset(v3[0] + 1, 0, 6 * sizeof(v3[0][1]));
  for (i = 1; i < 4; i++) {
    for (j = 1; j < 4; j++) {
      v3[i][j] = (3+i-j)%3 +1;
      v3[i][j+3] = (4+j-i)%3 + 1;
    }
  }
  T324a = malloc(324 * wl324 * sizeof(*T324a));
  T324b = malloc(324 * wl324 * sizeof(*T324b));
  T538 = malloc( 538 * wl538 * sizeof(*T538));
  j = 0xaa;
  for (i = 0; i < 256; i++) bar[i] = ((i&j)>>1)^i;
  for (i = 0; i < 256; i++) w[i] = (i&j)^((i&j)>>1)^((i<<1)&j);
  for (i = 0; i < 256; i++) {
    uc = (unsigned char)w[i];
    ww[i] = w[uc];
  }
  t729 = malloc(90*wl729*sizeof(*t729));
  tw729 = malloc(90*wl729*sizeof(*tw729));
  tww729 = malloc(90*wl729*sizeof(*tww729));
  vwork = malloc(wl729*sizeof(*vwork));

  k = 1;
  l = 65880;
  for (i = 0; i < 21870; i++) suztab[k++] = l++;
  l += 2;
  for (i = 0; i < 66; i++) {
    for (j = 0; j < 162; j++) suztab[k++] = l++;
    l += 2;
  }
  for (i = 0; i < 198; i++) suztab[k++] = l++;
  rows_init(2, &operations);
}



static void rdvec(const char *filnam, unsigned char *vecin)
{
  FILE *f;
  unsigned int i;
  assert(NULL != filnam);
  assert(NULL != vecin);
  f = fopen(filnam, "rb");
  if (f == NULL) {
    printf("File %s does not exist!\n", filnam);
    exit(-1);
  }
  fread(vectemp, 1, 12, f);
  fread(vectemp, 1, 24611, f);
  memset(vecin, 0, VECLEN);
  ptr1 = 0;
  ptr2 = 0;
  for (i = 0; i < 90; i++)
    FGAP(vectemp, vecin, 729, 3);
  FGAP(vectemp, vecin, 21870, 2);
  for (i = 0; i < 66; i++)
    FGAP(vectemp, vecin, 162, 2);
  FGAP(vectemp, vecin, 198, 2);
  FGAP(vectemp, vecin, 71, 1);
  fclose(f);
}

void malsuz(suzel *m)
{
  suzel t;
  assert(NULL != m);
  t = (suzel)malloc(sizeof(suzex));
  t->m729 = malloc(729 * wl729 * sizeof(*t->m729));
  t->w729 = malloc(729 * wl729 * sizeof(*t->w729));
  t->ww729 = malloc(729 * wl729 * sizeof(*t->ww729));
  t->m90 = malloc(90 * wl90 * sizeof(*t->m90));
  t->w90 = malloc(90 * wl90 * sizeof(*t->w90));
  t->ww90 = malloc(90 * wl90 * sizeof(*t->ww90));
  t->m142 = malloc(142 * wl142 * sizeof(*t->m142));
  t->p32760 = malloc(32761 * sizeof(unsigned int));
  t->b32760 = malloc(32761);
  *m = t;
}

void rdsuz1(suzel m, const char *fn)
{
  FILE *f ;
  char *ptr;
  unsigned int *ptrl;
  unsigned char *ptrc;
  unsigned int i, j;
  unsigned char c[3];
  assert(NULL != m);
  assert(NULL != fn);
  m->greased = 0;
  m->inout = 0;
  f = fopen(fn, "rb");
  if (f == NULL) {
    printf("File %s does not exist!\n", fn);
    exit(-1);
  }
  fread(vectemp, 1, 8, f);
  if (0 == memcmp(vectemp, suz1head, 8)) m->inout = 1;
  if (0 == memcmp(vectemp, suz2head, 8)) m->inout = 2;
  if (m->inout == 0) {
    printf("File %s is not a Monster-Suzuki element\n", fn);
    exit(-1);
  }
  ptr = (char *)m->m729;
  for (i = 0; i < 729; i++) {
    fread(ptr, 1, 183, f);
    ptr += wl729 * sizeof(word);
  }
  ptr = (char *)m->m90;
  for (i = 0; i < 90; i++) {
    fread(ptr, 1, 23, f);
    ptr += wl90 * sizeof(word);
  }
  ptr = (char *)m->m142;
  for (i = 0; i < 142; i++) {
    fread(ptr, 1, 18, f);
    ptr += wl142 * sizeof(word);
  }

  ptrc = (m->b32760)+1;
  ptrl = (m->p32760)+1;
  for (i = 1; i <= 32760; i++) {
    fread(c, 1, 3, f);
    *(ptrc++) = c[2];
    j = 256*c[1]+c[0];
    *(ptrl++) = j;
  }
  fclose(f);
}

static void rdT(const char *fn)
{
  unsigned int i;
  unsigned char c[4];
  FILE *f;
  char *ptr;
  assert(NULL != fn);
  f = fopen(fn, "rb");
  if (f == NULL) {
    printf("File %s does not exist\n", fn);
    exit(-1);
  }
  fread(vectemp, 1, 8, f);
  ptr = (char *)T324a;
  for (i = 0; i < 324; i++) {
    fread(ptr, 1, 41, f);
    ptr += wl324 * sizeof(word);
  }
  ptr = (char *)T324b;
  for (i = 0; i < 324; i++) {
    fread(ptr, 1, 41, f);
    ptr += wl324 * sizeof(word);
  }
  ptr = (char *)T538;
  for (i = 0; i < 538; i++) {
    fread(ptr, 1, 68, f);
    ptr += wl538 * sizeof(word);
  }
  for (i = 0; i < 87750; i++) {
    fread(c, 1, 4, f);
    Tperm[i] = c[0]+256*c[1]+256*256*c[2];
    Tbact[i] = c[3];
    if (c[3] == 0) Tperm[i] = -1;
  }
  fclose(f);
}

void rdall(void)
{
  malsuz(&A);
  malsuz(&B);
  malsuz(&C);
  malsuz(&E);
  malsuz(&suzwork);
  rdsuz1(A, "A.m");
  rdsuz1(B, "B.m");
  rdsuz1(C, "C.m");
  rdsuz1(E, "E.m");
  rdT("T.m");
  rdvec("vorder.vec", vorvec);
}

static int grease(suzel m)
{
  unsigned int i, j;
  unsigned char uc;
  char *ptc1, *ptc2, *ptc3;
  word *ptw1, *ptw2, *ptw3;
  assert(NULL != m);
  if (m->greased == 1) return(1);
  ptw1 = m->m729;
  ptw2 = m->w729;
  ptw3 = m->ww729;
  for (i = 0; i < 729; i++) {
    ptc1 = (char *)ptw1;
    ptc2 = (char *)ptw2;
    ptc3 = (char *)ptw3;
    memset(ptw2, 0, (wl729) * sizeof(*ptw2));
    memset(ptw3, 0, (wl729) * sizeof(*ptw3));
    ptw2 += (wl729);
    ptw3 += (wl729);
    for (j = 0; j < 183; j++) {
      uc = (unsigned char)*(ptc1++);
      *(ptc2++) = w[uc];
      *(ptc3++) = ww[uc];
    }
    ptw1 += wl729;
  }
  ptw1 = m->m90;
  ptw2 = m->w90;
  ptw3 = m->ww90;
  for (i = 0; i < 90; i++) {
    ptc1 = (char *)ptw1;
    ptc2 = (char *)ptw2;
    ptc3 = (char *)ptw3;
    memset(ptw2, 0, (wl90) * sizeof(unsigned int));
    memset(ptw3, 0, (wl90) * sizeof(unsigned int));
    ptw2 += (wl90);
    ptw3 += (wl90);
    for (j = 0; j < 23; j++) {
      uc = (unsigned char)*(ptc1++);
      *(ptc2++) = w[uc];
      *(ptc3++) = ww[uc];
    }
    ptw1 += wl90;
  }
  m->greased = 1;
  return(0);
}

void vecsuz(const unsigned char *vecin, suzel m, unsigned char *vecout)
{
  unsigned char uc;
  unsigned int i, j, k;
  unsigned char entry, bact;
  unsigned char *ptc1, *ptc2, *ptc3;
  unsigned int *ptl1;
  word *ptw1, *ptw2, *ptw3, *ptw2w, *ptw2ww;
  assert(NULL != vecin);
  assert(NULL != m);
  assert(NULL != vecout);
  if (m->greased == 0) grease(m);
  memset(vecout, 0, VECLEN);
  j = 0;
  ptw1 = t729;
  for (i = 0; i < 90; i++) {
    memset(ptw1, 0, (wl729) * sizeof(word));
    ptw2 = m->m729;
    ptw2w = m->w729;
    ptw2ww = m->ww729;
    for (k = 0; k < 729; k++) {
      entry = FFRV(vecin, j);
      if (entry != 0) {
	ptw3 = ptw2;
	if (entry == 2) ptw3 = ptw2w;
	if (entry == 3) ptw3 = ptw2ww;
        (*operations.incer)(ptw3, ptw1, wl729);
      }
      ptw2 += wl729;
      ptw2w += wl729;
      ptw2ww += wl729;
      j++;
    }
    ptw1 += wl729;
    j += 3;
  }
  ptw1 = t729;
  ptw2 = tw729;
  ptw3 = tww729;
  for (i = 0; i < 90; i++) {
    ptc1 = (unsigned char *)ptw1;
    ptc2 = (unsigned char *)ptw2;
    ptc3 = (unsigned char *)ptw3;
    memset(ptc2, 0, (wl729) * sizeof(word));
    memset(ptc3, 0, (wl729) * sizeof(word));
    ptw2 += wl729;
    ptw3 += wl729;
    for (j = 0; j < 183; j++) {
      uc = (unsigned char)*(ptc1++);
      *(ptc2++) = w[uc];
      *(ptc3++) = ww[uc];
    }
    ptw1 += wl729;
  }
  ptc1 = vecout;
  ptw1 = m->m90;
  for (i = 0; i < 90; i++) {
    memset(vwork, 0, (wl729) * sizeof(*vwork));
    ptw2 = t729;
    ptw2w = tw729;
    ptw2ww = tww729;
    for (k = 0; k < 90; k++) {
      entry = FFRV((unsigned char *)ptw1, k);
      if (entry != 0) {
	ptw3 = ptw2;
	if (entry == 2) ptw3 = ptw2w;
	if (entry == 3) ptw3 = ptw2ww;
        (*operations.incer)(ptw3, vwork, wl729);
      }
      ptw2 += wl729;
      ptw2w += wl729;
      ptw2ww += wl729;
    }
    ptw1 += wl90;
    ptc2 = (unsigned char *)vwork;
    if (m->inout == 1) {
      memcpy(ptc1, ptc2, 183);
      ptc1 += 183;
    } else if (m->inout == 2) {
      for (j = 0; j < 183; j++) {
	uc = (unsigned char) *(ptc2++);
	*(ptc1++) = bar[uc];
      }
    }
  }
  ptl1 = (unsigned int *)(m->p32760) +1;
  ptc2 = (m->b32760) + 1;
  for (j = 1; j <= 32760; j++) {
    k = suztab[*(ptl1++)];
    i = suztab[j];
    bact = *(ptc2++);
    entry = FFRV(vecin, i);
    if (entry != 0) FTOV(vecout, k, v3[entry][bact]);
  }
  j = 197552;
  memset(vwork, 0, (wl142) * (sizeof(word)));
  ptw3 = m->m142;
  for (i = 0; i < 142; i++) {
    entry = FFRV2(vecin, j++);
    assert(0 == entry || 1 == entry);
    if (entry == 1) {
      (*operations.incer)(ptw3, vwork, wl729);
    }
    ptw3 += wl142;
  }
  memcpy(ptc1 + 8224, vwork, 18);
}


void vecT(unsigned char *vecin, unsigned char *vecout)
{
  unsigned int i, j, l;
  unsigned char entry;
  unsigned char *ptc1;
  word *ptw3;

  assert(NULL != vecin);
  assert(NULL != vecout);
  memset(vecout, 0, VECLEN);
  for (j = 0; j < 87750; j++) {
    int k = Tperm[j];
    if (k == -1) continue;
    entry = FFRV(vecin, j);
    if (entry != 0) FTOV(vecout, k, v3[entry][Tbact[j]]);
  }

  j = 175504;

  ptc1 = vecout+21938;
  for (l = 0; l < 11; l++) {
    memset(vwork, 0, (wl324) * sizeof(word));
    ptw3 = T324a;
    for (i = 0; i < 324; i++) {
      entry = FFRV2(vecin, j++);
      assert(0 == entry || 1 == entry);
      if (1 == entry) {
        (*operations.incer)(ptw3, vwork, wl324);
      }
      ptw3 += wl324;
    }
    memcpy(ptc1, vwork, 41);
    ptc1 += 41;
    j += 4;
  }
  for (l = 0; l < 55; l++) {
    memset(vwork, 0, (wl324) * sizeof(word));
    ptw3 = T324b;
    for (i = 0; i < 324; i++) {
      entry = FFRV2(vecin, j++);
      assert(0 == entry || 1 == entry);
      if (1 == entry) {
        (*operations.incer)(ptw3, vwork, wl324);
      }
      ptw3 += wl324;
    }

    memcpy(ptc1, vwork, 41);
    ptc1 += 41;
    j += 4;
  }
  memset(vwork, 0, (wl538) * sizeof(word));
  ptw3 = T538;
  for (i = 0; i < 396; i++) {
    entry = FFRV2(vecin, j++);
    assert(1 == entry || 0 == entry);
    if (1 == entry) {
      (*operations.incer)(ptw3, vwork, wl538);
    }
    ptw3 += wl538;
  }
  j += 4;
  for (i = 0; i < 142; i++) {
    entry = FFRV2(vecin, j++);
    assert(0 == entry || 1 == entry);
    if (1 == entry) {
      (*operations.incer)(ptw3, vwork, wl538);
    }
    ptw3 += wl538;
  }
  memcpy(ptc1, vwork, 68);
}

void cpsuz(suzel a, suzel b)
{
  assert(NULL != a);
  assert(NULL != b);
  b->greased = a->greased;
  b->inout = a->inout;
  memcpy(b->m729, a->m729, 729 * wl729 * sizeof(*a->m729));
  memcpy(b->m90, a->m90, 90 * wl90 * sizeof(*a->m90));
  memcpy(b->m142, a->m142, 142 * wl142 * sizeof(*a->m142));
  memcpy((b->p32760)+1, (a->p32760)+1, 32760 * sizeof(unsigned int));
  memcpy((b->b32760)+1, (a->b32760)+1, 32760);
  if (1 == a->greased) {
    memcpy(b->w729, a->w729, 729 * wl729 * sizeof(*a->w729));
    memcpy(b->w90, a->w90, 90 * wl90 * sizeof(*a->w90));
    memcpy(b->ww729, a->ww729, 729 * wl729 * sizeof(*a->ww729));
    memcpy(b->ww90, a->ww90, 90 * wl90 * sizeof(*a->ww90));
  }
}


void suzmult(suzel a, suzel b, suzel c)
{
  unsigned int i, j, k;
  char unsigned entry;
  unsigned char uc;
  unsigned char *ptc1, *ptc2, *ptc3;
  unsigned int *ptl1, *ptl2, *ptl3;
  word *ptw1, *ptw2, *ptw3, *ptw2w, *ptw2ww, *ptw2a;
  assert(NULL != a);
  assert(NULL != b);
  assert(NULL != c);
  cpsuz(b, suzwork);

  if (a->greased == 0) grease(a);
  if (a->inout == 2) {
    ptw1 = suzwork->m90;
    for (i = 0; i < 90; i++) {
      ptc1 = (unsigned char *)ptw1;
      for (j = 0; j < 23; j++) {
        uc = (unsigned char) *(ptc1);
        *(ptc1++) = bar[uc];
      }
      ptw1 += wl90;
    }
    ptw1 = suzwork->m729;
    for (i = 0; i < 729; i++) {
      ptc1 = (unsigned char *)ptw1;
      for (j = 0; j < 183; j++) {
        uc = (unsigned char) *(ptc1);
        *(ptc1++) = bar[uc];
      }
      ptw1 += wl729;
    }
    suzwork->greased = 0;
  }
  if (suzwork->greased == 0) grease (suzwork);
  c->greased = 0;
  c->inout = (((a->inout) + (suzwork->inout))%2) + 1;
  ptw1 = a->m729;
  ptw3 = c->m729;
  for (i = 0; i < 729; i++) {
    memset(ptw3, 0, (wl729) * (sizeof(*ptw3)));
    ptw2 = suzwork->m729;
    ptw2w = suzwork->w729;
    ptw2ww = suzwork->ww729;
    for (k = 0; k < 729; k++) {
      entry = FFRV((unsigned char *)ptw1, k);
      assert(entry <= 3);
      if (entry != 0) {
        ptw2a = ptw2;
        if (entry == 2) ptw2a = ptw2w;
        if (entry == 3) ptw2a = ptw2ww;
        (*operations.incer)(ptw2a, ptw3, wl729);
      }
      ptw2 += wl729;
      ptw2w += wl729;
      ptw2ww += wl729;
    }
    ptw1 += wl729;
    ptw3 += wl729;
  }
  ptw1 = suzwork->m90;
  ptw3 = c->m90;
  for (i = 0; i < 90; i++) {
    memset(ptw3, 0, (wl90) * sizeof(*ptw3));
    ptw2 = a->m90;
    ptw2w = a->w90;
    ptw2ww = a->ww90;
    for (k = 0; k < 90; k++) {
      entry = FFRV( (unsigned char *)ptw1, k);
      if (entry != 0) {
        ptw2a = ptw2;
        if (entry == 2) ptw2a = ptw2w;
        if (entry == 3) ptw2a = ptw2ww;
        (*operations.incer)(ptw2a, ptw3, wl90);
      }
      ptw2 += wl90;
      ptw2w += wl90;
      ptw2ww += wl90;
    }
    ptw1 += wl90;
    ptw3 += wl90;
  }
  ptl1 = (a->p32760 + 1);
  ptl2 = (suzwork->p32760);
  ptl3 = (c->p32760 + 1);
  ptc1 = (a->b32760)+1;
  ptc2 = (suzwork->b32760);
  ptc3 = (c->b32760)+1;
  for (j = 1; j <= 32760; j++) {
    k = *(ptl1++);
    *(ptl3++) = *(ptl2+k);
    *(ptc3++) = s3[*(ptc1++)][*(ptc2+k)];
  }
  ptw1 = a->m142;
  ptw3 = c->m142;
  for (i = 0; i < 142; i++) {
    memset(ptw3, 0, (wl142) * sizeof(*ptw3));
    ptw2 = suzwork->m142;
    for (k = 0; k < 142; k++) {
      entry = FFRV2((unsigned char *)ptw1, k);
      assert(0 == entry || 1 == entry);
      if (entry != 0) {
        (*operations.incer)(ptw2, ptw3, wl142);
      }
      ptw2 += wl142;
    }
    ptw1 += wl142;
    ptw3 += wl142;
  }
}

unsigned int suzor(suzel a)
{
  unsigned int i;
  assert(NULL != a);
  vecsuz(vorvec, a, vec2);
  for (i = 1; i <= 119; i++) {
    if(0 == memcmp(vec2, vorvec, VECLEN)) {
      if (PRINT == 1) printf("Order is %d\n", i);
      return(i);
    }
    memcpy(vec1, vec2, VECLEN);
    vecsuz(vec1, a, vec2);
  }
  return(120);
}
