/*
 * $Id: mop.c,v 1.1 2001/09/13 21:16:44 jon Exp $
 *
 * Monster operations for meataxe
 *
 * Derived from mop.h version 1.6  --  30.11.98 by R.A.Parker and R.A.Wilson
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mop.h"

unsigned char vectemp[24712];
long ptr1, ptr2;
int PRINT;
unsigned char vec1[24712],vec2[24712];
char suz1head[8],Thead[8],suz2head[8];
unsigned char orvec1[24712],orvec2[24712],vorvec[24712];
unsigned char s90head[12];
unsigned char s729head[12];
unsigned char s142head[12];
char w[256],ww[256],bar[256];
unsigned char vechead[12],mathead[12];
int s3[7][7];
char v3[4][7];
long suztab[32761];
long Tperm[87752];
long *T324a,*T324b,*T538;
long *t729,*tw729,*tww729,*vwork;
unsigned char Tbact[87752];

suzel A, B, C, E,suzwork;

static unsigned char FFRV(unsigned char* a, long b)
{
    unsigned char c;
    c=a[b/4];
    return (c>>(2*(3-(b%4))))&3;
}
static unsigned char FFRV2(unsigned char* a, long b)
{
    unsigned char c;
    c = a[(b>>3)];
    return ( c >> ( 7 - (b&7) ) ) & 1;
}
 
void FTOV(unsigned char* a, long b, unsigned char c)
{
    char f;
    long d,e;
    d=b/4;
    e = 3-(b%4);
    f = a[d];
    f = f|(c<<(e<<1));
    a[d]=f;
    return;
}

void FGAP(unsigned char * d, unsigned char * e,long f, long g)
{
    int h;
    for (h=0;h<f;h++)
        FTOV(e,ptr2++,FFRV(d,ptr1++));
    ptr2 += g;
}

void FUNGAP(unsigned char * d, unsigned char * e,long f, long g)
{
    int h;
    for (h=0;h<f;h++)
        FTOV(e,ptr2++,FFRV(d,ptr1++));
    ptr1 += g;
}

void init(void)
{
    unsigned char uc;
    int i,j;
    long k,l,m,n;
    strncpy(suz1head,"MONSUZ01",8);
    strncpy(suz2head,"MONSUZ02",8);
    strncpy(Thead,"MON--T01",8);
    vechead[0]=2;
    vechead[1]=0;
    vechead[2]=0;
    vechead[3]=0;
    vechead[4]=1;
    vechead[5]=0;
    vechead[6]=0;
    vechead[7]=0;
    vechead[8]=18;
    vechead[9]=1;
    vechead[10]=3;
    vechead[11]=0;
/*  GF2,  1 row,  196882 (=3 1 18 base 256) columns */
    mathead[0]=2;
    mathead[1]=0;
    mathead[2]=0;
    mathead[3]=0;
    mathead[4]=18;
    mathead[5]=1;
    mathead[6]=3;
    mathead[7]=0;
    mathead[8]=18;
    mathead[9]=1;
    mathead[10]=3;
    mathead[11]=0;
/*  GF2,  196882 rows,  196882 (=3 1 18 base 256) columns */

    s90head[0]=4;
    s90head[1]=0;
    s90head[2]=0;
    s90head[3]=0;
    s90head[4]=90;
    s90head[5]=0;
    s90head[6]=0;
    s90head[7]=0;
    s90head[8]=90;
    s90head[9]=0;
    s90head[10]=0;
    s90head[11]=0;

    s729head[0]=4;
    s729head[1]=0;
    s729head[2]=0;
    s729head[3]=0;
    s729head[4]=217;
    s729head[5]=2;
    s729head[6]=0;
    s729head[7]=0;
    s729head[8]=217;
    s729head[9]=2;
    s729head[10]=0;
    s729head[11]=0;

    s142head[0]=2;
    s142head[1]=0;
    s142head[2]=0;
    s142head[3]=0;
    s142head[4]=142;
    s142head[5]=0;
    s142head[6]=0;
    s142head[7]=0;
    s142head[8]=142;
    s142head[9]=0;
    s142head[10]=0;
    s142head[11]=0;
/*
    l729=(((729-1)/4)/sizeof(long))+1;
    l90=(((90-1)/4)/sizeof(long))+1;
    l142=(((142-1)/8)/sizeof(long))+1;
    l324=(((324-1)/8)/sizeof(long))+1;
    l538=(((538-1)/8)/sizeof(long))+1;
*/
    for(i=1;i<7;i++)
    {
        s3[i][1]=i;
        s3[1][i]=i;
        s3[i][i]=1;
    }
    s3[2][2]=3;
    s3[3][3]=2;
    s3[2][3]=1;
    s3[3][2]=1;
    s3[2][4]=5;
    s3[2][5]=6;
    s3[2][6]=4;
    s3[3][4]=6;
    s3[3][5]=4;
    s3[3][6]=5;
    s3[4][2]=6;
    s3[5][2]=4;
    s3[6][2]=5;
    s3[4][3]=5;
    s3[5][3]=6;
    s3[6][3]=4;
    s3[4][5]=3;
    s3[5][4]=2;
    s3[4][6]=2;
    s3[6][4]=3;
    s3[5][6]=3;
    s3[6][5]=2;
    for (i=1;i<7;i++) v3[0][i]=0;
    for (i=1;i<4;i++) 
    {
         for (j=1;j<4;j++) 
        {
            v3[i][j] = (3+i-j)%3 +1;
            v3[i][j+3] = (4+j-i)%3 + 1;
        }
    }
    T324a = malloc(324*l324*sizeof(long));
    T324b = malloc(324*l324*sizeof(long));
    T538 = malloc(538*l538*sizeof(long));
    j=0xaa;
    for (i=0;i<256;i++) bar[i] = ((i&j)>>1)^i;
    for (i=0;i<256;i++)  w[i]=(i&j)^((i&j)>>1)^((i<<1)&j);
    for (i=0;i<256;i++) 
    {
        uc=(unsigned char)w[i];
        ww[i]=w[uc];
    }
    t729 = malloc(90*l729*sizeof(long));
    tw729 = malloc(90*l729*sizeof(long));
    tww729 = malloc(90*l729*sizeof(long));
    vwork = malloc(l729*sizeof(long)); 

    k=1;
    l=65880;
    for (n=0;n<21870;n++) suztab[k++]=l++;
    l+=2;
    for (m=0;m<66;m++)
    {
        for (n=0;n<162;n++) suztab[k++]=l++;
        l+=2;
    }
    for (n=0;n<198;n++) suztab[k++]=l++;
}



static void rdvec(const char* filnam, unsigned char* vecin)
{
    FILE * f ;
    int i;
    f = fopen(filnam,"rb");
     if (f == NULL) 
    {
        printf("File %s does not exist!\n",filnam);
        exit(-1);
    }
    fread(vectemp,1,12,f);
    fread(vectemp,1,24611,f);
    for (i=0;i<24712;i++) vecin[i]=0;
    ptr1=0;
    ptr2=0;
    for (i=0;i<90;i++)     FGAP(vectemp,vecin,729,3);
    FGAP(vectemp,vecin,21870,2);
    for (i=0;i<66;i++)     FGAP(vectemp,vecin,162,2);
    FGAP(vectemp,vecin,198,2);
    FGAP(vectemp,vecin,71,1);
    fclose(f);
}


/*
static void wtvec(char* filnam, char* vecout)
{
    FILE * f ;
    int i;
    f = fopen(filnam,"wb");
     if (f == NULL) 
    {
        printf("File %s cannot be created!\n",filnam);
        exit(-1);
    }
    fwrite(vechead,1,12,f);
    for (i=0;i<24611;i++) vectemp[i]=0;
    ptr1=0;
    ptr2=0;
    for (i=0;i<90;i++)     FUNGAP(vecout,vectemp,729,3);
    FUNGAP(vecout,vectemp,21870,2);
    for (i=0;i<66;i++)     FUNGAP(vecout,vectemp,162,2);
    FUNGAP(vecout,vectemp,198,2);
    FUNGAP(vecout,vectemp,71,1);
    fwrite(vectemp,1,24611,f);
    fclose(f);
}
*/

static int veccomp(unsigned char *veca, unsigned char * vecb)
{
  return memcmp(veca, vecb, 24712);
}
static void cpvec(unsigned char *veca, unsigned char *vecb)
{
  (void)memcpy(vecb, veca, 24712);
}

/*
static int addvec(char *veca, char *vecb, char *vecc)
{
    int i;
    char *ptc1,*ptc2,*ptc3;
    ptc1=veca;
    ptc2=vecb;
    ptc3=vecc;
    for (i=0;i<24712;i++) *(ptc3++) = *(ptc1++)^*(ptc2++);
}
*/

void malsuz(suzel  * m)
{
    suzel t;
    *m=(suzel)malloc(sizeof(suzex));
    t=*m;
    t->m729 = malloc(729*l729*sizeof(long));
    t->w729 = malloc(729*l729*sizeof(long));
    t->ww729 = malloc(729*l729*sizeof(long));
    t->m90 = malloc(90*l90*sizeof(long));
    t->w90 = malloc(90*l90*sizeof(long));
    t->ww90 = malloc(90*l90*sizeof(long));
    t->m142 = malloc(142*l142*sizeof(long));
    t->p32760 = malloc(32761*sizeof(long));
    t->b32760 = malloc(32761);
}

void rdsuz1(suzel m, const char * fn)
{
  FILE * f ;
  char * ptr;
  long * ptrl;
  unsigned char * ptrc;
  long i;
  long j;
  unsigned char c[3];
  m->greased =0;
  m->inout=0;
  f= fopen(fn,"rb");
  if (f == NULL) {
    printf("File %s does not exist!\n",fn);
    exit(-1);
  }
  fread(vectemp,1,8,f);  
  if (0 == memcmp(vectemp,suz1head,8)) m->inout = 1;
  if (0 == memcmp(vectemp,suz2head,8)) m->inout = 2;
  if (m->inout ==0) {
    printf("File %s is not a Monster-Suzuki element\n",fn);
    exit(-1);
  }
  ptrl = m->m729;
  ptr = (char *) ptrl;
  for (i=0;i<729;i++) {
    fread(ptr,1,183,f);
    ptr+=l729*sizeof(long);
  }
  ptr = (char *) m->m90;
  for (i=0;i<90;i++) {
    fread(ptr,1,23,f);
    ptr+=l90*sizeof(long);
  }
  ptr = (char *) m->m142;
  for (i=0;i<142;i++) {
    fread(ptr,1,18,f);
    ptr+=l142*sizeof(long);
  }

  ptrc=(m->b32760)+1;
  ptrl=(m->p32760)+1;
  for (i=1;i<=32760;i++) {
    fread(c,1,3,f);
    *(ptrc++)=c[2];
    j=256*c[1]+c[0];
    *(ptrl++)=j;
  }
  fclose(f);
}

static void rdT(const char *fn)
{
  int i;
  unsigned char c[4];
  FILE *f;
  char *ptr;
  f=fopen(fn,"rb");
  if (f==NULL) {
    printf("File %s does not exist\n",fn);
    exit(-1);
  }
  fread(vectemp,1,8,f);
  ptr = (char *)T324a;
  for (i=0;i<324;i++) {
    fread(ptr,1,41,f);
    ptr+=l324*sizeof(long);
  }
  ptr=(char *)T324b;
  for (i=0;i<324;i++) {
    fread(ptr,1,41,f);
    ptr+=l324*sizeof(long);
  }
  ptr=(char *)T538;
  for (i=0;i<538;i++) {
    fread(ptr,1,68,f);
    ptr+=l538*sizeof(long);
  }
  for (i=0;i<87750;i++) {
    fread(c,1,4,f);
    Tperm[i]=c[0]+256*c[1]+256*256*c[2];
    Tbact[i] = c[3];
    if (c[3]==0) Tperm[i]=-1;
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
  rdsuz1(A,"A.m");
  rdsuz1(B,"B.m");
  rdsuz1(C,"C.m");
  rdsuz1(E,"E.m");
  rdT("T.m");
  rdvec("order.vec1",orvec1);
  rdvec("order.vec2",orvec2);
  rdvec("vorder.vec",vorvec);
}

static int grease(suzel m)
{
    unsigned long i,j;
    unsigned char uc;
    char *ptc1,*ptc2,*ptc3;
    long *ptl1,*ptl2,*ptl3;
    if (m->greased ==1) return(1);
    ptl1=m->m729;
    ptl2=m->w729;
    ptl3=m->ww729;
    for (i=0;i<729;i++)
    {
        ptc1=(char *)ptl1;
        ptc2=(char *)ptl2;
        ptc3=(char *)ptl3;
        for (j=0;j<l729;j++) 
        {
            *(ptl2++)=0;
            *(ptl3++)=0;
        }
        for (j=0;j<183;j++) 
        {
            uc=(unsigned char)*(ptc1++);
            *(ptc2++)=w[uc];
            *(ptc3++)=ww[uc];
        }
        ptl1+=l729;
    }
    ptl1=m->m90;
    ptl2=m->w90;
    ptl3=m->ww90;
    for (i=0;i<90;i++)
    {
        ptc1=(char *)ptl1;
        ptc2=(char *)ptl2;
        ptc3=(char *)ptl3;
        for (j=0;j<l90;j++) 
        {
            *(ptl2++)=0;
            *(ptl3++)=0;
        }
        for (j=0;j<23;j++) 
        {
            uc=(unsigned char)*(ptc1++);
            *(ptc2++)=w[uc];
            *(ptc3++)=ww[uc];
        }
        ptl1+=l90;
    }
    m->greased = 1;
    return(0);
}

void vecsuz(unsigned char * vecin, suzel m, unsigned char * vecout)
{
  unsigned char uc;
  unsigned long i,j,k,l;
  unsigned char entry, bact;
  unsigned char *ptc1,*ptc2,*ptc3;
  long *ptl1,*ptl2,*ptl3,*ptl2w,*ptl2ww,*ptl4;
  if (m->greased == 0) grease(m);
  for (i=0;i<24712;i++) vecout[i]=0;
  j=0;
  ptl1=t729;
  for (i=0;i<90;i++) {
    ptl2=ptl1;
    for (k=0;k<l729;k++) *(ptl2++)=0;
    ptl2=m->m729;
    ptl2w=m->w729;
    ptl2ww=m->ww729;
    for (k=0;k<729;k++) {
      entry=FFRV(vecin,j);
      if (entry!=0) {
	ptl3=ptl2;
	if (entry==2) ptl3=ptl2w;
	if (entry == 3) ptl3=ptl2ww;
	ptl4=ptl1;
	for (l=0;l<l729;l++) *(ptl4++)^=*(ptl3++);
      }
      ptl2+=l729;
      ptl2w+=l729;
      ptl2ww+=l729;
      j++;
    }
    ptl1+=l729;
    j+=3;
  }
  ptl1=t729;
  ptl2=tw729;
  ptl3=tww729;
  for (i=0;i<90;i++) {
    ptc1=(unsigned char *)ptl1;
    ptc2=(unsigned char *)ptl2;
    ptc3=(unsigned char *)ptl3;
    for (j=0;j<l729;j++) {
      *(ptl2++)=0;
      *(ptl3++)=0;
    }
    for (j=0;j<183;j++) {
      uc=(unsigned char)*(ptc1++);
      *(ptc2++)=w[uc];
      *(ptc3++)=ww[uc];
    }
    ptl1+=l729;
  }
  ptc1=vecout;
  ptl1=m->m90;
  for (i=0;i<90;i++) {
    ptl2=vwork;
    for (k=0;k<l729;k++) *(ptl2++)=0;
    ptl2=t729;
    ptl2w=tw729;
    ptl2ww=tww729;
    for (k=0;k<90;k++) {
      entry=FFRV((unsigned char *)ptl1,k);
      if (entry!=0) {
	ptl3=ptl2;
	if (entry==2) ptl3=ptl2w;
	if (entry == 3) ptl3=ptl2ww;
	ptl4=vwork;
	for (l=0;l<l729;l++) *(ptl4++)^=*(ptl3++);
      }
      ptl2+=l729;
      ptl2w+=l729;
      ptl2ww+=l729;
    }
    ptl1+=l90;
    ptc2=(unsigned char *) vwork;
    if (m->inout ==1) {
      for (j=0;j<183;j++) *(ptc1++) = *(ptc2++);
    }
    if (m->inout == 2) {
      for (j=0;j<183;j++) {
	uc = (unsigned char) *(ptc2++);
	*(ptc1++)=bar[uc];
      }
    }
  }
  ptl1 = (m->p32760) +1;
  ptc2 = (m->b32760) + 1;
  for (j=1;j<=32760;j++) {
    k = suztab[*(ptl1++)];
    i=suztab[j];
    bact = *(ptc2++);
    entry = FFRV(vecin,i);
    if (entry!=0) FTOV(vecout,k,v3[entry][bact]);
  }  
  j=197552;
  ptl4=vwork;
  for (k=0;k<l142;k++) *(ptl4++)=0;
  ptl3 = m->m142;
  for (i=0;i<142;i++) {
    entry = FFRV2(vecin,j++);
    if (entry ==0) ptl3+=l142;
    if (entry ==1) {
      ptl4 = vwork;
      for (k=0;k<l142;k++) *(ptl4++)^=*(ptl3++);
    }
  }  
  ptc1+=8224;
  ptc2=(unsigned char *) vwork;
  for (k=0;k<18;k++) *(ptc1++) = *(ptc2++);
}


void vecT(unsigned char *vecin, unsigned char *vecout)
{
  unsigned long i,j,k,l;
  unsigned char entry;
  unsigned char *ptc1,*ptc2;
  long *ptl3,*ptl4;

  for (i=0;i<24712;i++) vecout[i]=0; 
  for (j=0;j<87750;j++) {
    long k = Tperm[j];
    if (k==-1) continue;
    entry = FFRV(vecin,j);
    if (entry!=0) FTOV(vecout,k,v3[entry][Tbact[j]]);
  }  

  j=175504;

  ptc1=vecout+21938;
  for (l=0;l<11;l++) {
    ptl4=vwork;
    for (k=0;k<l324;k++) *(ptl4++)=0;
    ptl3 = T324a;
    for (i=0;i<324;i++) {
      entry = FFRV2(vecin,j++);
      if (entry ==0) {
	ptl3+=l324;
      } else {
	ptl4 = vwork;
	for (k=0;k<l324;k++) *(ptl4++)^=*(ptl3++);
      }
    }  
    ptc2=(unsigned char *) vwork;
    for (k=0;k<41;k++) *(ptc1++) = *(ptc2++);
    j+=4;
  }
  for (l=0;l<55;l++) {
    ptl4=vwork;
    for (k=0;k<l324;k++) *(ptl4++)=0;
    ptl3 = T324b;
    for (i=0;i<324;i++) {
      entry = FFRV2(vecin,j++);
      if (entry ==0) {
	ptl3+=l324;
      } else {
	ptl4 = vwork;
	for (k=0;k<l324;k++) *(ptl4++)^=*(ptl3++);
      }
    }  

    ptc2=(unsigned char *) vwork;
    for (k=0;k<41;k++) *(ptc1++) = *(ptc2++);
    j+=4;
  }
  ptl4=vwork;
  for (k=0;k<l538;k++) *(ptl4++)=0;
  ptl3 = T538;
  for (i=0;i<396;i++) {
    entry = FFRV2(vecin,j++);
    if (entry ==0) {
      ptl3+=l538;
    } else {
      ptl4 = vwork;
      for (k=0;k<l538;k++) *(ptl4++)^=*(ptl3++);
    }
  }  
  j+=4;
  for (i=0;i<142;i++) {
    entry = FFRV2(vecin,j++);
    if (entry ==0) {
      ptl3+=l538;
    } else {
      ptl4 = vwork;
      for (k=0;k<l538;k++) *(ptl4++)^=*(ptl3++);
    }
  }  
  ptc2=(unsigned char *) vwork;
  for (k=0;k<68;k++) *(ptc1++) = *(ptc2++);
}

void cpsuz(suzel a,suzel b)
{
    long *ptl1, *ptl2;
    unsigned char *ptc1, *ptc2;
    unsigned int i;
    b->greased=a->greased;
    b->inout=a->inout;
    ptl1=a->m729;
    ptl2=b->m729;
    for (i=0;i<729*l729;i++) *(ptl2++)=*(ptl1++);
    ptl1=a->m90;
    ptl2=b->m90;
    for (i=0;i<90*l90;i++) *(ptl2++)=*(ptl1++);

    ptl1=a->m142;
    ptl2=b->m142;
    for (i=0;i<142*l142;i++) *(ptl2++)=*(ptl1++);
    ptl1=(a->p32760)+1;
    ptl2=(b->p32760)+1;
    for (i=1;i<=32760;i++) *(ptl2++)=*(ptl1++);
    ptc1=(a->b32760)+1;
    ptc2=(b->b32760)+1;
    for (i=1;i<=32760;i++) *(ptc2++)=*(ptc1++);

    if (a->greased ==1)
    {
        ptl1=a->w729;
        ptl2=b->w729;
        for (i=0;i<729*l729;i++) *(ptl2++)=*(ptl1++);
        ptl1=a->w90;
        ptl2=b->w90;
        for (i=0;i<90*l90;i++) *(ptl2++)=*(ptl1++);
        ptl1=a->ww729;
        ptl2=b->ww729;
        for (i=0;i<729*l729;i++) *(ptl2++)=*(ptl1++);
        ptl1=a->ww90;
        ptl2=b->ww90;
        for (i=0;i<90*l90;i++) *(ptl2++)=*(ptl1++);
    }
}


void suzmult(suzel a, suzel b, suzel c)
{
    unsigned long i,j,k,l;
    char unsigned entry;
    unsigned char uc;
    unsigned char *ptc1,*ptc2,*ptc3;
    long *ptl1, *ptl2, *ptl2a, *ptl2w, *ptl2ww, *ptl3, *ptl3a;
    cpsuz(b,suzwork);

    if (a->greased==0) grease(a);
    if (a->inout ==2)
    {
        ptl1= suzwork->m90;
        for (i=0;i<90;i++)
        {
            ptc1 = (unsigned char *)ptl1;
            for (j=0;j<23;j++)
            {
                uc = (unsigned char) *(ptc1);
                *(ptc1++) = bar[uc];
            }
            ptl1+=l90;
        }
        ptl1= suzwork->m729;
        for (i=0;i<729;i++)
        {
            ptc1 = (unsigned char *)ptl1;
            for (j=0;j<183;j++)
            {
                uc = (unsigned char) *(ptc1);
                *(ptc1++) = bar[uc];
            }
            ptl1+=l729;
        }
        suzwork->greased =0;
    }
    if (suzwork->greased ==0) grease (suzwork);
    c->greased = 0;
    c->inout = (((a->inout) + (suzwork->inout))%2) + 1;
    ptl1=a->m729;
    ptl3=c->m729;
    for (i=0;i<729;i++)
    {
        ptl3a=ptl3;
        for (k=0;k<l729;k++) *(ptl3a++)=0;
        ptl2 = suzwork->m729;
        ptl2w = suzwork->w729;
        ptl2ww = suzwork->ww729;
        for (k=0;k<729;k++)
        {
            entry = FFRV( (unsigned char *)ptl1,k);
            if (entry != 0)
            {
                ptl2a=ptl2;
                if (entry==2) ptl2a=ptl2w;
                if (entry==3) ptl2a=ptl2ww;
                ptl3a=ptl3;
                for (l=0;l<l729;l++) *(ptl3a++)^=*(ptl2a++);
            }
            ptl2+=l729;
            ptl2w+=l729;
            ptl2ww+=l729;
        }
        ptl1+=l729;
        ptl3+=l729;
    }
    ptl1=suzwork->m90;
    ptl3=c->m90;
    for (i=0;i<90;i++)
    {
        ptl3a=ptl3;
        for (k=0;k<l90;k++) *(ptl3a++)=0;
        ptl2 = a->m90;
        ptl2w = a->w90;
        ptl2ww = a->ww90;
        for (k=0;k<90;k++)
        {
            entry = FFRV( (unsigned char *)ptl1,k);
            if (entry != 0)
            {
                ptl2a=ptl2;
                if (entry==2) ptl2a=ptl2w;
                if (entry==3) ptl2a=ptl2ww;
                ptl3a=ptl3;
                for (l=0;l<l90;l++) *(ptl3a++)^=*(ptl2a++);
            }
            ptl2+=l90;
            ptl2w+=l90;
            ptl2ww+=l90;
        }
        ptl1+=l90;
        ptl3+=l90;
    }
    ptl1 = (a->p32760)+1;
    ptl2 = (suzwork->p32760);
    ptl3 = (c->p32760)+1;
    ptc1 = (a->b32760)+1;
    ptc2 = (suzwork->b32760);
    ptc3 = (c->b32760)+1;
    for (j=1;j<=32760;j++)
    {
        k = *(ptl1++);
        *(ptl3++) = *(ptl2+k);
        *(ptc3++) = s3[*(ptc1++)][*(ptc2+k)];
    }
    ptl1 = a->m142;
    ptl3=c->m142;
    for (i=0;i<142;i++)
    {
        ptl3a=ptl3;
        for (k=0;k<l142;k++) *(ptl3a++)=0;
        ptl2=suzwork->m142;
        for (k=0;k<142;k++)
        {
            entry = FFRV2((unsigned char *)ptl1,k);
            if (entry!=0)
            {
                ptl2a=ptl2;
                ptl3a=ptl3;
                for(l=0;l<l142;l++) *(ptl3a++)^=*(ptl2a++);
            }
            ptl2+=l142;
        }
        ptl1+=l142;
        ptl3+=l142;
    }
}

int suzor(suzel a)
{
    int i,j;
    cpvec(vorvec,vec1);
    for (i=1;i<=119;i++)
    {
        vecsuz(vec1,a,vec2);
        cpvec(vec2,vec1);
        j=veccomp(vec1,vorvec);
        if(j==0)
        {
            if (PRINT==1) printf("Order is %d\n",i);
            return(i);
        }
    }
    return(120);
}
