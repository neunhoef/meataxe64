/*
 * $Id: mop.h,v 1.7 2002/01/06 16:35:48 jon Exp $
 *
 * Monster operations for meataxe
 *
 * Derived from mop.h version 1.6  --  30.11.98 by R.A.Parker and R.A.Wilson
 *
 */

#ifndef included__mop
#define included__mop

#define VECLEN 24712
extern unsigned char vectemp[VECLEN];
extern unsigned long ptr1, ptr2;
extern int PRINT;
extern unsigned char vec1[VECLEN], vec2[VECLEN];

#define l729 ((((729-1)/4)/sizeof(long))+1)
#define l90 ((((90-1)/4)/sizeof(long))+1)
#define l142 ((((142-1)/4)/sizeof(long))+1)
#define l324 ((((324-1)/4)/sizeof(long))+1)
#define l538 ((((538-1)/4)/sizeof(long))+1)

typedef struct suzy {
    long * m729;
    long * w729;
    long * ww729;
    long greased;
    long inout;
    long * m90;
    long * w90;
    long * ww90;
    long * p32760;
    unsigned char * b32760;
    long * m142;
} suzex;

typedef struct suzy *suzel;

extern suzel A, B;

extern void FTOV(unsigned char *a, unsigned int b, unsigned char c);

extern void FGAP(const unsigned char *d, unsigned char *e, unsigned int f, unsigned int g);

extern void FUNGAP(const unsigned char *d, unsigned char *e, unsigned int f, unsigned int g);

extern void init(void);

extern void rdall(void);

extern void malsuz(suzel *m);

extern void rdsuz1(suzel m, const char *fn);

extern void cpsuz(suzel a,suzel b);

extern void suzmult(suzel a, suzel b, suzel c);

extern unsigned int suzor(suzel a);

extern void vecsuz(const unsigned char *vecin, suzel m, unsigned char *vecout);

extern void vecT(unsigned char *vecin, unsigned char *vecout);

#endif
