// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// functions header file

// First the large functions each in a separate own source file
#include "field.h"

extern void fTranspose(const char * tmp, const char *in, int sin,
                 const char *out, int sout);  //ftra.c
extern void fMultiply(const char * tmp, const char *m1, int s1,
                 const char *m2, int s2, const char *m3, int s3);  //fmul.c
extern uint64_t fProduceNREF(const char * tmp, const char *m1, int s1,
                     const char *b2, int s2,const char *m3, int s3);  //fpef.c
extern uint64_t fFullEchelize(const char *temp,
                const char *m1, int s1, const char *b2, int s2,
                const char *b3, int s3, const char *m4, int s4,
                const char *m5, int s5, const char *m6, int s6);  // fech.c
// funs1 (Gaussian-related) functions
extern void fColumnExtract(const char *bs, int sbs, const char *in, int sin,
                           const char *sel, int ssel, const char * nsel, int snsel);
extern void fRowExtract(const char *bs, int sbs, const char *in, int sin,
                        const char *sel, int ssel, const char * nsel, int snsel);
extern void fRowRiffle(const char *bs, int sbs, const char * ins, int sins,
                       const char * inn, int sinn, const char * out, int sout);
/* Full pivot combine */
extern void fPivotCombine(const char *b1, int sb1, const char *b2, int sb2,
                          const char *bc, int sbc, const char *br, int sbr);
/* Pivot combine with empty first set (used in recursive echelise) */
extern void fPivotCombine0(const char *b2, int sb2,
                           const char *bc, int sbc, const char *br, int sbr);
extern uint64_t fColumnRiffleIdentity(const char *bs, int sbs,
              const char *rm, int srm, const char *out, int sout);
/* Column riffle zero, as required by recursive echelise */
extern void fColumnRiffleZero(const char *bs, int sbs,
                              const char *rm, int srm,
                              const char *out, int sout);
/* Add in identity, as required by recursive echelise */
extern void fAddIdentity(const char *bs, int sbs,
                         const char *rm, int srm,
                         const char *out, int sout);
/* MKR, take a pair of bitstrings, one contained in the other
 * and produce one of length the number of set bits in the larger
 * with 0s for the set bits and 1s for the unser bits
 */
extern void fMKR(const char *bs1, int sbs1, const char *bs2, int sbs2,
                 const char *rif, int srif);
/*
 * A helper function for zis to take two bitstrings and a matrix
 * The first bit string gives the rows to be ignored
 * The second is those rows to be multipled by -1
 * The non slected is to be multiplied by a remnant
 */
extern void fRowTriage(const char *zero_bs, const char *sig_bs, const char *in, const char *sel, const char *nsel);

/*
 * Helper function for zis and zsa to negate a matrix
 */
extern void fNegate(const char *in,  const char *out);

// funs2 (small, composite) functions
extern void fMultiplyAdd(const char * tmp, const char *m1, int s1,
           const char *m2, int s2, const char *m3, int s3,
           const char * m4, int s4);
extern uint64_t fNullSpace(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2);
extern void fInvert(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2);

// funs3 field-changing routines
extern void fFrobenius(const char *m1, int s1, const char *m2, int s2);

extern int  fFieldContract(const char *m1, int s1, uint64_t newfield,
                           const char *m2, int s2);
extern void fFieldExtend(const char *m1, int s1, uint64_t newfield,
                         const char *m2, int s2);

// funs4 pieces of tensor powers
extern void fTensor(const char *m1, int s1,
                 const char *m2, int s2, const char *m3, int s3);
extern void fExteriorSquare(const char *m1, int s1,
                           const char *m2, int s2);
extern void fExteriorCube(const char *m1, int s1,
                          const char *m2, int s2);
extern void fSymmetricSquare(const char *m1, int s1,
                           const char *m2, int s2);

// funs5 miscellanous small functions
extern void fAdd(const char *fn1, int s1, const char *fn2, int s2,
                 const char *fn3, int s3);
extern void fScalarMul(const char *m1, int s1,
                       const char *m2, int s2, FELT sc);
extern FELT fTrace(const char *m1, int s1);

void fProjectiveVector(const char *m1, int s1, uint64_t pvec,
                       const char *m2, int s2);
void fMulMatrixMap(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3);
void fMulMaps(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3);
void fRandomMatrix(const char *m1, int s1, uint64_t fdef,
                   uint64_t nor, uint64_t noc, uint64_t seed, uint64_t count);

/* funs6: stuff to allow recursive file based Gaussian */
/*
 * This function splits something we want Gaussian operations done on,
 * eg rank, nullspace, invert, ProduceNREF, FullEchelize
 * and splits 2x2 and recurses using files
 * The algorithm is as described in the paper
 * Parker, Linton, Nebe, Niemeyer, Thackray
 * but instead of splitting over cores and memory,
 * it chops the file into 4 roughly equal sized chunks
 * and operates on each in turn, using the tasks defined in the paper
 * to create the multiplier, cleaner, remnant and row and column selects
 * This will use a lot of temporary files
 * Given that all files concerned will already have been logged
 * none of its input will be logged, and hence the int parameters
 * in fFullEchelize aren't required here
 * The returned value is the rank
 */
extern uint64_t fRecurse_ECH(int first, /* Top level call? */
                             const char *name, /* Calling program name */
                             const char *temp, /* Base for creating tmp files */
                             const char *input, int in_mode, const char *row_sel,
                             const char *col_sel, const char *multiplier,
                             const char *cleaner, const char *remnant);

/* end of funs.h  */
