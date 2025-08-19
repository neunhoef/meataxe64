/*
      recech.c     meataxe-64 recursive echelise
      ========     J. G. Thackray   29.07.2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "funs.h"
#include "utils.h"
#include "mfuns.h"
#include "util.h"
#include "io.h"
#include "bitstring.h"

/*
 * This algorithm relies on 8 subroutines: the tasks in the paper
 * The parameter names reflect the names in the paper, but the packages
 * are split into their individual components
 */

/* TODO: translate the proggy for MKR ADI */

/* Task 1: Extend */
static void Extend(const char *arho, const char *erho_in,
                   const char *erho_out, const char *edelta_out, uint64_t j)
{
  if (1 == j) {
    fPivotCombine0(arho, 1, erho_out, 1, edelta_out, 1);
  } else {
    fPivotCombine(erho_in, 1, arho, 1, erho_out, 1, edelta_out, 1);
  }
}

/* Task 2: RowLengthen */
static void RowLengthen(const char *name, const char *temp,
                        unsigned int str_len, const char *min,
                        const char *rho1, const char *rho2, const char *mout)
{
  char *lam = mk_tmp(name, temp, str_len);
  fMKR(rho1, 1, rho2, 1, lam, 1);
  fColumnRiffleZero(lam, 1, min, 1, mout, 1);
  remove(lam);
  free(lam);
}

/* Task 3: ClearUp */
static void ClearUp(const char *rin, const char *x, const char *m,
                    const char *rout, const char *tmp)
{
  /* R := R + X * M */
  fMultiplyAdd(tmp, x, 1, m, 1, rin, 1, rout, 1);
}

/* Task 4: PreClearUp */
static void PreClearUp(const char *b, const char *dgamma, const char *x,
                       const char *r)
{
  fColumnExtract(dgamma, 1, b, 1, x, 1, r, 1);
}

/* Task 6: ClearDown */
/*
 * Note that this only produces lambda and a.a
 * in the case i != 1, where lambda comes from pivot combine
 * Note also that the column select output is the extension, for this
 * column, obtained by processing row i
 */
static void ClearDown(const char *name, const char *temp,
                      const char *c, const char *dgin, const char *drin, uint64_t i,
                      const char *dgout, const char *drout, /* outs */
                      const char *a, const char *m, const char *k,
                      const char *rho, const char *e, const char *lam) /* outs */
{
  if ( 1 == i) {
    /* First row, just echelise */
    uint64_t r = fRecurse_ECH(0, name, temp, c, rho, dgout, m, k, drout);
    NOT_USED(r);
  } else {
    unsigned int str_len = strlen(temp);
    char *nsel = mk_tmp(name, temp, str_len);
    char *H = mk_tmp(name, temp, str_len);
    char *gamma = mk_tmp(name, temp, str_len);
    char *R = mk_tmp(name, temp, str_len);
    char *nsel1 = mk_tmp(name, temp, str_len);
    char *nsel2 = mk_tmp(name, temp, str_len);
    fColumnExtract(dgin, 1, c, 1, a, 1, nsel, 1);
    fMultiplyAdd(temp, a, 1, drin, 1, nsel, 1, H, 1);
    fRecurse_ECH(0, name, temp, H, rho, gamma, m, k, R);
    fColumnExtract(gamma, 1, drin, 1, e, 1, nsel2, 1);
    fMultiplyAdd(temp, e, 1, R, 1, nsel2, 1, nsel1, 1);
    fPivotCombine(dgin, 1, gamma, 1, dgout, 1, lam, 1);
    fRowRiffle(lam, 1, nsel1, 1, R, 1, drout, 1);
    /* Remove and free internal temporaries */
    remove(nsel2);
    free(nsel2);
    remove(nsel1);
    free(nsel1);
    remove(R);
    free(R);
    remove(gamma);
    free(gamma);
    remove(H);
    free(H);
    remove(nsel);
    free(nsel);
  }
}

/* Task 7: UpdateRow */
static void UpdateRow(const char *name, const char *temp,
                      const char *a, const char *m, const char *k, /* ins */
                      const char *rho, const char *e, const char *lam, /* ins */
                      const char *cin, const char *bin, unsigned int i,/*ins*/
                      const char *cout, const char *bout) /* outs */
{
  unsigned int str_len = strlen(temp);
  char *V = mk_tmp(name, temp, str_len);
  char *W = mk_tmp(name, temp, str_len);
  if (1 == i) {
    /* Short version: REX, MUL, MAD, finessing Z and X */
    fRowExtract(rho, 1, cin, 1, V, 1, W, 1);
    fMultiply(temp, m, 1, V, 1, bout, 1);
  } else {
    /* Long version: MAD, REX, MUL, MAD, RRF, MAD */
    char *Z = mk_tmp(name, temp, str_len);
    char *X = mk_tmp(name, temp, str_len);
    char *S = mk_tmp(name, temp, str_len);
    fMultiplyAdd(temp, a, 1, bin, 1, cin, 1, Z, 1);
    fRowExtract(rho, 1, Z, 1, V, 1, W, 1);
    fMultiply(temp, m, 1, V, 1, X, 1);
    fMultiplyAdd(temp, e, 1, X, 1, bin, 1, S, 1);
    fRowRiffle(lam, 1, S, 1, X, 1, bout, 1);
    /* Free and delete S, X, Z */
    remove(S);
    remove(X);
    remove(Z);
    free(S);
    free(X);
    free(Z);
  }
  fMultiplyAdd(temp, k, 1, V, 1, W, 1, cout, 1);
  remove(V);
  remove(W);
  free(V);
  free(W);
}

/* Task 8: UpdateRowTrafo */
static void UpdateRowTrafo(const char *name, const char *temp,
                           const char *a, const char *m, const char *k, /* ins */
                           const char *rho, const char *e, const char *lam, /* ins */
                           const char *kin, const char *min, const char *edelta, /*ins*/
                           uint64_t i, uint64_t h, uint64_t j, /* ins */
                           const char *kout, const char *mout) /* outs */
{
  unsigned int str_len = strlen(temp);
  /* Create temporaries, used by all cases except j = 1 && h = i */
  char *S = mk_tmp(name, temp, str_len);
  char *Z = mk_tmp(name, temp, str_len);
  char *V = mk_tmp(name, temp, str_len);
  char *W = mk_tmp(name, temp, str_len);
  char *X = mk_tmp(name, temp, str_len);
  if (1 == j) {
    /* First + is empty, as is step 1, cf UpdateRow for i == 1 */
    /* Sub case h == i */
    if (h == i) {
      /* Step 2 and second + case empty */
      /* Use A.M in place of X */
      if (1 == i) {
        /* Step 4 empty when i = 1*/
        /* Step 5 output M is X which is A.M */
        copy_file(mout, m);
      } else {
        /* Step 4: MUL to temp S := A.E * X = A.E * A.M  in this case*/
        fMultiply(temp, e, 1, m, 1, S, 1);
        /* Step 5 when i != 1 */
        fRowRiffle(lam, 1, S, 1, m, 1, mout, 1);
      }
      /* Step 6 output K is A.K */
      copy_file(kout, k);
      /* Only one temporary created here */
    } else {
      /* h != i case */
      /* Step 1 Z := A.A * M */
      fMultiply(temp, a, 1, min, 1, Z, 1);
      /* Step 2 REX */
      fRowExtract(rho, 1, Z, 1, V, 1, W, 1);
      /* Step + empty as j = 1 || h != i */
      /* Step 3 X := A.M * V */
      fMultiply(temp, m, 1, V, 1, X, 1);
      /* Step 4 MAD S := M + A.E * X */
      fMultiplyAdd(temp, e, 1, X, 1, min, 1, S, 1);
      /* Step 5 M := RRF(lambda, S, X) */
      fRowRiffle(lam, 1, S, 1, X, 1, mout, 1);
      /* Step 6 K := A.K * V + W */
      fMultiplyAdd(temp, k, 1, V, 1, W, 1, kout, 1);
      /* Finally remove temporaries */
      remove(X);
      remove(W);
      remove(V);
      remove(Z);
    }
    remove(S);
  } else {
    /* j > 1 case */
    char *Kd = mk_tmp(name, temp, str_len); /* Original overwrites kin */
    /* + step column riffle 0 into kout from kin and edelta */
    fColumnRiffleZero(edelta, 1, kin, 1, Kd, 1);
    /* Now split according to whether h = i or h != i */
    if (h == i) {
      char *Vd = mk_tmp(name, temp, str_len); /* Original overwrites V */
      /* Step 1: Copy Kd to Z, finessed */
      /* Row extract with rho and Z */
      fRowExtract(rho, 1, Kd, 1, V, 1, W, 1); /* Step 2 */
      /* Step +: ADI */
      /* This would overwrite V, so we need another temporary */
      fAddIdentity(edelta, 1, V, 1, Vd, 1); /* Care needed in steps 3 and 6 */
      /* Step 3 X := A.M * Vd */
      fMultiply(temp, m, 1, Vd, 1, X, 1);
      /* Steps 4 and 5 depends on if i = 1 */
      if (1 == i) {
        /* No step 4, step 5 copies X to M */
        copy_file(mout, X);
      } else {
        /* S := A.E * X */
        fMultiply(temp, e, 1, X, 1, S, 1);
        /* M := RRF(A.lambda, S, X) */
        fRowRiffle(lam, 1, S, 1, X, 1, mout, 1);
      }
      /* Step 6: K := W + A.K * V */
      fMultiplyAdd(temp, k, 1, Vd, 1, W, 1, kout, 1);
      remove(Vd);
      free(Vd);
    } else {
      fMultiplyAdd(temp, a, 1, min, 1, Kd, 1, Z, 1); /* Step 1 */
      fRowExtract(rho, 1, Z, 1, V, 1, W, 1); /* Step 2 */
      /* Step +: not done as h != i */
      /* Step 3 X := A.M * V */
      fMultiply(temp, m, 1, V, 1, X, 1);
      /* Step 4 MAD S := M + A.E * X */
      fMultiplyAdd(temp, e, 1, X, 1, min, 1, S, 1);
      /* Step 5: M := RRF(A.lambda, S, X) */
      fRowRiffle(lam, 1, S, 1, X, 1, mout, 1);
      /* Step 6: K := W + A.K * V */
      fMultiplyAdd(temp, k, 1, V, 1, W, 1, kout, 1);
    }
    remove(Kd);
    remove(X);
    remove(W);
    remove(V);
    remove(Z);
    remove(S);
    free(Kd);
  }
  free(X);
  free(W);
  free(V);
  free(Z);
  free(S);
}

/* Assume we'll only do 2x2, but make bigger splits easy */
#define ROW_SPLIT 2
#define COL_SPLIT 2

/* A macro to convert a nx2 2d array to 2nx1, plus the size */
#define index2(i, j) ((COL_SPLIT * ((i) - 1)) + (j) - 1)
#define size2(i, j) ((i) * (j) * sizeof(const char *))

/* A macro to convert a nx2x2 3d array to 4nx1 */
#define index3(h, i, j) ((ROW_SPLIT * COL_SPLIT * ((h) - 1)) + (COL_SPLIT * ((i) - 1)) + (j) - 1)
#define size3(h, i, j) ((h) * (i) * (j) * sizeof(const char *))

/*
 * This is basically a reimplementation of the contents of mech.c
 * using filres and recursion rather than memory objects and iteration
 */
uint64_t fRecurse_ECH(int first, /* Is this top level */
                      const char *name, /* The name of the calling program */
                      const char *temp, /* Base for creating tmp files */
                      const char *input, const char *row_sel,
                      const char *col_sel, const char *multiplier,
                      const char *cleaner, const char *remnant)
{
  /*
   * This implements the algorithm called the CHIEF
   * It uses a great deal of temporary files
   * Some have 2 subscripts, and some have 2 subscripts and a superscript
   * For those with a superscript, that will be the slowest changing
   * The temporaries are A, B, C, D, K, M, R, X
   * Of these B, C, K, M, R have superscripts as well
   * D comes in 2 halves, DR and DGamma
   * Some subscripts and superscripts exceed 2, and some go below 1
   * Those going below 1 are B, D and R
   * For D, the callee using 0 (ClearDown) doesn't reference D in this case
   * Similarly for B, UpdateRow doesn't reference the zero case
   * But step 3 does define and use the superscript 0
   * Those that exceed 2 occur for C, K, M
   * M in particular can get to 5 in step 3
   * There is also E, a pair of bistrings rho and delta
   * and D, a bitstring gamma and a pair of files R, Rdash
   *
   * The main loops, on i and j, will go from 1 to 2,
   * with h, k and l as dictated in the CHIEF.
   * This allows each temporary to be allocated at the start
   * with a unique name. All temporaries will be allocated by mk_tmp
   * which increments a static idx to ensure uniqueness even across recursion.
   * The various temporaries will be allocated arrays, with macros
   * to convert the various subscripts and superscripts to a falt range
   *
   * Ultimately the algorithm will call fFullEchelise within task 6
   * and that will in turn recurse to here
   * if it cannot complete the request in the available memory
   *
   * For testing purposes, we will instead recurse directly to here
   * until such time as the file to be echelised has
   * only one row or one column
   *
   * Allowing fFullEchelise to recurse to here is a separate project
   * as it will require calibration in the way multiply does
   * to determine whether to do the job itself or pass it back
   * As a first step, it could simply bounce anything
   * with less than 50 rows and columns
   */
  /* Step -1: Check we can actually split, ie nor, noc >= 2 */
  int mode = (1 == first) ? 0 : 1;
  header hdr;
  uint64_t nor;
  uint64_t noc;
  EPeek(input, hdr.hdr);
  nor = hdr.named.nor;
  noc = hdr.named.noc;
  if ((1 == nor || 1 == noc) || (0 == first)) {
    /* Too small for us */
    return fFullEchelize(temp, input, mode, row_sel, mode, col_sel, mode,
                         multiplier, mode, cleaner, mode, remnant, mode);
  } else {
    uint64_t rank = 0;
    unsigned int h, i, j, k, l;
    char **B = malloc(size3(2, 2, 2)); /* Max super 2 */
    char **C = malloc(size3(3, 2, 2)); /* Max super 3 */
    char **K = malloc(size3(3, 2, 2)); /* Max super 3 */
    char **M = malloc(size3(5, 2, 2)); /* Max super 5 */
    /* R needs more care, as it starts at 0 rather than 1 */
    char **R = malloc(size3(3, 2, 2)); /* Min super 0 */
    char **A = malloc(size2(2, 2));
    char **X = malloc(size2(2, 2));
    /* D, the one in 2 parts */
    char **DR = malloc(size2(2, 2));
    char **DGamma = malloc(size2(2, 2));
    /* E, the other one in two parts */
    char **ERho = malloc(size2(2, 2));
    char **EDelta = malloc(size2(2, 2));
    unsigned int tmp_len = strlen(temp);
    char *Atmp = mk_tmp(name, temp, tmp_len);
    char *Etmp = mk_tmp(name, temp, tmp_len);
    char *Ktmp = mk_tmp(name, temp, tmp_len);
    char *Mtmp = mk_tmp(name, temp, tmp_len);
    char *rho = mk_tmp(name, temp, tmp_len);
    char *lambda = mk_tmp(name, temp, tmp_len);
    uint64_t fdef;
    FIELD *f;
    unsigned int *chp = malloc(COL_SPLIT * sizeof(*chp));
    unsigned int *chpcol = malloc(COL_SPLIT * sizeof(*chpcol));

    /* Step 0: chop input using chop to give C superscript 1, at 2 x 2 */
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the C matrix file names */
          C[index3(h, i, j)] = mk_tmp(name, temp, tmp_len);
        }
      }
    }
    /* Now allocate the other temporaries */
    /* First those with superscripts */
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the K and R matrix file names */
          K[index3(h, i, j)] = mk_tmp(name, temp, tmp_len);
          R[index3(h, i, j)] = mk_tmp(name, temp, tmp_len);
        }
      }
    }
    for (h = 1; h <= ROW_SPLIT * 2; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the M matrix file names */
          M[index3(h, j, i)] = mk_tmp(name, temp, tmp_len);
        }
      }
    }
    for (h = 1; h <= COL_SPLIT; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the B matrix file names */
          B[index3(h, i, j)] = mk_tmp(name, temp, tmp_len);
        }
      }
    }
    /* Now those without superscripts */
    for (i = 1; i <= ROW_SPLIT; i++) {
      for (j = 1; j <= COL_SPLIT; j++) {
        /* Create the A, X, DR, DGamma, ERho, EDelta matrix file names */
        A[index2(i, j)] = mk_tmp(name, temp, tmp_len);
        X[index2(i, j)] = mk_tmp(name, temp, tmp_len);
        DR[index2(i, j)] = mk_tmp(name, temp, tmp_len);
        DGamma[index2(i, j)] = mk_tmp(name, temp, tmp_len);
        ERho[index2(i, j)] = mk_tmp(name, temp, tmp_len);
        EDelta[index2(i, j)] = mk_tmp(name, temp, tmp_len);
      }
    }
    {
      const char *C1[] = {C[index3(1, 1, 1)], C[index3(1, 1, 2)],
        C[index3(1, 2, 1)], C[index3(1, 2, 2)]}; /* Needs work if bigger split */
      chop(input, 2, C1);
    }
    /* Step 1: down clearing */
    /* This step uses temporaries A, E, M, K, rho, lambda */
    for (i = 1; i <= ROW_SPLIT; i++) {
      for (j = 1; j <= COL_SPLIT; j++) {
        /* It's unclear that this needs a loop index dependent A */
        /* For the moment I've use Atmp instead */
        ClearDown(name, temp, C[index3(j, i, j)],
                  (1 == i) ? NULL : DGamma[index2(i - 1, j)],
                  (1 == i) ? NULL : DR[index2(i - 1, j)], i,
                  DGamma[index2(i, j)], DR[index2(i, j)], Atmp,
                  Mtmp, Ktmp, rho, Etmp, lambda);
        Extend(rho, (1 == j) ? NULL : ERho[index2(i, j - 1)],
               ERho[index2(i, j)], EDelta[index2(i, j)], j);
        for (k = j + 1; k <= COL_SPLIT; k++) {
          UpdateRow(name, temp, Atmp, Mtmp, Ktmp, rho, Etmp, lambda,
                    C[index3(j, i, k)],
                    (1 == i) ? NULL : B[index3(i - 1, j, k)], i,
                    C[index3(j + 1, i, k)], B[index3(i, j, k)]);
        }
        for (h = 1; h <= i; h++) {
          UpdateRowTrafo(name, temp, Atmp, Mtmp, Ktmp, rho, Etmp, lambda,
                         K[index3(j, i, h)],
                         (1 == i) ? NULL : M[index3(i - 1, j, h)],
                         EDelta[index2(h, j)], i, h, j,
                         K[index3(j + 1, i, h)], M[index3(i, j, h)]);
          /* Don't need input K so remove */
          remove(K[index3(j, i, h)]);
          /* Don't need input M so remove */
          if (1 != i) {
            remove(M[index3(i - 1, j, h)]);
          }
        }
      }
    }
    remove(Atmp);
    free(Atmp);
    remove(Etmp);
    free(Etmp);
    remove(Ktmp);
    free(Ktmp);
    remove(Mtmp);
    free(Mtmp);
    remove(rho);
    free(rho);
    remove(lambda);
    free(lambda);
    /* Step 2: multiplier lengthening */
    for (j = 1; j <= COL_SPLIT; j++) {
      for (h = 1; h <= ROW_SPLIT; h++) {
        RowLengthen(name, temp, tmp_len,
                    M[index3(ROW_SPLIT, j, h)], ERho[index2(h, j)],
                    ERho[index2(h, COL_SPLIT)],
                    M[index3(1 + ROW_SPLIT, j, h)]);
        /* Don't need input M so remove */
        remove(M[index3(ROW_SPLIT, j, h)]);
      }
    }
    /* Step 3: back cleaning */
    for (k = 1; k <= COL_SPLIT; k++) {
      /* Note we run the superscript on R from 1 to 3 rather than 0 to 2 */
      copy_file(R[index3(1, k, k)], DR[index2(ROW_SPLIT, k)]);
    }
    for (k = COL_SPLIT; k >= 1; k--) {
      for (j = 1; j + 1 <= k; j++) {
        PreClearUp(B[index3(ROW_SPLIT, j, k)], DGamma[index2(ROW_SPLIT, k)],
                   X[index2(j, k)], R[index3(1, j, k)]);
        for (l = k; l <= COL_SPLIT; l++) {
          ClearUp(R[index3(l + 1 - k, j, l)], X[index2(j, k)],
                  R[index3(l + 1 - k, k, l)], R[index3(l + 2 - k, j, l)],
                  temp);
        }
        for (h = 1; h <= ROW_SPLIT; h++) {
          char *T = mk_tmp(name, temp, tmp_len);
          /* Note the increment on the superscripts on M */
#if 1
          ClearUp(M[index3(ROW_SPLIT + 1, j, h)], X[index2(j, k)],
                  M[index3(ROW_SPLIT + 1, k, h)],
                  T,
                  temp);
          rename(T, M[index3(ROW_SPLIT + 1, j, h)]);
#else /* Removed in favour of not incrementing superscripts */
          ClearUp(M[index3(ROW_SPLIT + h, j, h)], X[index2(j, k)],
                  M[index3(ROW_SPLIT + h, k, h)],
                  M[index3(ROW_SPLIT + h + 1, j, h)],
                  temp);
#endif
          free(T);
        }
      }
    }
    /* Step 4: splice the multiplier, cleaner and remnant, concatenate the selects */
    /* We should be able to find how this is done in mech.c */
    /* compute the column-select bit string */
    /*
     * We need the superscript ROW_SPLIT of the elements DGamma
     * for all the column block indixes j from 1 to COL_SPLIT
     * ie DGamma[index2(ROW_SPLIT, j)] for all j
     * We can also compute some of the values needed for the remnant here
     * Note: this doesn't depend on back cleaning
     */
    {
      uint64_t cslen = 2 * sizeof(uint64_t) + ((noc + 63) / 64) * sizeof(uint64_t);
      uint64_t *bscs = malloc(cslen);
      uint64_t *inbs = malloc(cslen); /* For the partial pivots, can't be any longer */
      uint64_t shift;
      EFIL *e;
      memset(bscs, 0, cslen);
      shift = 0;
      for (j = 1; j <= COL_SPLIT; j++) {
        header hdr1;
        EFIL *e1 = ERHdr(DGamma[index2(ROW_SPLIT, j)], hdr1.hdr);
        uint64_t in_size = sizeof(uint64_t) * (2 + (hdr1.named.nor + 63) / 64);
        ERData(e1, in_size, (uint8_t *)inbs);
        ERClose1(e1, 1);
        BSShiftOr(inbs, shift, bscs);
        shift += hdr1.named.nor;
        rank += hdr1.named.noc;
        chp[j - 1] = hdr1.named.noc;
        chpcol[j - 1] = hdr1.named.nor - hdr1.named.noc;
      }
      bscs[0] = noc;
      bscs[1] = rank;
      hdr.hdr[0] = 2;
      hdr.hdr[1] = 1;
      hdr.hdr[2] = noc;
      hdr.hdr[3] = rank;
      hdr.hdr[4] = 0;
      e = EWHdr(col_sel, hdr.hdr);
      EWData(e, cslen, (uint8_t *)bscs);
      EWClose1(e, mode);
      free(bscs);
      free(inbs);
    }
    /* now write out cleaner  */
    {
      unsigned int *chr = malloc(ROW_SPLIT * sizeof(*chr));
      unsigned int *chrcol = malloc(ROW_SPLIT * sizeof(*chrcol));
      unsigned int *corstart = malloc(ROW_SPLIT * sizeof(*corstart));
      uint64_t maxrows = 0;
      header hdr, hdr2;
      DSPACE ds, ds1;
      Dfmt *buf, *buf1;
      EFIL *e;
      /* First work out the shape */
      /* Row shape */
      for (i = 0; i < ROW_SPLIT; i++) {
        EPeek(K[index3(COL_SPLIT + 1, i + 1, 1)], hdr.hdr);
        fdef = hdr.named.fdef; /* Need this at some point */
        chr[i] = hdr.named.nor;
        if (maxrows < (uint64_t)chr[i]) {
          maxrows = chr[i];
        }
      }
      /* Column shape */
      k = 0;
      for (h = 0; h < ROW_SPLIT; h++) {
        EPeek(K[index3(COL_SPLIT + 1, ROW_SPLIT, h + 1)], hdr.hdr);
        chrcol[h] = hdr.named.noc;
        corstart[h] = k;
        k += chrcol[h];
      }
      f = malloc(FIELDLEN);
      FieldASet1(fdef,f,NOMUL);
      DSSet(f, k, &ds);
      /* FIXME: The following code assumes we can fit the cleaner in memory. We may not want that */
      buf = malloc(ds.nob * maxrows);
      hdr.named.rnd1 = 1;
      hdr.named.fdef = f->fdef;
      hdr.named.nor = nor - rank;
      hdr.named.noc = rank;
      hdr.named.rnd2 = 0;
      e = EWHdr(cleaner, hdr.hdr);
      for (i = 0; i < ROW_SPLIT; i++) {
        memset(buf, 0, chr[i] * ds.nob);
        for (h = 0; h <= i; h++) {
          EFIL *e2 = ERHdr(K[index3(COL_SPLIT + 1, i + 1, h + 1)], hdr2.hdr);
          DSSet(f, chrcol[h], &ds1);
          buf1 = malloc(ds1.nob * hdr2.named.nor);
          ERData(e2, ds1.nob * hdr2.named.nor, (uint8_t *)buf1);
          DPaste(&ds1, buf1, chr[i], corstart[h], &ds, buf);
          ERClose1(e2, 1);
          free(buf1);
          remove(K[index3(COL_SPLIT + 1, i + 1, h + 1)]);
        }
        EWData(e, chr[i] * ds.nob, (uint8_t *)buf);
      }
      EWClose1(e, mode);
      free(chr);
      free(chrcol);
      free(corstart);
      free(buf);
      /* FIXME: We can remove the cleaner files here and free the strings */
    }
    /* now write out remnant, very similar to cleaner */
#define ROW_BY_ROW
    {
      unsigned int *colstart = malloc(COL_SPLIT * sizeof(*colstart));
#ifdef ROW_BY_ROW
      header hdr, *hdrs;
      DSPACE ds, *dss;
      Dfmt *buf, **bufs;
      EFIL *e, **es;
#else
      uint64_t maxrows = 0;
      header hdr, hdr2;
      DSPACE ds, ds1;
      Dfmt *buf, *buf1;
      EFIL *e;
#endif
      k = 0;
      for (j = 0; j < COL_SPLIT; j++) {
        /* We've already acquired chp and chpcol whilst writing the column select */
#ifndef ROW_BY_ROW
        if (maxrows < chp[j]) {
          /*
           * Every block in a column has the same number of columns
           * This is an upper bound on the rank of any block
           * and hence an upper bound on the number of rows in its remnant
           */
          maxrows = chp[j];
        }
#endif
        colstart[j] = k;
        k += chpcol[j];
      }
      DSSet(f, noc - rank, &ds); /* Width of remnant */
#ifdef ROW_BY_ROW
      buf = malloc(ds.nob); /* Space for one output row */
      /* Allocate headers, buffers and DSs */
      hdrs = malloc(COL_SPLIT * sizeof(*hdrs));
      es = malloc(COL_SPLIT * sizeof(*es));
      bufs = malloc(COL_SPLIT * sizeof(*bufs));
      dss = malloc(COL_SPLIT * sizeof(*dss));
#else
      buf = malloc(ds.nob * maxrows); /* Space for width * depth */
#endif
      hdr.named.rnd1 = 1;
      hdr.named.nor = rank;
      hdr.named.noc = noc - rank;
      hdr.named.fdef = f->fdef;
      hdr.named.rnd2 = 0;
      e = EWHdr(remnant, hdr.hdr);
      for (j = 0; j < COL_SPLIT; j++) {
        /* j indexes blocks of remnant fragments */
        /* What we want to do here is to do each set of column blocks
         * one row at a time. So we must open all the
         * fragments that will be indexed by j at once
         * buf must be allocated 1 row in size (as if maxrows were 1)
         * Do we know if all fragments have the same number of rows?
         * Not sure, so we may have to remember nor for each of them and
         * take care not to exceed it
         * So we need an array of headers, indexed by j,
         * rather than a single hdr2, and a corresponding array of EFIL
         * rather than just e2
         */
#ifdef ROW_BY_ROW
        uint64_t row_num = 0; /* This will count output rows */
        /* Read in all the headers for this row */
        for (k = j; k < COL_SPLIT; k++) {
          es[k] = ERHdr(R[index3((j == k) ? 1 : COL_SPLIT, j + 1, k + 1)], hdrs[k].hdr);
          DSSet(f, chpcol[k], &dss[k]); /* Width of this column segment */
          /* We only allocate one row for each fragment */
          bufs[k] = malloc(dss[k].nob); /* Space for this fragment */
        }
        while (row_num < chp[j]) {
          memset(buf, 0, ds.nob); /* Zero 1 row of output */
          for (k = j; k < COL_SPLIT; k++)  {
            /* Read a row from es[j] if we haven't exceeded it max rows
             * then paste it in */
            if (row_num < hdrs[k].named.nor) {
              ERData(es[k], dss[k].nob, (uint8_t *)bufs[k]);
              /* Only paste 1 row */
              DPaste(&dss[k], bufs[k], 1, colstart[k], &ds, buf);
            }
          }
          row_num++;
          /* Done a row, write to putput */
          EWData(e, ds.nob, (uint8_t *)buf);
        }
        /* Now free and all the stuff we allocated earlier */
        for (k = j; k < COL_SPLIT; k++) {
          ERClose1(es[k], 1);
          free(bufs[k]);
        }
#else
        memset(buf, 0, chp[j] * ds.nob); /* Zero block of output */
        for (k = j; k < COL_SPLIT; k++) {
          /*
           * k indexs across column blocks
           * The final remnant is block upper triangular
           * so as we go down we have less and less blocks to paste together
           */
          /* Get R[J,k] latest version and paste in */
          EFIL *e2;
          /* This is all a bit tricky
           * I think the possible superscript gets higher
           * the bigger the splits. The on diagonal fragment is superscript 0
           * the others come from UpdateRowTrafo and have maximum superscript
           */
          e2 = ERHdr(R[index3((j == k) ? 1 : COL_SPLIT, j + 1, k + 1)], hdr2.hdr);
          DSSet(f, chpcol[k], &ds1); /* Width of this column segment */
          buf1 = malloc(ds1.nob * hdr2.named.nor); /* Space for this fragment */
          ERData(e2, ds1.nob * hdr2.named.nor, (uint8_t *)buf1);
          /* Take the data described by ds1, and paste
           * chp[j] rows into the output starting at colstart[k]
           */
          DPaste(&ds1, buf1, chp[j], colstart[k], &ds, buf);
          ERClose1(e2, 1);
          free(buf1);
        }
        EWData(e, chp[j] * ds.nob, (uint8_t *)buf);
#endif
      }
      free(chp);
      free(chpcol);
      free(colstart);
      free(buf);
      EWClose1(e, mode);
#ifdef ROW_BY_ROW
      /* free headers, buffers and DSs */
      free(hdrs);
      free(es);
      free(bufs);
      free(dss);
#endif
    }
    /* now write out multiplier */
    /*
     * We need
     * for j from 1 to COL_SPLIT - 1
     * for h from 1 to ROW_SPLIT
     * Changed to use the reviwed ClearUp
     * M super(ROW_SPLIT + 1) sub(j, h)
     */
    {
      unsigned int *chq = malloc(COL_SPLIT * sizeof(*chq));
      unsigned int *chqcol = malloc(ROW_SPLIT * sizeof(*chqcol));
      unsigned int *coqstart = malloc(ROW_SPLIT * sizeof(*coqstart));
      uint64_t maxrows = 0;
      DSPACE ds, ds1;
      header hdr, hdr1;
      Dfmt *buf, *buf1;
      EFIL *e, *e1;
      for (j = 0; j < COL_SPLIT; j++) {
        header hdr;
        EPeek(M[index3(ROW_SPLIT + 1, j + 1, 1)], hdr.hdr);
        chq[j] = hdr.named.nor;
        if (maxrows < (uint64_t)chq[j]) {
          maxrows = chq[j];
        }
      }
      k = 0;
      for (h = 0; h < ROW_SPLIT; h++) {
        EPeek(M[index3(ROW_SPLIT + 1, 1, h + 1)], hdr.hdr);
        chqcol[h] = hdr.named.noc;
        coqstart[h] = k;
        k += chqcol[h]; 
      }
      DSSet(f, k, &ds);
      buf = malloc(ds.nob * maxrows);
      hdr.named.rnd1 = 1;
      hdr.named.fdef = f->fdef;
      hdr.named.nor = rank;
      hdr.named.noc = rank;
      hdr.named.rnd2 = 0;
      e = EWHdr(multiplier, hdr.hdr);
      for (j = 0; j < COL_SPLIT; j++)  {
        memset(buf, 0, chq[j] * ds.nob);
        for (h = 0; h < ROW_SPLIT; h++)  {
          e1 = ERHdr(M[index3(ROW_SPLIT + 1, j + 1, h + 1)], hdr1.hdr);
          DSSet(f, chqcol[h], &ds1);
          buf1 = malloc(hdr1.named.nor * ds1.nob);
          ERData(e1, ds1.nob * hdr1.named.nor, (uint8_t *)buf1);
          DPaste(&ds1, buf1, chq[j], coqstart[h], &ds, buf);
          free(buf1);
          ERClose1(e1, 1);
          /* M no longer bneeded, so remove */
          remove(M[index3(ROW_SPLIT + 1, j + 1, h + 1)]);
        }
        EWData(e, chq[j] * ds.nob, (uint8_t *)buf);
      }
      EWClose1(e, mode);
      free(buf);
      free(chq);
      free(chqcol);
      free(coqstart);
    }
    free(f); /* Don't need the field any more */
    /* compute the row-select bit string */
    /*
     * We need the superscript COL_SPLIT of the elements E
     * for all the row block indixes i from 1 to ROW_SPLIT
     * ie ERho[index2(i, ROW_SPLIT)] for all i
     * Note: this doesn't depend on back cleaning
     */
    {
      uint64_t rslen = 2 * sizeof(uint64_t) + ((nor + 63) / 64) * sizeof(uint64_t);
      uint64_t *bsrs = malloc(rslen);
      uint64_t *inbs = malloc(rslen); /* For the partial pivots, can't be any longer */
      uint64_t shift;
      EFIL *e;
      memset(bsrs, 0, rslen);
      shift = 0;
      for (i = 1; i <= ROW_SPLIT; i++) {
        header hdr1;
        EFIL *e1 = ERHdr(ERho[index2(i, COL_SPLIT)], hdr1.hdr);
        uint64_t in_size = sizeof(uint64_t) * (2 + (hdr1.named.nor + 63) / 64);
        ERData(e1, in_size, (uint8_t *)inbs);
        ERClose1(e1, 1);
        BSShiftOr(inbs, shift, bsrs);
        shift += hdr1.named.nor;
      }
      bsrs[0] = nor;
      bsrs[1] = rank;
      hdr.hdr[0] = 2;
      hdr.hdr[1] = 1;
      hdr.hdr[2] = nor;
      hdr.hdr[3] = rank;
      hdr.hdr[4] = 0;
      e = EWHdr(row_sel, hdr.hdr);
      EWData(e, rslen, (uint8_t *)bsrs);
      EWClose1(e, mode);
      free(bsrs);
      free(inbs);
    }
    /* Remove temporary files */
    for (i = 1; i <= 2; i++) {
      for (j = 1; j <= 2; j++) {
        /* Those created by chop */
        remove(C[index3(1, i, j)]);
      }
    }
    /* Delete temporaries */
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          remove(C[index3(h, i, j)]);
        }
      }
    }
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Delete the R matrix file names */
          /*remove(K[index3(h, i, j)]); */
          remove(R[index3(h, i, j)]);
        }
      }
    }
    for (h = 1; h <= COL_SPLIT; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the B matrix file names */
          remove(B[index3(h, i, j)]);
        }
      }
    }
    for (i = 1; i <= ROW_SPLIT; i++) {
      for (j = 1; j <= COL_SPLIT; j++) {
        /* Remove the A, X, DR, DGamma, ERho, EDelta matrix files */
        remove(A[index2(i, j)]);
        remove(X[index2(i, j)]);
        remove(DR[index2(i, j)]);
        remove(DGamma[index2(i, j)]);
        remove(ERho[index2(i, j)]);
        remove(EDelta[index2(i, j)]);
      }
    }
    /* Free filenames */
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          free(C[index3(h, i, j)]);
        }
      }
    }
    /* First those with superscripts */
    for (h = 1; h <= COL_SPLIT + 1; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Create the K and R matrix file names */
          free(K[index3(h, i, j)]);
          free(R[index3(h, i, j)]);
        }
      }
    }
    for (h = 1; h <= ROW_SPLIT * 2; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Free the M matrix file names */
          free(M[index3(h, j, i)]);
        }
      }
    }
    for (h = 1; h <= COL_SPLIT; h++) {
      for (i = 1; i <= ROW_SPLIT; i++) {
        for (j = 1; j <= COL_SPLIT; j++) {
          /* Free the B matrix file names */
          free(B[index3(h, i, j)]);
        }
      }
    }
    /* Now those without superscripts */
    for (i = 1; i <= ROW_SPLIT; i++) {
      for (j = 1; j <= COL_SPLIT; j++) {
        /* Free the A, X, DR, DGamma, ERho, EDelta matrix file names */
        free(A[index2(i, j)]);
        free(X[index2(i, j)]);
        free(DR[index2(i, j)]);
        free(DGamma[index2(i, j)]);
        free(ERho[index2(i, j)]);
        free(EDelta[index2(i, j)]);
      }
    }
    free(ERho);
    free(EDelta);
    free(DGamma);
    free(DR);
    free(X);
    free(R);
    free(M);
    free(K);
    free(C);
    free(B);
    free(A);
    return rank;
  }
}
