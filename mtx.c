/*
 * $Id: mtx.c,v 1.1 2001/09/13 21:16:44 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtx.h"

typedef unsigned long ELT;

typedef unsigned char * VPTR;

typedef struct
{
    unsigned long  rdef;     /*  coded definition of which ring        */
    unsigned long order;              /*  order,  or 0 if not finite field      */
    unsigned long charc;              /*  characteristic                        */
    int vltype;              /*  for switch statement in VMAD,VADD,VMUL*/
    int etyp;                /*  for switch statement in ELT operations*/
    unsigned char * radd;    /*  addition table for some types         */
    unsigned char * rmul;    /*  scalar multiplication table for some  */
    unsigned char * rinv;    /*  inverse table for elements            */
    unsigned long eperb;              /*  elements per byte (or 1)              */
    unsigned long bpere;              /*  bytes per element (or 1)              */
    unsigned long eper4;              /*  elements per 4-byte quantity          */
    unsigned char * efrvt;   /*  extraction table for EFRV             */
    ELT ZERO;
    ELT ONE;
    ELT MIN1;
    ELT GEN;
}                 RINGSTR;

typedef RINGSTR * RING;

typedef struct
{
    unsigned long dim;    /* number of dimensions  */
    unsigned long nox;             /* number of bytes read and written for 1 row */
    unsigned long nob;             /* number of bytes in memory for 1 row */
    unsigned long nol;             /* number of longs in memory for 1 row */
    RING r;
}                 SPACE;

typedef struct
{
    unsigned char * mem;
    char progname[20];
}                 ENV;

static ENV env;
static int initialised = 0;
static int got_open_files = 0;
static FILE **outputs;
static RING r;
static SPACE *spaces;
static VPTR *vectors;
unsigned long T1[32],T2[32],T3[32];

static void EEXIT(const char * a)
{
  printf("error %s in program %s\n",a,env.progname);
  exit(15);
}

static void * mymalloc(unsigned long siz)
{
  void * a;
  a = (void *)malloc(siz);
  if (a == NULL) {
    fprintf(stderr, "mymalloc fails to obtain %ld bytes\n", siz);
    EEXIT("mymalloc failed");
  }
  return a;
}

static void PCONS(const char * progname)
{
  strcpy(env.progname,progname);
  env.mem = (unsigned char *) mymalloc(MBR*1000);
}

static unsigned int digits(unsigned long a)
{
  if (a < 10) {
    return 1;
  } else {
    return 1 + digits(a / 10);
  }
}

static void putlong(FILE *f,  unsigned long u)
{
  unsigned char byt;
  int i;
  for(i=0;i<4;i++) {
    byt = (u>>24) & 255;
    fwrite(&byt,1,1,f);
    u = (u << 8);
  }
}

static FILE * WRHDR(const char * n,  unsigned long rdef,  unsigned long nor,  unsigned long noc)
{
  FILE *f;
  f = fopen(n,"wb");
  if(f == NULL)
    EEXIT(n);
  putlong(f,rdef);
  putlong(f,nor);
  putlong(f,noc);
  return f;
}

static void WRROW(FILE * f, SPACE s, VPTR v)
{
  fwrite(v,s.nox,1,f);
}
 
static void CLOSE(FILE * f)
{
  fclose(f);
}

static unsigned long smlpr( unsigned long a )
{
  unsigned long b,c;
  if(a%2 == 0) return 2;
  c = a;
  for(b=3;b*b<=c;b+=2)
    if(a%b == 0) return b;
  return a;
}

static void TZER(unsigned long *a , unsigned long b)
{
  unsigned long i;
  for(i=0;i<b;i++)
    *(a++) = 0;
}

static void EADD(RING r,  ELT a , ELT b, ELT * c)
{
  if(r->etyp == 1)
    *c = (a+b)%r->order;
}

static unsigned long TPACK(unsigned long *a , unsigned long b , unsigned long p)
{
  unsigned long c,i;
  c=0;
  for(i=0;i<b;i++)
    c=c*p + *(a++);
  return c;
}

static unsigned long TINC(unsigned long *a , unsigned long b , unsigned long p)
{
  a += (b-1);
  while(1) {
    (*a)++;
    if(*a != p) return 0;
    *(a--) = 0;
    b--;
    if(b==0) return 1;
  }
}

static void EMUL(RING r,  ELT a , ELT b, ELT * c)
{
  if(r->etyp == 1)
    *c = (a*b)%r->order;
}

static RING RPOINT(unsigned long rdef)
{
    unsigned long i,j,k;
    unsigned long t1;
    unsigned char *a1,*a2;
    RING r;
    r = (RING) mymalloc(sizeof(RINGSTR));
/*  calculate eperb and bpere to get started   */
    r->bpere=1;
    r->eperb=1;
    if(rdef>256)
    {
        if(rdef>65536) 
            r->bpere = 4;
        else
            r->bpere = 2;
    }
    if(rdef<=16)
    {
       switch (rdef)
       {
         case 2:
           r->eperb=8;
           break;
         case 3:
           r->eperb=5;
           break;
         case 4:
           r->eperb=4;
           break;
         case 5:
           r->eperb=3;
           break;
         default:
           r->eperb=2;
        }
    }
    r->eper4 = 4*(r->eperb)/(r->bpere);
    r->rdef = rdef;
    r->order=rdef;
    i=smlpr(rdef);
    if( (rdef != i) || (rdef > 256) || (rdef < 2) )
        EEXIT("only works for primes < 256 so far");
    r->etyp=1;
    r->charc = i;
    r->vltype=1;
    if(r->charc == 2)
        r->vltype=2;
    k=r->eperb;
/*  if char != 2   make the radd array    */
    if(r->charc != 2)
    {
        a1 = (unsigned char *) mymalloc(65536);
        r->radd = a1;
        TZER(T1,k);
        while(1)
        {
            TZER(T2,k);
            a2=a1;
            while(1)
            {
                for(i=0;i<k;i++)
                    EADD(r,T1[i],T2[i],&T3[i]);
                *(a2++) = (unsigned char) TPACK(T3,k,rdef);
                i=TINC(T2,k,rdef);
                if(i==1) break;
            }
            a1 += 256;
            i=TINC(T1,k,rdef);
            if(i == 1) break;
        }
    }

/*  if rdef > 2  make the rmul array    */
    if(rdef > 2)
    {
        a1 = (unsigned char *) mymalloc(256 * (rdef-2));
        r->rmul = a1;
        for(t1=2;t1<rdef;t1++)
        {
            TZER(T2,k);
            a2=a1;
            while(1)
            {
                for(i=0;i<k;i++)
                    EMUL(r,t1 , T2[i],&T3[i]);
                *(a2++) = (unsigned char) TPACK(T3,k,rdef);
                i = TINC(T2,k,rdef);
                if(i == 1) break;
            }
            a1 += 256;
        }
    }
/*    make the inverse table    */
    a1 = (unsigned char *) mymalloc(r->order);
    r->rinv = a1;
    for(i=0;i<r->order;i++)
    {
        for(j=0;j<r->order;j++)
        {
            EMUL(r,i,j,&k);
            if(k == 1) (r->rinv)[i]=j;
        }
    }
/*     make the extraction table if field <= 16     */
    if(r->eperb>1)
    {
        a1=(unsigned char *) mymalloc(256*r->eperb);
        r->efrvt = a1;
        for(i=0;i<r->eperb;i++)
        {
            TZER(T1,r->eperb);
            for(j=0;j<256;j++)
            {
                *(a1++) = (unsigned char) T1[i];
                TINC(T1,r->eperb,rdef);
            }
        }
    }
    r->ONE=1;
    r->ZERO=0;
    r->MIN1=r->charc - 1;
    r->GEN=2;
    return r;
}

static unsigned long SNOR(SPACE s, unsigned long a)
{
  unsigned long b;
  b = (MBR * a) / s.nob;
  return b;
}

static void SSET(SPACE *s_ptr , RING r , unsigned long dim)
{
  unsigned long i;
  s_ptr->dim=dim;
  i=( (dim+r->eper4 - 1)/r->eper4) * 4;
  s_ptr->nox=i;
  if(i%sizeof(unsigned long)!=0)
    i=i-(i%sizeof(unsigned long))+sizeof(unsigned long);
  s_ptr->nob=i;
  s_ptr->nol=i/sizeof(unsigned long);
  s_ptr->r = r;
}

static VPTR VINC (SPACE s , VPTR v)
{
  return v+s.nob;
}

static void SCONS(SPACE * s_ptr)
{
  (void)s_ptr;
  return;
}
 
static VPTR VPOINT(unsigned long a)
{
  VPTR b;
  b = (VPTR) ( env.mem + (MBR * a) );
  return b;
}

void put_row(unsigned int row_num, unsigned int total_rows, unsigned int split_size, unsigned char *bits)
{
  unsigned int cols = (total_rows + split_size - 1) / split_size;
  unsigned int rows = cols;
  unsigned int i, j;
  if (!initialised) {
    FILE *output = fopen("map", "wb");
    unsigned long maxr;
    initialised = 1;
    PCONS("monster mtx");
    got_open_files = 0;
    r = RPOINT(2);
    if (output == NULL) {
      EEXIT("Cannot open map file");
    }
    spaces = mymalloc(cols * sizeof(SPACE));
    vectors = mymalloc(cols * sizeof(VPTR));
    vectors[0] = VPOINT(0);
    for (j = 0; j < cols; j++) {
      SCONS(spaces + j);
      SSET(spaces + j , r, (j < cols-1) ? split_size : total_rows - j * split_size);
    }
    /******  check that at least (col_pieces + 1) small rows fit in half store */
    maxr = SNOR(spaces[0], 1000/(cols + 1));
    if(maxr < 1 ) {
      EEXIT("not enough memory for output row");
    }
    for (j = 1; j < cols; j++) {
      vectors[j] = VINC(spaces[j-1], vectors[j-1]);
    }
    fprintf(output, "%6u%6u\n", rows, cols);
    for (i = 0; i < rows; i++) {
      for (j = 0; j < cols; j++) {
	fprintf(output, "%u_%u ", i, j);
      }
      fprintf(output, "\n");
    }
    fclose(output);
  }
  if (row_num % split_size == 0) {
    /* Open relevant files */
    unsigned int row = row_num / split_size;
    unsigned int nor = (row_num + split_size <= total_rows) ? split_size : total_rows - row * split_size;
    outputs = mymalloc(cols * sizeof(FILE *));
    for(i = 0; i < cols; i++) {
      unsigned int noc = ((i+1) * split_size <= total_rows) ? split_size : total_rows - i * split_size;
      char foo[20];
      sprintf(foo, "%u_%u", row, i);
      outputs[i] = WRHDR(foo, 2, nor, noc);
    }
  }
  /* Now output the row */
  for (i = 0; i < cols; i++) {
#if 0
    unsigned int cols_in = i * split_size;
    unsigned int piece_size = (i == cols-1) ? total_rows - cols_in : split_size;
    for (j = 0; j < piece_size; j++) {
      ELT e;
      EFRV(r,bits,cols_in+j,&e);
      ETOV(r,vectors[i],j,e);
    }
    WRROW(outputs[i], spaces[i], vectors[i]);
#else
    VPTR v = (VPTR)(bits + (i * split_size) / 8);
    WRROW(outputs[i], spaces[i], v);
#endif
  }    
  /* Now close files if last row for this set */
  if (row_num + 1 == total_rows || (row_num + 1) % split_size == 0) {
    unsigned int i;
    for (i = 0; i < cols; i++) {
      CLOSE(outputs[i]);
    }
  }
}
