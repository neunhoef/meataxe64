/*
 * $Id: mop.h,v 1.8 2005/06/22 21:52:53 jon Exp $
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
extern unsigned int ptr1, ptr2;
extern int PRINT;
extern unsigned char vec1[VECLEN], vec2[VECLEN];

#define wl729 ((((729-1)/4)/sizeof(word))+1)
#define wl90 ((((90-1)/4)/sizeof(word))+1)
#define wl142 ((((142-1)/4)/sizeof(word))+1)
#define wl324 ((((324-1)/4)/sizeof(word))+1)
#define wl538 ((((538-1)/4)/sizeof(word))+1)

typedef struct suzy {
    word *m729;
    word *w729;
    word *ww729;
    unsigned int greased;
    unsigned int inout;
    word *m90;
    word *w90;
    word *ww90;
    unsigned int *p32760;
    unsigned char *b32760;
    word *m142;
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
