/*
 * $Id: mop.h,v 1.3 2001/10/11 23:02:06 jon Exp $
 *
 * Monster operations for meataxe
 *
 * Derived from mop.h version 1.6  --  30.11.98 by R.A.Parker and R.A.Wilson
 *
 */

#ifndef included__mop
#define included__mop

extern unsigned char vectemp[24712];
extern unsigned long ptr1, ptr2;
extern int PRINT;
extern unsigned char  vec1[24712],vec2[24712];
extern char suz1head[8],Thead[8],suz2head[8];
extern unsigned char orvec1[24712],orvec2[24712],vorvec[24712];
extern unsigned char s90head[12];
extern unsigned char s729head[12];
extern unsigned char s142head[12];
extern char w[256],ww[256],bar[256];
extern unsigned char vechead[12],mathead[12];
extern int s3[7][7];
extern char v3[4][7];
extern long suztab[32761];
extern long Tperm[87752];
extern long *T324a,*T324b,*T538;
extern long *t729,*tw729,*tww729,*vwork;
extern unsigned char Tbact[87752];


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

extern suzel A, B, C, E, suzwork;

extern void FTOV(unsigned char *a, unsigned long b, unsigned char c);

extern void FGAP(unsigned char *d, unsigned char *e, unsigned long f, unsigned long g);

extern void FUNGAP(unsigned char *d, unsigned char *e, unsigned long f, unsigned long g);

extern void init(void);

extern void rdall(void);

extern void malsuz(suzel *m);

extern void rdsuz1(suzel m, const char *fn);

extern void cpsuz(suzel a,suzel b);

extern void suzmult(suzel a, suzel b, suzel c);

extern int suzor(suzel a);

extern void vecsuz(const unsigned char *vecin, suzel m, unsigned char *vecout);

extern void vecT(unsigned char *vecin, unsigned char *vecout);

#endif
