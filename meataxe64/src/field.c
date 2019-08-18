// Copyright (C) Richard Parker Jon Thackray  2017
// Meataxe64 Nikolaus version
// field.c operations in finite fields

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"
#include "linf.h"
#include "tuning.h"


// size of uint16_t table for extracing small characteristic from
// large fields in PExtract, where both Barrett and lookup needed.
#define EXTIVCNT 1000

//  Malloc that aligns to the page - 4096

uint8_t * AlignMalloc(size_t len)
{
    uint64_t *x,y;
    x=malloc(len+4160);
    if(x==NULL) return NULL;
    y=1;
    x++;
    while( (((long int) x)&0xfff)!=0) 
    {
        x++;
        y++;
    }
    *(x-1)=y;
    return (uint8_t *) x;
}

//  And the corresponding free

void AlignFree(uint8_t * z)
{
    uint64_t y,*x;
    x=(uint64_t *) z;
    y=(uint64_t) *(x-1);
    x -= y;
    free(x);
}

static uint64_t extroot(uint64_t k, uint64_t n)
{
    uint64_t rh,rl,rm,i,pw;
    if(n==2) rh=4294967295;
    if(n==3) rh=2642245;
    if(n==4) rh=65535;
    rl=1;
    while(1)
    {
        rm=(rl+rh)/2;
        pw=1;
        for(i=0;i<n;i++) pw=pw*rm;
        if(pw==k) return rm;
        if(pw<k) rl=rm;
          else   rh=rm;
        if( (rh-rl)==1 ) return 1;
    }
    
}

/* smallest prime divisor of a  */

/* this routine works by trial division, and for the
   setting of very large prime fields, this should probably
   be dealt with in some other way  */

static uint64_t smlpr( uint64_t a )
{
    uint64_t b,c;
    if(a%2 == 0) return 2;

/* 4294967291 = 2^32 - 5 is the largest prime less than 2^32. */

    b=4294967291UL;
    b=b*b;
    if(a>b) c=b;
      else    c=a;
    for(b=3;b*b<=c;b+=2)
        if(a%b == 0) return b;
    return a;
}

static uint64_t smlpr16( uint64_t a )
{
    uint64_t b,c;
    if(a%2 == 0) return 2;

    b=4294967296UL;    // 2^32

    if(a>b) c=b;
      else    c=a;
    for(b=3;b*b<=c;b+=2)
        if(a%b == 0) return b;
    return a;
}

/*   p-routines for working mod p < 2^64    */

/*  a is reduced mod characteristic,  but b may be any uint64_t  */
static uint64_t padd (const FIELD * f, uint64_t a , uint64_t b)
{
    uint64_t c;
    c = a + b;
    if(c<a) c -= f->charc;
    return c%(f->charc);
}

static uint64_t pneg(const FIELD * f, uint64_t a)
{
    if(a==0) return 0;
    else return f->charc-a;
}

static uint64_t psub(const FIELD * f, uint64_t a , uint64_t b)
{
    uint64_t c;
    c=a-b;
    if(c>a) return c+f->charc;
    return c;
}

static uint64_t pinv(const FIELD * f, uint64_t a)
{
    uint64_t b1,b2,b3,c1,c2,c3,d,one;
    one=1;
/*   a.b1=c1  a.b2=c2  (a.b3=c3)  */
    if(a==one) return one;
    b1=one;
    c1=a;
    b2=one + f->charc/a;
    c2=pcpmad(f->charc,a,b2,0);
    if(c1>c2)
    {
        c3=c1;
        c1=c2;
        c2=c3;
        b3=b1;
        b1=b2;
        b2=b3;
    }
    while(c1 != 1)
    {
        d=c2/c1;
        c3=c2-d*c1;
        b3=psub(f,b2,pcpmad(f->charc,d,b1,0));  // can be done better
        b2=b1;
        c2=c1;
        b1=b3;
        c1=c3;
    }
    return b1;
}

static uint64_t ppow (const FIELD * f, uint64_t a , uint64_t b )
{
    uint64_t c;
    uint64_t one,zero;
    one=1;
    zero=0;
    c=one;
/* invariant   answer = c * (a^b) */
    while (b != zero)
    {
        if((b&one) == one) c = pcpmad(f->charc,a,c,0);
        b >>= one;
        a = pcpmad(f->charc,a,a,0);
    }
    return c;
}

static uint64_t pdiv(const FIELD * f, uint64_t a , uint64_t b)
{
    return pcpmad(f->charc,a,pinv(f,b),0);
}

/*  q-routines - works for all fields,  but may be slow */

static uint64_t qadd(const FIELD * f, uint64_t a , uint64_t b)
{
    unsigned int i;
    uint64_t c,one;
    one=1;
    if(f->pow == one) return padd(f,a,b);
    c=0;
    for(i=0;i<f->pow;i++)
    {
        c=c*f->charc+((a/f->qstep)+(b/f->qstep))%f->charc;
        a=(a%f->qstep)*f->charc;
        b=(b%f->qstep)*f->charc;
    }
    return c;
}

static uint64_t smul(const FIELD * f, uint64_t s , uint64_t a)
{
    unsigned int i;
    uint64_t c;
    c=0;
    for(i=0;i<f->pow;i++)
    {
        c=c*f->charc+((a/f->qstep)*s)%f->charc;
        a=(a%f->qstep)*f->charc;
    }
    return c;
}

static uint64_t qmul(const FIELD * f, uint64_t a , uint64_t b)
{
    unsigned int i;
    uint64_t c,one;
    one=1;
    if(f->pow == one) return pcpmad(f->charc,a,b,0);
    c=0;
    for(i=0;i<f->pow;i++)
    {
        c=qadd(f,c,smul(f,b%f->charc,a));
        b=b/f->charc;
        a=qadd(f,(a%f->qstep)*f->charc,smul(f,a/f->qstep,f->conp));
    }
    return c;
}

static void qdeg(const FIELD * f, uint64_t a, uint64_t * deg , uint64_t * lead)
{
    uint64_t d;
    d=0;
    while(a>=f->charc)
    {
        a=a/f->charc;
        d++;
    }
    *deg=d;
    *lead=a;
}

static uint64_t qinv(const FIELD * f, uint64_t a)
{
    uint64_t b1,b2,b3,c1,c2,c3,d1,d2,e1,e2,k;
    uint64_t one,temp;
    uint64_t i;
    one=1;
    if(f->pow==one) return pinv(f,a);

/*    a.b1=c1 (degree d1 leading coeff e1)
      a.b2=c2 (d2,e2 similarly)
      a.b3=c3   */
    b1=one;
    c1=a;
    qdeg(f,c1,&d1,&e1);
    k=one;
    for(i=0;i<f->pow-d1;i++)
        k=k*f->charc;
    b2=k;
    c2=qmul(f,k,a);
    qdeg(f,c2,&d2,&e2);
    while(d1 != 0)
    {
        if(d2>d1)
        {
             temp=b1;  b1=b2;  b2=temp;
             temp=c1;  c1=c2;  c2=temp;
             temp=d1;  d1=d2;  d2=temp;
             temp=e1;  e1=e2;  e2=temp;
        }
        k=1;
        for(i=0;i<d1-d2;i++)
            k=k*f->charc;
        k=k*pdiv(f,e1,pneg(f,e2));
        b3=qadd(f,b1,qmul(f,k,b2));
        c3=qadd(f,c1,qmul(f,k,c2));
        b1=b2;
        b2=b3;
        c1=c2;
        c2=c3;
        d1=d2;
        e1=e2;
        qdeg(f,c2,&d2,&e2);
    }
    return smul(f,pinv(f,e1),b1);
}

static uint64_t qpowr(const FIELD * f, uint64_t a , uint64_t b)
{
    uint64_t c,one,zero;
    zero=0;
    one=1;
    c=1;
    while (b!=zero)
    {
        if((b&one)==one) c=qmul(f,a,c);
        b>>=one;
        a=qmul(f,a,a);
    }
    return c;
}

static uint64_t qneg(const FIELD * f, uint64_t a)
{
    uint64_t one;
    one=1;
    if(f->pow == one) return pneg(f,a);
    return smul(f,f->charc-one,a);
}

static uint64_t qsub(const FIELD * f, uint64_t a , uint64_t b)
{
    uint64_t one;
    one=1;
    if(f->pow == one) return psub(f,a,b);
    return qadd(f,a,smul(f,f->charc-one,b));
}

static uint64_t qdiv(const FIELD * f, uint64_t a , uint64_t b)
{
    uint64_t one;
    one=1;
    if(f->pow == one) return pcpmad(f->charc,a,pinv(f,b),0);
    return qmul(f,a,qinv(f,b));
}

/* flags are bit-significant  */
/*   8 => return error code if fdef is not a prime power  */
/*   4 => really check that field is prime power, even if slow */
/*   last two bits 0 can use 0 for primitive root p > 2^32  */
/*                 1 get it right */
/*         (future 2 guess primitive root )  */

uint64_t conex(uint64_t fdef);

int  FieldASet1(uint64_t fdef, FIELD * f, int flags)
{
    uint64_t x,y,z,t,one,zero,two,ndv1,ndv2,ndv3,alpha,beta;
    uint64_t dv1[16],dv2[16],dv3[16],tfdef,gamma,delta;
    FELT a1,a2,a3,a4,a5,a6,a7,a8,b1,b2,b3,b4,b5,an,bn,cn;
    uint8_t * ftab8;
    uint8_t * f8;
    uint16_t * ftab16;
    uint16_t * sqrt;
    uint16_t * lfx;
    uint32_t * lfy;
    uint32_t * lfb;
    uint64_t * lfz;
    uint16_t * lfa;
    uint16_t * tra16;
    uint32_t * tra32;
    uint64_t * tra64;
    uint64_t i,j;
    int q1,q2,q3,q4,q5,q6,q7,bias;
    int r1,r2,r3,r4,r5,r6,r7,spacp;
    uint16_t s1,s2,s3,s4,s5,s6,s7,t1,t2,t3,t4,t5,t6,t7;
    uint16_t *pt1,*pt2;
//uint32_t bin;
// sort out cpuid and mact

    mactype(f->mact);
//bin=*((uint32_t *)&f->mact[4]);
//for(i=0;i<32;i++) {
//    if(bin&1) printf("1");
//       else   printf("0");
//     if((i%4)==3) printf(" "); 
//     bin>>=1; }
//  printf(" - %c\n",f->mact[0]);

//  strcpy(f->mact,"j00");

    f->threads=THREADS;
    f->megabytes=MEGABYTES;
    f->maxchop=MAXCHOP;

    f8=(uint8_t *)f;
    ftab8=f8 + sizeof(FIELD);
    ftab16=(uint16_t*)ftab8;
    zero=0;
    one=1;
    two=2;

/* start investigating fdef  */

    f->fdef = fdef;
    if(fdef<two)
    {
        if((flags&8)==8) return -1;  /* invalid field order 0 or 1 */
        printf("Invalid field order %ld\n",(long)fdef);
        exit(12);
    }
    if( (flags&4)==4 )  f->charc=smlpr(fdef);
    else
    {
/* if fdef is a prime power, and not divisible by a prime < 2^16 */
/* then it must be a perfect square or cube  */
/* we have been told (flags 4-bit unset) to trust it as a last resort */
        f->charc=smlpr16(fdef);
        if( (f->charc==fdef) && (fdef>4294967296UL) )
        {
            y=extroot(fdef,2);
            if(y==1) y=extroot(fdef,3);
            if(y!=1) f->charc=y;
        }
    }
/*    now f->charc is the characteristic */
    x=1;
    x=x<<48;
    if(f->charc==2) f->bar48=(x>>1);
          else      f->bar48=(x/f->charc)+1;
/*    calculate pow = degree of field  */
    x=fdef;
    f->pow = zero;
    while (x != one)
    {
        if(x%f->charc != zero)
        {
            if((flags&8)==8) return -1;  /* invalid field order 0 or 1 */
            printf("Invalid field order %ld\n",(long)fdef);
            exit(12);
        }
        f->pow++;
        x=x/f->charc;
    }
    sqrt=NULL;
    if(f->charc<65536)
    {
/* now populate squareroot table if p < 65536 */
        sqrt=malloc(2*f->charc);
        for(i=0;i<f->charc;i++) sqrt[i]=0;
        for(y=1;y<=(f->charc-1)/2;y++)
        {
            z=(y*y)%f->charc;
            sqrt[z]=y;
        }
    }

    x=65536;
                       f->paktyp = 0;
    if(fdef<=x*x)      f->paktyp = 1;
    if(fdef<=x)
    {
                       f->paktyp = 2;
        if(fdef<257)   f->paktyp = 3;
        if(fdef<17)    f->paktyp = 4;
        if(fdef==5)    f->paktyp = 5;
        if(fdef==4)    f->paktyp = 6;
        if(fdef==3)    f->paktyp = 7;
        if(fdef==2)    f->paktyp = 8;
    }

                           f->ppaktyp = 0;
    if(f->charc<=x*x)      f->ppaktyp = 1;
    if(f->charc<=x)
    {
                           f->ppaktyp = 2;
        if(f->charc<257)   f->ppaktyp = 3;
        if(f->charc<17)    f->ppaktyp = 4;
        if(f->charc==5)    f->ppaktyp = 5;
        if(f->charc==3)    f->ppaktyp = 7;
        if(f->charc==2)    f->ppaktyp = 8;
    }


/* First look to see if the Conway Polynomial is listed */
    f->conp=conex(fdef);
    f->qstep=fdef/f->charc;
    if(f->conp==0)
    {
/* prime factors of charc-1 into dv1.            */
/* note that any 16 distinct primes multiplied   */
/* comes to more than 2^64                       */
/* 1 + 2.3.5.7.11.13.17.19.23.29.31.37.41.43.53  */
/* is indeed prime                               */
        z=1;   /* did we factorize charc-1 */
        if( (flags&3)==1 )  /* primitive root compulsory */
        {
            x=f->charc-one;
            ndv1=zero;
            while(x!=one)
            {
                y = smlpr(x);
                dv1[ndv1++] = y;
                while(x%y==0) x = x/y;
            }
        }
        else
        {
            x=f->charc-one;
            ndv1=zero;
            while(x!=one)
            {
                y = smlpr16(x);
                dv1[ndv1++] = y;
                while(x%y==0) x = x/y;
                if(y>4294967296) z=0;    // y may not be prime
            }
        }
        alpha = 0;
        if(z==1)
        {
            while(1)
            {
                alpha++;
                for(i=0;i<ndv1;i++)
                    if ( ppow(f,alpha,(f->charc-one)/dv1[i]) == one) break;
                if(i==ndv1) break;
            }
            f->conp=alpha;
            if(f->pow%2==0) f->conp=f->charc-alpha;
            if(f->pow!=1)
            {
/*  prime factors of q-1 into dv2   */
                x=fdef-one;
                ndv2=zero;
                while(x!=one)
                {
                    y = smlpr(x);
                    dv2[ndv2++] = y;
                    while(x%y==0) x = x/y;
                }
                if(f->pow==4)
                {
                    f->pow=2;
                    tfdef=f->charc*f->charc;
/*  prime factors of p^2-1 into dv3   */
                    x=tfdef-one;
                    ndv3=zero;
                    while(x!=one)
                    {
                        y = smlpr(x);
                        dv3[ndv3++] = y;
                        while(x%y==0) x = x/y;
                    }
/* first find beta where quadratic is x^2 - beta + alpha  */
                    f->conp = f->charc - alpha;
                    f->qstep=tfdef/f->charc;
                    for(beta=1;beta<f->charc;beta++)
                    {
                        f->conp+=f->charc;
                        if(qpowr(f,f->charc,tfdef-1)!=1) continue;
                        for(j=0;j<ndv3;j++)
                            if(qpowr(f,f->charc,(tfdef-1)/dv3[j])==1) break;
                        if(j==ndv3) break;
                    }
                    z=(4*f->charc + beta*beta - 4*alpha)%f->charc;/*disc*/
                    f->qstep=fdef/f->charc;
                    f->pow=4;
/* got beta.  now compute polynomial */
                    j=0;
                    for(gamma=0;gamma<f->charc;gamma++)
                    {
                        x=((f->charc + beta - gamma)*z)%f->charc;
                        if(sqrt[x]==0) continue;
                        delta=sqrt[x];
                        for(i=0;i<2;i++)
                        {
                            f->conp=(f->charc-gamma)*f->charc*f->charc
                                + delta*f->charc + f->charc-alpha;
                            if(i==1)
                            f->conp=(f->charc-gamma)*f->charc*f->charc
                     + ((f->charc-delta)%f->charc)*f->charc + f->charc-alpha;
                            if(qpowr(f,f->charc,fdef-1)!=1) continue;
                            for(j=0;j<ndv2;j++)
                                if(qpowr(f,f->charc,(fdef-1)/dv2[j])==1) break;
                            if(j==ndv2) break;
                        }
                        if(j==ndv2) break;
                        f->conp=2;  // deterministically wrong for conreg /
                    }
                }
                else
                {
/*  find first primitive polynomial x^n+-<beta>.x -+<alpha>   */
                    j=0;
                    if(f->pow%2==0) f->conp = f->charc - alpha;
                            else    f->conp = alpha +f->charc*f->charc;
                    for(i=1;i<f->charc;i++)
                    {
                        if(f->pow%2==0) f->conp+=f->charc;
                              else      f->conp-=f->charc;
                        if(qpowr(f,f->charc,fdef-1)!=1) continue;
                        for(j=0;j<ndv2;j++)
                            if(qpowr(f,f->charc,(fdef-1)/dv2[j])==1) break;
                        if(j==ndv2) break;
                    }
                    if(j!=ndv2) f->conp=2;
                }
            }
        }
    }
    free(sqrt);
/* So now we have the Conway Polynomial  */
    f->cp0=f->conp%f->charc;
    f->cp1=(f->conp/f->charc)%f->charc;
    f->entbyte=1;
    f->bytesper=1;
    f->dbytevals=f->fdef;
    f->qminus1=f->fdef-1;
    f->p32=f->charc;   // if it doesn't fit, it isn't used
    f->spaclev=0;   /* for both spac and bar41 */
    if( f->charc < 13360) f->spaclev=1;
    if( (f->charc>=23) && (f->charc<=127) ) f->spaclev=2;
    if( (f->charc>=11) && (f->charc<=19) ) f->spaclev=3;
    if( f->charc==7) f->spaclev=4;
    if( f->charc==5) f->spaclev=5;
    if( f->charc==3) f->spaclev=6;
    if( f->charc==2) f->spaclev=8;
    f->digit=1;
    for(i=0;i<f->spaclev;i++)
        f->digit=f->digit*f->charc;
    if(f->charc==2) f->digit=256;
    f->digit2=f->digit*f->digit;
    i=f->fdef;
    f->nodigits=0;
    if(f->digit>1)
    {
        while(i>1)
        {
            i=i/f->digit;
            f->nodigits++;
        }
    }

    switch(f->paktyp)
    {
        case 0:
          f->bytesper=8;
          if(f->charc==2)
            {
                f->madtyp=13;
                f->clpm[2]=64-f->pow;
                x=1;
                t=f->conp+(x<<f->pow);  // t is whole Conway polynomial
                f->clpm[1]=t<<(f->clpm[2]-1);
// compute z = X^2d/t by polynomial long division.
                x=x<<(f->pow);   // x is just X^d

                y=x;
                z=0;
                for(i=0;i<=f->pow;i++)
                {
                    z=z<<1;
                    if((x&y)!=0)
                    {
                        z++;
                        y^=t;
                    }
                    y=y<<1;
                }
                f->clpm[0]=z<<(f->clpm[2]-1);   // e left justified!
            }
          else f->madtyp=12;
          if(f->charc==2) f->addtyp=1;
          else if (f->pow==1) f->addtyp=9;
          else f->addtyp=10;
          f->multyp=7;
          if((f->charc==2)&&(f->mact[0]>='g')) f->multyp=9;
          break;
        case 1:
          f->bytesper=4;
          if(f->pow==1) f->madtyp=9;
            else if (f->charc==2)
            {
                f->madtyp=11;
                f->clpm[2]=64-f->pow;
                x=1;
                t=f->conp+(x<<f->pow);  // t is whole Conway polynomial
                f->clpm[1]=t<<(f->clpm[2]-1);
// compute z = X^2d/t by polynomial long division.
                x=x<<(f->pow);   // x is just X^d

                y=x;
                z=0;
                for(i=0;i<=f->pow;i++)
                {
                    z=z<<1;
                    if((x&y)!=0)
                    {
                        z++;
                        y^=t;
                    }
                    y=y<<1;
                }
                f->clpm[0]=z<<(f->clpm[2]-1);   // e left justified!
            }
            else f->madtyp=10;
/* 32-bit addtyp  */
          if(f->charc==2) f->addtyp=1;
          else
          {
              if (f->pow==1)
                  f->addtyp=7;
              else 
              {
                  f->addtyp=8;
                  if( (f->pow <= 2*f->spaclev) && (f->spaclev>1) )
                              f->addtyp=11;
                  if( (f->pow == 2) && (f->spaclev==1) )
                              f->addtyp=13;
                  if( (f->pow==3) && (f->spaclev==1) && (f->charc<1278) )
                              f->addtyp=14;
              }
          }
/* multyp  */
          if(f->pow==1) f->multyp=5;
                   else f->multyp=6;
          if((f->charc==2)&&(f->mact[0]>='g')) f->multyp=8;
          break;
        case 2:
          f->bytesper=2;
          if(f->pow==1) f->madtyp=4;
            else if (f->charc==2)
            {
                if(f->fdef<4200) f->madtyp=6;
                       else      f->madtyp=8;
            }
            else if (f->fdef<4200) f->madtyp=5;
                            else   f->madtyp=7;
          if(f->charc==2) f->addtyp=1;
            else if (f->pow==1) f->addtyp=4;
            else
            {
                f->addtyp=6;
                if(f->spaclev>=2)
                {
                    if(f->pow<=2*f->spaclev) f->addtyp=12;
                    if(f->pow<=f->spaclev) f->addtyp=5;
                }
            }
          if(f->pow==1) f->multyp=2;
            else if(f->fdef<4200) f->multyp=3;
                             else f->multyp=4;
          break;
        case 3:
          if(f->charc==2) f->madtyp=1;
          else if (f->pow==1) f->madtyp=2;
               else           f->madtyp=2;
          if(f->charc==2) f->addtyp=1;
          else if (f->pow==1) f->addtyp=3;
              else        f->addtyp=2;
          f->multyp=1;
          break;
        case 4:
          f->entbyte=2;
          f->dbytevals=f->fdef*f->fdef;
          if(f->charc==2) f->madtyp=1;
               else       f->madtyp=2;
          if(f->charc==2) f->addtyp=1;
               else       f->addtyp=2;
          f->multyp=1;
          break;
        case 5:
          f->entbyte=3;
          f->dbytevals=125;
          f->madtyp=2;
          f->addtyp=2;
          f->multyp=1;
          break;
        case 6:
          f->entbyte=4;
          f->dbytevals=256;
          f->madtyp=1;
          f->addtyp=1;
          f->multyp=1;
          break;
        case 7:
          f->entbyte=5;
          f->dbytevals=243;
          f->madtyp=2;
          f->addtyp=2;
          f->multyp=1;
          break;
        case 8:
          f->entbyte=8;
          f->dbytevals=256;
          f->madtyp=1;
          f->addtyp=1;
          f->multyp=1;
          break;
    }

// now a similar thing for paddtyp etc.

    switch(f->ppaktyp)
    {
      case 0:
        f->pentbyte=1;
        f->pbytesper=8;
        f->paddtyp=9;
        f->pmadtyp=12;
        f->pmultyp=7;
        break;
      case 1:
        f->pentbyte=1;
        f->pbytesper=4;
        f->paddtyp=7;
        f->pmadtyp=9;
        f->pmultyp=5;
        break;
      case 2:
        f->pentbyte=1;
        f->pbytesper=2;
        f->paddtyp=4;
        f->pmadtyp=4;
        f->pmultyp=2;
        break;
      case 3:
        f->pentbyte=1;
        f->pbytesper=1;
        f->paddtyp=3;
        f->pmadtyp=2;
        f->pmultyp=1;
        break;
      case 4:
        f->pentbyte=2;
        f->pbytesper=1;
        f->paddtyp=2;
        f->pmadtyp=2;
        f->pmultyp=1;
        break;
      case 5:
        f->pentbyte=3;
        f->pbytesper=1;
        f->paddtyp=2;
        f->pmadtyp=2;
        f->pmultyp=1;
        break;
      case 7:
        f->pentbyte=5;
        f->pbytesper=1;
        f->paddtyp=2;
        f->pmadtyp=2;
        f->pmultyp=1;
        break;
      case 8:
        f->pentbyte=8;
        f->pbytesper=1;
        f->paddtyp=1;
        f->pmadtyp=1;
        f->pmultyp=1;
        break;
      default:
        printf("Internal field error - ppaktyp not set\n");
        exit(42);
        break;
    }
/* now to populate the look-up tables    */
    if( f->fdef<=65536 )
    {
        x=1;
        for(z=0;z<f->fdef-1;z++)
        {
            f->log16[x]=z;
            f->alog16[z]=x;
            f->alog16[z+f->fdef-1]=x;
            if(f->pow==1)
            {
                x=(x*f->conp)%f->fdef;
                continue;
            }
            x=x*f->charc;
            y=x/f->fdef;
            x=x%f->fdef;
            if(f->charc!=2)    x=qadd(f,x,smul(f,y,f->conp));
                else           if(y==1) x^=f->conp;
        }
        f->log16[0]=0xffff;
        for(x=f->fdef-1;x<3*f->fdef;x++)
                f->alog16[x]=f->alog16[x-f->fdef+1];
        if(f->charc!=2)
        {
            for(x=0;x<f->fdef-1;x++)
            {
                z=qadd(f,one,f->alog16[x]);
                if(z==0) f->zech16[x]=f->qminus1;
                else f->zech16[x]=f->log16[z];
            }
            for(x=f->fdef-1;x<3*f->fdef;x++)
                f->zech16[x]=f->zech16[x-f->fdef+1];
        }
    }
/*    Populate the spac16 and sqid16 tables    */
/*    Also bar41 depending only on characteristic */

    if(f->spaclev==1)
    {
        x=1;
        x=x<<41;
        f->bar41 = 1 + x/f->charc;
    }
    if(f->spaclev>1)
    {
        q1=1;
        q2=1;
        f->spaczero=0;
        for(i=0;i<f->spaclev;i++)
        {
            q1=q1*f->charc;
            q2=q2*(2*f->charc-1);
            f->spaczero = f->spaczero*(2*f->charc-1)+f->charc-1;
        }
        x=1;
        x=x<<41;
        f->bar41=1 + x/q1;
        if(f->spaclev<=6) q1=1;  else q1=f->charc;
        if(f->spaclev<=5) q2=1;  else q2=f->charc;
        if(f->spaclev<=4) q3=1;  else q3=f->charc;
        if(f->spaclev<=3) q4=1;  else q4=f->charc;
        if(f->spaclev<=2) q5=1;  else q5=f->charc;
        q6=f->charc;
        q7=f->charc;
        bias=(f->charc-1)/2;
        pt1=&(f->spac16[0]);
        spacp=2*f->charc-1;
        for(r1=0;r1<q1;r1++) {
          if(f->spaclev<=6) s1=0; else s1=(r1+bias)%f->charc;
          t1=s1;
          for(r2=0;r2<q2;r2++) {
            if(f->spaclev<=5) s2=0; else s2=(r2+bias)%f->charc;
            t2=t1*spacp+s2;
            for(r3=0;r3<q3;r3++) {
              if(f->spaclev<=4) s3=0; else s3=(r3+bias)%f->charc;
              t3=t2*spacp+s3;
              for(r4=0;r4<q4;r4++) {
                if(f->spaclev<=3) s4=0; else s4=(r4+bias)%f->charc;
                t4=t3*spacp+s4;
                for(r5=0;r5<q5;r5++) {
                  if(f->spaclev<=2) s5=0; else s5=(r5+bias)%f->charc;
                  t5=t4*spacp+s5;
                  for(r6=0;r6<q6;r6++) {
                    s6=(r6+bias)%f->charc;
                    t6=t5*spacp+s6;
                    for(r7=0;r7<q7;r7++) {
                      s7=(r7+bias)%f->charc;
                      t7=t6*spacp+s7;
                      *(pt1++)=t7;
        } } } } } } }
        if(f->spaclev<=6) q1=1;  else q1=2*f->charc-1;
        if(f->spaclev<=5) q2=1;  else q2=2*f->charc-1;
        if(f->spaclev<=4) q3=1;  else q3=2*f->charc-1;
        if(f->spaclev<=3) q4=1;  else q4=2*f->charc-1;
        if(f->spaclev<=2) q5=1;  else q5=2*f->charc-1;
        q6=2*f->charc-1;
        q7=2*f->charc-1;
        pt2=&(f->sqid16[0]);
        for(r1=0;r1<q1;r1++) {
          if(f->spaclev<=6) s1=0; else s1=(r1+1)%f->charc;
          t1=s1;
          for(r2=0;r2<q2;r2++) {
            if(f->spaclev<=5) s2=0; else s2=(r2+1)%f->charc;
            t2=t1*f->charc+s2;
            for(r3=0;r3<q3;r3++) {
              if(f->spaclev<=4) s3=0; else s3=(r3+1)%f->charc;
              t3=t2*f->charc+s3;
              for(r4=0;r4<q4;r4++) {
                if(f->spaclev<=3) s4=0; else s4=(r4+1)%f->charc;
                t4=t3*f->charc+s4;
                for(r5=0;r5<q5;r5++) {
                  if(f->spaclev<=2) s5=0; else s5=(r5+1)%f->charc;
                  t5=t4*f->charc+s5;
                  for(r6=0;r6<q6;r6++) {
                    s6=(r6+1)%f->charc;
                    t6=t5*f->charc+s6;
                    for(r7=0;r7<q7;r7++) {
                      s7=(r7+1)%f->charc;
                      t7=t6*f->charc+s7;
                      *(pt2++)=t7;
        } } } } } } }
        f->spacneg=f->spaczero+f->spac16[0];
    }
    if( f->charc<65536 )
    {
        y=0;
        for(x=0;x<131072;x++)
        {
            f->red16[x]=y;
            y++;
            if(y==f->charc) y=0;
        }
    }

/*   end of  16-bit look-up tables.  Now for the 8-bit ones  */

    ftab8=(uint8_t*)ftab16;

/*    add8 depends only on the characterstic and not 2*/
    if(f->charc<256)
    {
        switch(f->charc)
        {
          case 2:
            for(a1=0;a1<256;a1++)
              for(b1=0;b1<256;b1++)
                f->add8[256*a1+b1]=(a1^b1);
            break;
/*   add8 table for characteristic 3  */
          case 3:
            for (a1=0;a1<3;a1++)
              for (a2=0;a2<3;a2++)
                for (a3=0;a3<3;a3++)
                  for (a4=0;a4<3;a4++)
                    for (a5=0;a5<3;a5++)
                      for (b1=0;b1<3;b1++)
                        for (b2=0;b2<3;b2++)
                          for (b3=0;b3<3;b3++)
                            for (b4=0;b4<3;b4++)
                              for (b5=0;b5<3;b5++)
            {
                an=a1*81+a2*27+a3*9+a4*3+a5;
                bn=b1*81+b2*27+b3*9+b4*3+b5;
                cn=((a1+b1)%3)*81 +  ((a2+b2)%3)*27  +
                   ((a3+b3)%3)*9  +  ((a4+b4)%3)*3  + (a5+b5)%3;
                f->add8[256*an+bn]=cn;
            }
            break;
/*   add8 table for characteristic 5  */
          case 5:
            for (a1=0;a1<5;a1++)
              for (a2=0;a2<5;a2++)
                for (a3=0;a3<5;a3++)
                  for (b1=0;b1<5;b1++)
                    for (b2=0;b2<5;b2++)
                      for (b3=0;b3<5;b3++)
                      {
                          an=a1*25+a2*5+a3;
                          bn=b1*25+b2*5+b3;
                          cn=((a1+b1)%5)*25+((a2+b2)%5)*5+(a3+b3)%5;
                          f->add8[256*an+bn]=cn;
                      }
            break;
          case 7:
          case 11:
          case 13:
            for (a1=0;a1<f->charc;a1++)
              for (a2=0;a2<f->charc;a2++)
                for(b1=0;b1<f->charc;b1++)
                  for (b2=0;b2<f->charc;b2++)
                  {
                      an=a1*f->charc+a2;
                      bn=b1*f->charc+b2;
                      cn=((a1+b1)%f->charc)*f->charc + (a2+b2)%f->charc;
                      f->add8[256*an+bn]=cn;
                  }
            break;
          default:
            for (a1=0;a1<f->charc;a1++)
              for (b1=0;b1<f->charc;b1++)
                f->add8[256*a1+b1]=(a1+b1)%f->charc;
            break;
        }
    }

    a5=f->fdef;          // number of values of a
    if(f->fdef>256) a5=f->charc;
    if(a5<=256)
    {
/* notice that characteristic tables may be partly overwritten */
/* by later field tables for 9,27,81,25,125,49,121,169         */
/* and for 4,8,16,32,64,128 and 256                            */
/* but only for ground field multipliers where the answer      */
/* is anyway the same!                                         */
        if(f->charc==2)
        {
            for(a1=0;a1<256;a1++) f->mul8[a1]=0;
            for(a1=0;a1<256;a1++) f->mul8[a1+256]=a1;
        }
        if(f->charc==3)
        {
            for (a1=0;a1<3;a1++)
              for (b1=0;b1<3;b1++)
                for (b2=0;b2<3;b2++)
                  for (b3=0;b3<3;b3++)
                    for (b4=0;b4<3;b4++)
                      for (b5=0;b5<3;b5++)
            {
                bn=b1*81+b2*27+b3*9+b4*3+b5;
                cn=((a1*b1)%3)*81 +  ((a1*b2)%3)*27  +
                   ((a1*b3)%3)*9  +  ((a1*b4)%3)*3  + (a1*b5)%3;
                f->mul8[256*a1+bn]=cn;
            }
        }
        if(f->charc==5)
        {
/*   mul8 table for the field of order 5  */
            for (a1=0;a1<5;a1++)
              for (b1=0;b1<5;b1++)
                for (b2=0;b2<5;b2++)
                  for (b3=0;b3<5;b3++)
                  {
                      bn=b1*25+b2*5+b3;
                      cn=((a1*b1)%5)*25+((a1*b2)%5)*5+(a1*b3)%5;
                      f->mul8[256*a1+bn]=cn;
                  }
        }
        if( (f->charc>6) && (f->charc<16) )    // 7 11 13
        {
            for (a1=0;a1<f->charc;a1++)
              for (b1=0;b1<f->charc;b1++)
                for (b2=0;b2<f->charc;b2++)
                {
                    bn=b1*f->charc + b2;
                    cn=((a1*b1)%f->charc)*f->charc+(a1*b2)%f->charc;
                    f->mul8[256*a1+bn]=cn;
                }
        }
        if( (f->charc>16) && (f->charc<256) )   // one byte primes 17-251
        {
            for (a1=0;a1<f->charc;a1++)
              for (b1=0;b1<f->charc;b1++)
                  f->mul8[256*a1+b1]=(a1*b1)%f->charc;
        }
        if(f->fdef==4)
        {
            for (a1=0;a1<4;a1++)
              for (b1=0;b1<4;b1++)
                for (b2=0;b2<4;b2++)
                  for (b3=0;b3<4;b3++)
                    for (b4=0;b4<4;b4++)
                    {
                        bn=b1*64+b2*16+b3*4+b4;
    cn=qmul(f,a1,b1)*64+qmul(f,a1,b2)*16+qmul(f,a1,b3)*4+qmul(f,a1,b4);
                        f->mul8[256*a1+bn]=cn;
                    }
        }
        if( (f->fdef==8) || (f->fdef==9) || (f->fdef==16) )
        {
            for (a1=0;a1<f->fdef;a1++)
              for(b1=0;b1<f->fdef;b1++)
                for (b2=0;b2<f->fdef;b2++)
                {
                    bn=b1*f->fdef+b2;
                    cn=qmul(f,a1,b1)*f->fdef+qmul(f,a1,b2);
                    f->mul8[256*a1+bn]=cn;
                }
        }
        if( (f->pow>1) && (f->fdef>16) && (f->fdef<=256) )
        {
            for (a1=0;a1<f->fdef;a1++)
              for (b1=0;b1<f->fdef;b1++)
              {
                if( (a1==0) || (b1==0) ) f->mul8[256*a1+b1]=0;
            else f->mul8[256*a1+b1]=f->alog16[f->log16[a1]+f->log16[b1]];
              }
        }
    }

/* sub8 table  - same fields as add8 */
    if((f->charc!=2) && (f->charc<256) )
    {
        b5=f->charc;
        if(f->charc<16) b5=f->charc*f->charc;
        if(f->charc==3) b5=243;
        if(f->charc==5) b5=125;
        x=(f->charc-1)*256;
        for (a1=0;a1<b5;a1++)
            for (b1=0;b1<b5;b1++)
                f->sub8[256*a1+b1]=
                    f->add8[256*a1+f->mul8[x+b1]];
    }

/* inv8 table */
    if (f->fdef <= 256)
    {
        f->inv8[0]=0;
        for(a1=1;a1<f->fdef;a1++) f->inv8[a1]=qinv(f,a1);
    }
/* early8 and late8 tables */
    if(f->fdef <=16)
    {
        for(a1=0;a1+1<f->entbyte;a1++)
        {
            for(a2=0;a2<f->dbytevals;a2++)
            {
                a3=a2;
                for(i=0;i<a1+1;i++) a3= (a3*f->fdef)%f->dbytevals;
                f->late8[a1*f->dbytevals+a2]=a3;
            }
        }

        for(a1=0;a1+1<f->entbyte;a1++)
        {
            for(a2=0;a2<f->dbytevals;a2++)
            {
                a3=a2;
                for(i=0;i<a1+1;i++) a3= (a3/f->fdef);
                f->early8[a1*f->dbytevals+a2]=a3;
            }
        }
    }

// now for the linf tables
    f->pastype=5;    
    if(f->fdef==4)
    {
        f->pastype=1;
        while( (((long)ftab8)&1) != 0 ) ftab8++;  // align 16 bits
        f->Tlfx=ftab8-f8;
        lfx=(uint16_t *) ftab8;
        ftab8+=512;
        f->Tlfa=ftab8-f8;
        lfa=(uint16_t *) ftab8;
        ftab8+=512;
        for(a1=0;a1<2;a1++)
         for(a2=0;a2<2;a2++)
          for(a3=0;a3<2;a3++)
           for(a4=0;a4<2;a4++)
            for(a5=0;a5<2;a5++)
             for(a6=0;a6<2;a6++)
              for(a7=0;a7<2;a7++)
               for(a8=0;a8<2;a8++)
               {
                  lfx[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                  2048*a8+1024*a6+512*a4+256*a2+8*a7+4*a5+2*a3+a1;
                  lfa[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                  16384*a8+4096*a7+1024*a6+256*a5+64*a4+16*a3+4*a2+a1;
               }
    }

    if(f->fdef==8)
    {
        f->pastype=2;
        while( (((long)ftab8)&3) != 0 ) ftab8++;  // align 32 bits
        f->Tlfx=ftab8-f8;
        lfy=(uint32_t *) ftab8;
        ftab8+=256;           // 64 * sizof(uint32_t)
        f->Tlfa=ftab8-f8;
        lfb=(uint32_t *) ftab8;
        ftab8+=1024;           // 256 * sizof(uint32_t)
        for(a1=0;a1<2;a1++)
         for(a2=0;a2<2;a2++)
          for(a3=0;a3<2;a3++)
           for(a4=0;a4<2;a4++)
            for(a5=0;a5<2;a5++)
             for(a6=0;a6<2;a6++)
             {
                  lfy[32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                  131072*a6+65536*a3+512*a5+256*a2+2*a4+a1;
                  for(a7=0;a7<2;a7++)
                   for(a8=0;a8<2;a8++)
                    lfb[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                    134217728*a8+16777216*a7+524288*a6+65536*a5+
                      2048*a4+256*a3+8*a2+a1;                
             }
    }

    if(f->fdef==9)
    {
        f->pastype=3;
        while( (((long)ftab8)&3) != 0 ) ftab8++;  // align 32 bits
        f->Tlfx=ftab8-f8;
        lfy=(uint32_t *) ftab8;
        ftab8+=648;    // 81 (F9) * 2(tables) * 4(bytes)
        f->Tlfa=ftab8-f8;
        lfb=(uint32_t *) ftab8;
        ftab8+=1944;    // 243 x 2 4-byte entries
        for(a1=0;a1<3;a1++)
         for(a2=0;a2<3;a2++)
          for(a3=0;a3<3;a3++)
           for(a4=0;a4<3;a4++)
           {
//              | . . . . . |. . . a4 a2| . . . . .| . . . a3 a1|
             lfy[27*a4+9*a3+3*a2+a1] = 196608*a4+65536*a2+3*a3+a1;
//              | . . . . a4|a2 . . . . |. . . . a3| a1 . . . . |
             lfy[81+27*a4+9*a3+3*a2+a1] = 16777216*a4+5308416*a2+256*a3+81*a1;
            for(a5=0;a5<3;a5++)
            {
             lfb[81*a5+27*a4+9*a3+3*a2+a1] =
//               | . . . . a5 | . . a4 . a3|. . a2 . a1|
                65536*a5+2304*a4+256*a3+9*a2+a1;
             lfb[243+81*a5+27*a4+9*a3+3*a2+a1] =
//               | . . a5 . a4| . . a3 . a2| . . a1 . .|
                589824*a5+65536*a4+2304*a3+256*a2+a1*9;
            }
           }
           
    }
    if(f->fdef==16)
    {
        f->pastype=4;
        while( (((long)ftab8)&3) != 0 ) ftab8++;  // align 32 bits
        f->Tlfx=ftab8-f8;
        lfy=(uint32_t *) ftab8;
        ftab8+=1024;           // 256 * sizof(uint32_t)
        f->Tlfa=ftab8-f8;
        lfb=(uint32_t *) ftab8;
        ftab8+=1024;           // 256 * sizof(uint32_t)
        for(a1=0;a1<2;a1++)
         for(a2=0;a2<2;a2++)
          for(a3=0;a3<2;a3++)
           for(a4=0;a4<2;a4++)
            for(a5=0;a5<2;a5++)
             for(a6=0;a6<2;a6++)
              for(a7=0;a7<2;a7++)
               for(a8=0;a8<2;a8++)
               {
                  lfy[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                  33554432*a8+16777216*a4+131072*a7+65536*a3
                        +512*a6+256*a2+2*a5+a1;
                  lfb[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                    268435456*a8+16777216*a7+1048576*a6+65536*a5+
                      4096*a4+256*a3+16*a2+a1;                
               }
    }
    if( (f->fdef>=32) && (f->charc==2) )
    {
        while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
        f->Tlfx=ftab8-f8;
        lfz=(uint64_t *) ftab8;
        ftab8+=8*256;
        for(a1=0;a1<2;a1++)
         for(a2=0;a2<2;a2++)
          for(a3=0;a3<2;a3++)
           for(a4=0;a4<2;a4++)
            for(a5=0;a5<2;a5++)
             for(a6=0;a6<2;a6++)
              for(a7=0;a7<2;a7++)
               for(a8=0;a8<2;a8++)
               {
                  lfz[128*a8+64*a7+32*a6+16*a5+8*a4+4*a3+2*a2+a1]=
                  0x100000000000000*a8+0x1000000000000*a7+
                  0x10000000000*a6+0x100000000*a5+0x1000000*a4+0x10000*a3+0x100*a2+a1;
               }
    }
    if( (f->fdef>=27) && (f->charc==3) )
    {
        while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
        f->Tlfx=ftab8-f8;
        lfz=(uint64_t *) ftab8;
        ftab8+=8*729;
        for(a1=0;a1<3;a1++)
         for(a2=0;a2<3;a2++)
          for(a3=0;a3<3;a3++)
           for(a4=0;a4<3;a4++)
            for(a5=0;a5<3;a5++)
             for(a6=0;a6<3;a6++)
             {
                  lfz[243*a6+81*a5+27*a4+9*a3+3*a2+a1]=
                  0x10000000000*a6+0x100000000*a5+0x1000000*a4+0x10000*a3+0x100*a2+a1;
             }
    }
    if(f->charc==5)  
    {
        while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
        f->Tlfx=ftab8-f8;
        lfz=(uint64_t *) ftab8;
        ftab8+=8*3125;
        for(a1=0;a1<5;a1++)
         for(a2=0;a2<5;a2++)
          for(a3=0;a3<5;a3++)
           for(a4=0;a4<5;a4++)
            for(a5=0;a5<5;a5++)
            {
                lfz[625*a5+125*a4+25*a3+5*a2+a1]=
                0x100000000*a5+0x1000000*a4+0x10000*a3+0x100*a2+a1;
            }
    }
    if (f->charc==7)
    {
        while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
        f->Tlfx=ftab8-f8;
        lfz=(uint64_t *) ftab8;
        ftab8+=8*2401;
        for(a1=0;a1<7;a1++)
         for(a2=0;a2<7;a2++)
          for(a3=0;a3<7;a3++)
           for(a4=0;a4<7;a4++)
           {
               lfz[343*a4+49*a3+7*a2+a1]=
               0x1000000*a4+0x10000*a3+0x100*a2+a1;
           }
    }
    if( (f->charc>=11) && (f->charc<=19) )
    {
        while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
        f->Tlfx=ftab8-f8;
        lfz=(uint64_t *) ftab8;
        ftab8+=8*f->charc*f->charc*f->charc;
        for(a1=0;a1<f->charc;a1++)
         for(a2=0;a2<f->charc;a2++)
          for(a3=0;a3<f->charc;a3++)
          {
              lfz[f->charc*f->charc*a3+f->charc*a2+a1]=
              0x10000*a3+0x100*a2+a1;
          }
    }

//  sort out pextype - how to do PExtract

    f->pextype=5;      // default - ulitmately obsolete
    if(f->ppaktyp<=3)  // prime 8 bits or bigger
    {
        f->pextype=6;
        pcbarprp(f->paktyp,f->ppaktyp,f->charc,f->pow,
                            f->fdef-1,&(f->barpar[0]));
    }
    if( (f->ppaktyp>=4) && (f->ppaktyp<=7) )   // charc 3,5,7,11,13
    {
        if(f->fdef==9) f->pextype=3;   // GF9
        if(f->paktyp==3) f->pextype=7;  // 27,81,243,25,125,49,121,169
// next is uint16_t FELT but still less than digit
        if( (f->paktyp==2)&&(f->nodigits==1) ) f->pextype=10;
        if((f->paktyp<3)&&(f->nodigits>1))
        {
            pcbarprp(f->paktyp,2,f->digit, f->nodigits,
                            f->fdef-1,&(f->barpar[0]));
            f->atatime=EXTIVCNT/f->nodigits;
//  each chunk but the last must be an integral number of bytes
            while((f->atatime%f->pentbyte)!=0) 
                    f->atatime--;
            if(f->atatime==0)
            {
                printf("EXTIVCNT too low at %d- aborting\n",EXTIVCNT);
                exit(62);
            }
            f->pextype=9;
        }
// otherwise default so far
    }
    if(f->ppaktyp==8)  // characteristic 2
    {
        if(f->pow==2) f->pextype=1;  // GF4
        if(f->pow==3) f->pextype=2;  // GF8
        if(f->pow==4) f->pextype=4;  // GF16
        if(f->paktyp==3) f->pextype=7;  // 32,64,128,256
        if(f->paktyp <3) f->pextype=8;  // field 16 bits or bigger
    }

    if(f->paktyp==3) f->pastype=6;
    if((f->paktyp==2)&&(f->charc<=13)&&(f->fdef!=16807) ) f->pastype=7;

/*  Now for the transpose tables 2-16 */
    if(f->fdef<=16)
    {

        if(f->fdef>5)
        {
            while( (((long)ftab8)&1) != 0 ) ftab8++;  // align 16 bits
            f->Ttra=ftab8-f8;
            tra16=(uint16_t *)ftab8;
            for(a1=0;a1<f->fdef;a1++)
              for(a2=0;a2<f->fdef;a2++)
                tra16[a1+(f->fdef*a2)]=a1+256*a2;
            ftab8+=f->fdef*f->fdef*2;
        }
        if(f->fdef==5)
        {
            while( (((long)ftab8)&3) != 0 ) ftab8++;  // align 32 bits
            f->Ttra=ftab8-f8;
            tra32=(uint32_t *)ftab8;
            for(a1=0;a1<5;a1++)
              for(a2=0;a2<5;a2++)
                for(a3=0;a3<5;a3++)
                  tra32[a1+5*a2+25*a3]=a1+256*a2+65536*a3;
            ftab8+=125*4;
        }
        if(f->fdef==4)
        {
            while( (((long)ftab8)&3) != 0 ) ftab8++;  // align 32 bits
            f->Ttra=ftab8-f8;
            tra32=(uint32_t *)ftab8;
            for(a1=0;a1<4;a1++)
              for(a2=0;a2<4;a2++)
                for(a3=0;a3<4;a3++)
                  for(a4=0;a4<4;a4++)
                    tra32[a1+4*a2+16*a3+64*a4] =
                       a1+256*a2+65536*a3+16777216*a4;
            ftab8+=256*4;
        }
        if(f->fdef==3)
        {
            while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
            f->Ttra=ftab8-f8;
            tra64=(uint64_t *)ftab8;
            for(a1=0;a1<3;a1++)
              for(a2=0;a2<3;a2++)
                for(a3=0;a3<3;a3++)
                  for(a4=0;a4<3;a4++)
                    for(a5=0;a5<3;a5++)
                      tra64[a1+3*a2+9*a3+27*a4+81*a5] =
                         a1+(a2<<8)+(a3<<16)+(a4<<24)+(a5<<32);
            ftab8+=243*8;
        }
        if(f->fdef==2)
        {
            while( (((long)ftab8)&7) != 0 ) ftab8++;  // align 64 bits
            f->Ttra=ftab8-f8;
            tra64=(uint64_t *)ftab8;
            for(a1=0;a1<2;a1++)
              for(a2=0;a2<2;a2++)
                for(a3=0;a3<2;a3++)
                  for(a4=0;a4<2;a4++)
                    for(a5=0;a5<2;a5++)
                      for(a6=0;a6<2;a6++)
                        for(a7=0;a7<2;a7++)
                          for(a8=0;a8<2;a8++)
                      tra64[a1+2*a2+4*a3+8*a4+16*a5+32*a6+64*a7+128*a8] =
                               a1+(a2<<8) +(a3<<16)+(a4<<24)+
                         (a5<<32)+(a6<<40)+(a7<<48)+(a8<<56);
           ftab8+=256*8;
        }
    }
    f->hwm=ftab8-f8;
    return 1;
}

void FieldASet(uint64_t fdef, FIELD * f)
{
    int res;
    res=FieldASet1(fdef,f,0);
    (void)res;
}



void DSSet(const FIELD * f, uint64_t noc, DSPACE * ds)
{
    ds->f=f;
    ds->noc=noc;

    switch(f->paktyp)
    {
      case 0:  ds->nob=noc*8;         break;
      case 1:  ds->nob=noc*4;         break;
      case 2:  ds->nob=noc*2;         break;
      case 3:  ds->nob=noc;           break;
      case 4:  ds->nob=(noc+1)/2;     break;
      case 5:  ds->nob=(noc+2)/3;     break;
      case 6:  ds->nob=(noc+3)/4;     break;
      case 7:  ds->nob=(noc+4)/5;     break;
      case 8:  ds->nob=(noc+7)/8;     break;
    }
    ds->ground=0;
}

void PSSet(const FIELD * f, uint64_t noc, DSPACE * ds)
{
    ds->f=f;
    ds->noc=noc;
    ds->nob=noc*8;
    if(f->charc<4294967296) ds->nob=noc*4;
    if(f->charc<65536) ds->nob=noc*2;
    if(f->charc<256) ds->nob=noc;
    if(f->charc<16) ds->nob=(noc+1)/2;
    if(f->charc==5) ds->nob=(noc+2)/3;
    if(f->charc==3) ds->nob=(noc+4)/5;
    if(f->charc==2) ds->nob=(noc+7)/8;
    ds->ground=1;
}

FELT FieldAdd(const FIELD * f, FELT a, FELT b)
{
    uint64_t x64, pb1, pb2, pb3, qb1, qb2, qb3, rb1, rb2, rb3;
    switch ( f->addtyp )
    {
      case 1:
        return a^b;
      case 2:
        return f->add8[a*256 + b];
      case 3:
      case 4:
        return f->red16[a+b];
      case 5:
        return f->sqid16[f->spac16[a]+f->spac16[b]];
      case 6:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->red16[pb1+qb1];
        rb2=f->red16[pb2+qb2];
        return rb1*f->digit+rb2;
      case 7:
      case 9:
        x64=a+b;
/* find a way to speed up the next line  */
        if( (x64 < a) ||(x64 >= f->fdef) ) x64-=f->fdef;
        return x64;
      case 8:
      case 10:
        return qadd(f,a,b);
      case 11:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->sqid16[f->spac16[pb1]+f->spac16[qb1]];
        rb2=f->sqid16[f->spac16[pb2]+f->spac16[qb2]];
        return rb1*f->digit+rb2;
      case 12:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->sqid16[f->spac16[pb1]+f->spac16[qb1]];
        rb2=f->sqid16[f->spac16[pb2]+f->spac16[qb2]];
        return rb1*f->digit+rb2;
      case 13:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->red16[pb1+qb1];
        rb2=f->red16[pb2+qb2];
        return rb1*f->digit+rb2;
      case 14:
        pb2=(a*f->bar41)>>41;
        pb3=a-(pb2*f->digit);
        pb1=(pb2*f->bar41)>>41;
        pb2=pb2-(pb1*f->digit);
        qb2=(b*f->bar41)>>41;
        qb3=b-(qb2*f->digit); 
        qb1=(qb2*f->bar41)>>41;
        qb2=qb2-(qb1*f->digit);
        rb1=f->red16[pb1+qb1];
        rb2=f->red16[pb2+qb2];
        rb3=f->red16[pb3+qb3];
        return rb1*f->digit2+rb2*f->digit+rb3;
      default:
        printf("Internal error in FieldAdd - type not set\n");
        exit(42);
    }
}

FELT FieldNeg(const FIELD * f, FELT a)
{
    uint64_t pb1,pb2,pb3;
    switch ( f->addtyp )
    {
      case 1:
        return a;
      case 2:
        return f->mul8[(f->charc-1)*256 + a];
      case 3:
      case 4:
        return f->red16[f->fdef-a];
      case 5:
        return f->sqid16[f->spacneg - f->spac16[a]];
      case 6:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        pb1=f->red16[f->charc-pb1];
        pb2=f->red16[f->charc-pb2];
        return pb1*f->digit+pb2;
      case 7:
      case 9:
        if(a==0) return 0;
        return f->fdef-a;
      case 8:
      case 10:
        return qneg(f,a);
      case 11:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit); 
        pb1=f->sqid16[f->spacneg-f->spac16[pb1]];
        pb2=f->sqid16[f->spacneg-f->spac16[pb2]];
        return pb1*f->digit+pb2;
      case 12:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit); 
        pb1=f->sqid16[f->spacneg - f->spac16[pb1]];
        pb2=f->sqid16[f->spacneg - f->spac16[pb2]];
        return pb1*f->digit+pb2;
      case 13:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        pb1=f->red16[f->charc-pb1];
        pb2=f->red16[f->charc-pb2];
        return pb1*f->digit+pb2;
      case 14:
        return qneg(f,a);
//  what is going on here???!!!
        pb2=(a*f->bar41)>>41;
        pb3=a-(pb2*f->digit);
        pb1=(pb2*f->bar41)>>41;
        pb2=pb2-(pb1*f->digit);
        pb1=f->red16[f->charc-pb1];
        pb2=f->red16[f->charc-pb2];
        pb3=f->red16[f->charc-pb3];
        return pb1*f->digit2+pb2*f->digit+pb3;
      default:
        printf("Internal error in FieldNeg - type not set\n");
        exit(49);
    }
}

FELT FieldSub(const FIELD * f, FELT a, FELT b)
{
    uint64_t pb1,pb2,pb3,qb1,qb2,qb3,rb1,rb2,rb3;
    switch ( f->addtyp )
    {
      case 1:
        return a^b;
      case 2:
        return f->sub8[a*256 + b];
      case 3:
      case 4:
        return f->red16[f->fdef+a-b];;
      case 5:
        return f->sqid16[f->spaczero+f->spac16[a]-f->spac16[b]];
      case 6:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->red16[f->charc+pb1-qb1];
        rb2=f->red16[f->charc+pb2-qb2];
        return rb1*f->digit+rb2;
      case 7:
      case 9:
        if(a>=b) return a-b;
        else return f->fdef+a-b;
      case 8:
      case 10:
        return qsub(f,a,b);
      case 11:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->sqid16[f->spaczero+f->spac16[pb1]-f->spac16[qb1]];
        rb2=f->sqid16[f->spaczero+f->spac16[pb2]-f->spac16[qb2]];
        return rb1*f->digit+rb2;
      case 12:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit);
        rb1=f->sqid16[f->spaczero+f->spac16[pb1]-f->spac16[qb1]]; 
        rb2=f->sqid16[f->spaczero+f->spac16[pb2]-f->spac16[qb2]]; 
        return rb1*f->digit+rb2;
      case 13:
        pb1=(a*f->bar41)>>41;
        pb2=a-(pb1*f->digit);
        qb1=(b*f->bar41)>>41;
        qb2=b-(qb1*f->digit); 
        rb1=f->red16[f->charc+pb1-qb1];
        rb2=f->red16[f->charc+pb2-qb2];
        return rb1*f->digit+rb2;
      case 14:
        pb2=(a*f->bar41)>>41;
        pb3=a-(pb2*f->digit);
        pb1=(pb2*f->bar41)>>41;
        pb2=pb2-(pb1*f->digit);
        qb2=(b*f->bar41)>>41;
        qb3=b-(qb2*f->digit); 
        qb1=(qb2*f->bar41)>>41;
        qb2=qb2-(qb1*f->digit);
        rb1=f->red16[f->charc+pb1-qb1];
        rb2=f->red16[f->charc+pb2-qb2];
        rb3=f->red16[f->charc+pb3-qb3];     
        return rb1*f->digit2+rb2*f->digit+rb3;
      default:
        printf("Internal error in FieldSub - type not set\n");
        exit(42);
    }
}

FELT FieldMul(const FIELD * f, FELT a, FELT b)
{
    uint32_t a32,b32;
    if( (a==0) || (b==0) ) return 0;
    switch ( f->multyp )
    {
      case 1:
        return f->mul8[a*256+b];
      case 2:
        a32=a;
        b32=b;
        return (a32*b32)%f->p32;
      case 3:
      case 4:
        return f->alog16[f->log16[a]+f->log16[b]];
      case 5:
        return (a*b)%f->p32;
      case 6:
      case 7:
      case 8:
      case 9:
        return qmul(f,a,b);
      default:
        printf("Internal error in FieldMul - type not set\n");
        exit(40);
    }
}

FELT FieldInv(const FIELD * f, FELT a)
{
    if(a<=1) return a;
    switch ( f->multyp )
    {
      case 1:
        return f->inv8[a];
      case 2:
      case 5:
        return pinv(f,a);
      case 6:
      case 7:
      case 8:
      case 9:
        return qinv(f,a);
      case 3:
      case 4:
        return f->alog16[f->qminus1-f->log16[a]];
      default:
        printf("Internal error in FieldInv - type not set\n");
        exit(42);
    }

}

FELT FieldDiv(const FIELD * f, FELT a, FELT b)
{
    uint32_t a32,b32;
    if( (a==0) || (b==0) ) return 0;
    switch ( f->multyp )
    {
      case 1:
        return f->mul8[a*256+f->inv8[b]];
      case 2:
        a32=a;
        b32=pinv(f,b);
        return (a32*b32)%f->p32;
      case 3:
        return f->alog16[f->log16[a]+f->qminus1-f->log16[b]];
      case 4:
        a32=f->log16[a]+f->qminus1-f->log16[b];
        return f->alog16[a32];
      case 5:
        return (a*pinv(f,b))%f->p32;
      case 6:
      case 7:
      case 8:
      case 9:
        return qdiv(f,a,b);
      default:
        printf("Internal error in FieldDiv - type not set\n");
        exit(44);
    }
}

void DPak(const DSPACE * ds, uint64_t col, Dfmt * d, FELT a)
{
    uint8_t * pv;
    uint8_t g;
    int paktyp,s;
    uint64_t fdef;
    if(ds->ground==0)
    {
        paktyp=(ds->f)->paktyp;
        fdef=(ds->f)->fdef;
    }
         else     
    {
        paktyp=(ds->f)->ppaktyp;
        fdef=(ds->f)->charc;
    }
    switch(paktyp)
    {
      case 0:
        *(((uint64_t *)d)+col)=a;
        break;
      case 1:
        *(((uint32_t *)d)+col)=a;
        break;
      case 2:
        *(((uint16_t *)d)+col)=a;
        break;
      case 3:
        *((uint8_t *)d+col)=a;
        break;
      case 4:
        pv=(uint8_t *)d+(col/2);
        g=*pv;
        switch(col%2)
        {
          case 0:
            if(fdef==7) *pv=a+(g/7)*7;
            if(fdef==11) *pv=a+(g/11)*11;
            if(fdef==13) *pv=a+(g/13)*13;
            if(fdef==8) *pv=a+(g/8)*8;
            if(fdef==16) *pv=a+(g/16)*16;
            if(fdef==9) *pv=a+(g/9)*9;
            break;
          case 1:
            if(fdef==7) *pv=7*a+g%7;
            if(fdef==11) *pv=11*a+g%11;
            if(fdef==13) *pv=13*a+g%13;
            if(fdef==8) *pv=8*a+g%8;
            if(fdef==16) *pv=16*a+g%16;
            if(fdef==9) *pv=9*a+g%9;
            break;
        }
        break;
      case 5:
        pv=(uint8_t *)d+(col/3);
        g=*pv;
        switch(col%3)
        {
          case 0:
            *pv=(g/5)*5+a;
            break;
          case 1:
            *pv=a*5+g%5+(g/25)*25;
            break;
          case 2:
            *pv=a*25+g%25;
            break;
        }
        break;
      case 6:
        s=(col&3)<<1;
        pv=(uint8_t *)d+(col>>2);
        (*pv)=((*pv)&(255^(3<<s)))+(a<<s);
        break;
      case 7:
        pv=(uint8_t *)d+(col/5);
        g=*pv;
        switch(col%5)
        {
          case 0:
            *pv=a+((g/3)*3);
            break;
          case 1:
            *pv=(a*3)+(g%3)+((g/9)*9);
            break;
          case 2:
            *pv=(a*9)+(g%9)+((g/27)*27);
            break;
          case 3:
            *pv=(a*27)+(g%27)+((g/81)*81);
            break;
          case 4:
            *pv=(a*81)+(g%81);
            break;
        }
        break;
      case 8:
        s=col&7;
        pv=(uint8_t *)d+(col/8);
        (*pv)=((*pv)&(255^(1<<s)))+(a<<s);
        break;
    }
    return;
}

FELT DUnpak(const DSPACE * ds, uint64_t col, const Dfmt * d)
{
    FELT x = 0;
    int pwp5[3]={25,5,1};
    int pwp3[5]={81,27,9,3,1};
    uint8_t e;
    int paktyp;
    uint64_t fdef;
    if(ds->ground==0)
    {
        paktyp=(ds->f)->paktyp;
        fdef=(ds->f)->fdef;
    }
         else     
    {
        paktyp=(ds->f)->ppaktyp;
        fdef=(ds->f)->charc;
    }

    switch(paktyp)
    {
      case 0:
        x=*(((uint64_t *)d)+col);
        break;
      case 1:
        x=*(((uint32_t *)d)+col);
        break;
      case 2:
        x=*(((uint16_t *)d)+col);
        break;
      case 3:
        x=*((uint8_t *)d+col);
        break;
      case 4:
        e=*((uint8_t *)d+(col/2));
        switch(col%2)
        {
          case 0:
            if(fdef==7) x=e%7;
            if(fdef==11) x=e%11;
            if(fdef==13) x=e%13;
            if(fdef==8) x=e%8;
            if(fdef==16) x=e%16;
            if(fdef==9) x=e%9;
            break;
          case 1:
            if(fdef==7) x=e/7;
            if(fdef==11) x=e/11;
            if(fdef==13) x=e/13;
            if(fdef==8) x=e/8;
            if(fdef==16) x=e/16;
            if(fdef==9) x=e/9;
            break;
        }
        break;
      case 5:
        x=*((uint8_t *)d+(col/3));
        x=((x*pwp5[col%3])/25)%5;
        break;
      case 6:
        e=*((uint8_t *)d+(col>>2));
        return (e>>(2*(col&3)))&3;
      case 7:
        x=*((uint8_t *)d+col/5);
        x=((x*pwp3[col%5])/81)%3;
        break;
      case 8:
        e=*((uint8_t *)d+(col>>3));
        return (e>>(col&7))&1;
      default:
        break;
    }
    return x;
}

uint64_t DNzl(const DSPACE * ds, const Dfmt * d)
{
    const FIELD * f;
    uint64_t i;
    const uint8_t * d1;
    int paktyp;
    f=ds->f;
    if(ds->ground==0) paktyp=f->paktyp;
          else        paktyp=f->ppaktyp;
    d1=(uint8_t *) d;
    for(i=0;i<ds->nob;i++) if(d1[i]!=0) break;
    if(i==ds->nob) return ZEROROW;
    switch (paktyp)
    {
      case 0:
        return i>>3;
      case 1:
        return i>>2;
      case 2:
        return i>>1;
      case 3:
        return i;
      case 4:
        i<<=1;
        while(DUnpak(ds,i,d)==0) i++;
        return i;
      case 5:
        i*=3;
        while(DUnpak(ds,i,d)==0) i++;
        return i;
      case 6:
        i<<=2;
        while(DUnpak(ds,i,d)==0) i++;
        return i;
      case 7:
        i*=5;
        while(DUnpak(ds,i,d)==0) i++;
        return i;
      case 8:
        i<<=3;
        while(DUnpak(ds,i,d)==0) i++;
        return i;
      default:
        printf("Internal error in DNzl - Paktyp not set\n");
        exit(41);
    }
}

void DCut(const DSPACE *ms, uint64_t nor, uint64_t col,
          const Dfmt *m, const DSPACE *cbs, Dfmt * cb)
{
    uint64_t gshft;               /* early shift required  */
    uint64_t movecols;            /* number of columns of input moved */
    uint64_t outbyt;              /* output bytes affected by input */
    uint64_t clearbytes;          /* the rest of the output bytes */
    uint64_t r,s;
    uint64_t lbcbits;             /* bits output to last byte */
    uint64_t lbmbits;
    uint8_t *cbp, *mp;
    const uint8_t *ses, *sls, *ges, *gls;
    const FIELD *f = ms->f;
    int uptoend;       /* flag set if last col of m used */
    uptoend=1;
/* set movecols to be the number of columns to move  */
    movecols=ms->noc-col;
    if(movecols>=cbs->noc)
    {
        uptoend=0;
        movecols=cbs->noc;
    }

/*  Input is col, movecols, the.rest */
/*  Output is     movecols, 000      */

/* outbyte is movecols in bytes, rounded up */
    outbyt=((movecols+f->entbyte-1)/f->entbyte)*f->bytesper;
/* clearbytes is complete bytes of zeros  */
    clearbytes=cbs->nob-outbyt;
    gshft = col%f->entbyte;       /* early shift required */
    lbcbits = movecols%f->entbyte; /* bits in last output byte */
/* get pointers to first byte to deal with */
    cbp = (uint8_t *)cb;
    mp = (uint8_t *)m;
    mp+=(col/f->entbyte)*f->bytesper;  /* point to first input byte */

/* first major case - no shift required  */

    if (gshft==0)                   /* bytes are in registration */
    {
/* three cases - some to zeroize, no partial last byte, partial last byte */

/* some to zeroize */
        if (clearbytes!=0)
        {
/* Cut Case 1 ---- */
            for (r=0; r<nor; r++)
            {
                memcpy(cbp, mp, outbyt);
                memset(cbp+outbyt,0,clearbytes);
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
        if ( (lbcbits==0) || (uptoend==1) )
        {
/* Cut Case 2 ---- */
            for (r = 0; r < nor; r++)
            {
                memcpy(cbp, mp, outbyt);
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
        sls = &(f->late8[0])  + f->dbytevals * (f->entbyte-lbcbits-1);
        ses = (&f->early8[0]) + f->dbytevals * (f->entbyte-lbcbits-1);
        if(outbyt>1)
        {
/* Cut Case 3 ---- */
            for (r = 0; r < nor; r++)
            {
                memcpy(cbp, mp, outbyt - 1);
                cbp[outbyt - 1] = ses[sls[mp[outbyt - 1]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Cut Case 4 ---- */
        for (r = 0; r < nor; r++)
        {
            *cbp = ses[sls[*mp]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }

    lbmbits = (col+movecols)%f->entbyte;  /* bits in last input byte */
/* otherwise generic shift needed  */
    ges = &(f->early8[0]) + f->dbytevals * (gshft - 1);
    gls = &f->late8[0] + f->dbytevals * (f->entbyte - gshft - 1);

/* Note, lbmbits and lbcbits cannot be equal - not aligned */
/* cases lbmbits==0, lbcbits==0 lbmbits bigger, lbcbits bigger */
    if (lbmbits==0)
    {
        if (clearbytes==0)
        {
/* Cut Case 5 ---- */
            for (r = 0; r < nor; r++)
            {
                for (s = 0; s + 1 < outbyt; s++)
                    cbp[s] = ges[mp[s]] + gls[mp[s+1]];
                cbp[outbyt - 1] = ges[mp[outbyt - 1]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Cut Case 6 ---- */
        for (r = 0; r < nor; r++)
        {
            for (s = 0; s + 1 < outbyt; s++)
                cbp[s] = ges[mp[s]] + gls[mp[s+1]];
            cbp[outbyt - 1] = ges[mp[outbyt - 1]];
            memset(cbp + outbyt, 0, clearbytes);
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if (lbcbits==0)
    {
        if (clearbytes==0)
        {
/* Cut Case 7 ---- */
            for (r = 0; r < nor; r++)
            {
                for (s = 0; s < outbyt; s++)
                    cbp[s] = ges[mp[s]] + gls[mp[s+1]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Cut Case 8 ---- */
        for (r = 0; r < nor; r++)
        {
            for (s = 0; s < outbyt; s++)
                cbp[s] = ges[mp[s]] + gls[mp[s+1]];
            memset(cbp + outbyt, 0, clearbytes);
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
/* sls puts the last input bit at the end of the byte */
    sls=&f->late8[0]+f->dbytevals*(f->entbyte-lbmbits-1);
/* ses puts the last bit of a byte to the last wanted position */
    ses=&(f->early8[0])+f->dbytevals*(f->entbyte-lbcbits-1);
    if (lbmbits > lbcbits)
    {
        if (clearbytes==0)
        {
/* Cut Case 9 ---- */
            for (r = 0; r < nor; r++)
            {
                for (s = 0; s+1 < outbyt; s++)
                    cbp[s] = ges[mp[s]] + gls[mp[s+1]];
                cbp[outbyt-1]=ses[sls[mp[outbyt-1]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Cut Case 10 ---- */
        for (r = 0; r < nor; r++)
        {
            for (s = 0; s+1 < outbyt; s++)
                cbp[s] = ges[mp[s]] + gls[mp[s+1]];
            cbp[outbyt-1]=ses[sls[mp[outbyt-1]]];
            memset(cbp + outbyt, 0, clearbytes);
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if (clearbytes==0)
    {
/* Cut Case 11 ---- */
        for (r = 0; r < nor; r++)
        {
            for (s = 0; s+1 < outbyt; s++)
                cbp[s] = ges[mp[s]] + gls[mp[s+1]];
            cbp[outbyt-1]=ges[mp[outbyt-1]]+ses[sls[mp[outbyt]]];
            cbp += cbs->nob;
             mp += ms->nob;
        }
        return;
    }
/* Cut Case 12 ---- */
    for (r = 0; r < nor; r++)
    {
        for (s = 0; s+1 < outbyt; s++)
            cbp[s] = ges[mp[s]] + gls[mp[s+1]];
        cbp[outbyt-1]=ges[mp[outbyt-1]]+ses[sls[mp[outbyt]]];
        memset(cbp + outbyt, 0, clearbytes);
        cbp += cbs->nob;
        mp += ms->nob;
    }
    return;
}

void DPaste(const DSPACE *cbs, const Dfmt * cb, uint64_t nor, uint64_t col,
          const DSPACE *ms, Dfmt * m)
{
    const FIELD *f = ms->f;
    uint64_t gshft;               /* early shift required  */
    uint64_t movecols;            /* number of columns of input moved */
    uint64_t lbcbits;             /* bits input from last byte */
    uint64_t lbmbits;             /* bits output to last byte */
    uint64_t inbyt;               /* input bytes used */
    uint8_t *cbp, *mp;
    const uint8_t *me,*ml,*ce,*cl;  /* shifters for removing junk */
    int wlong;                    /* 1 m longer, 2 = , 3 cb longer */
    const uint8_t *ges, *gls;     /* general early and late shifts */
    const uint8_t *fls, *fes;     /* first byte late and early shifts */
    uint64_t r,s;                 /* index for counting rows    */
    if(cbs->noc==0) return;     /* if no cols, nothing to do */
    gshft = col%f->entbyte;     /* late shift required  */

/* set movecols to be the number of columns to move  */
    wlong=3;
    movecols=ms->noc-col;
    if(movecols==cbs->noc) wlong=2;
    else if(movecols>cbs->noc)
    {
        wlong=1;
        movecols=cbs->noc;
    }
    lbcbits = movecols%f->entbyte; /* bits in last input byte */
/* inbyte is movecols in bytes, rounded up */
    inbyt=((movecols+f->entbyte-1)/f->entbyte)*f->bytesper;
/* get pointers to first byte to deal with */
    cbp = (uint8_t *)cb;
    mp = (uint8_t *)m;
    mp+=(col/f->entbyte)*f->bytesper;  /* point to first output byte */

/* first major case - no shift required  */

    if (gshft==0)                   /* bytes are in registration */
    {
        if( (lbcbits==0) || (wlong==2) )
        {
/* Paste Case 1 ---- */
            for (r=0; r<nor; r++)
            {
                memcpy(mp, cbp, inbyt);
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
        if(wlong==1)
        {
/* Paste Case 2 ---- */
            ml=&f->late8[0]  + f->dbytevals*(lbcbits-1);
            me=&(f->early8[0]) + f->dbytevals*(lbcbits-1);
            for (r=0; r<nor; r++)
            {
                memcpy(mp, cbp, inbyt-1);
                mp[inbyt-1]=cbp[inbyt-1]+ml[me[mp[inbyt-1]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
        if(wlong==3)
        {
/* Paste Case 3 ---- */
            ce=&(f->early8[0]) + f->dbytevals*(f->entbyte-lbcbits-1);
            cl=&f->late8[0]  + f->dbytevals*(f->entbyte-lbcbits-1);
            for (r=0; r<nor; r++)
            {
                memcpy(mp, cbp, inbyt-1);
                mp[inbyt-1]=ce[cl[cbp[inbyt-1]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
    }

/* otherwise generic shift needed  */
    gls = &f->late8[0]  + f->dbytevals * (gshft - 1);
    fes = &(f->early8[0]) + f->dbytevals * (gshft - 1);
    ges = &(f->early8[0]) + f->dbytevals * (f->entbyte - gshft - 1);
    fls = &f->late8[0]  + f->dbytevals * (f->entbyte - gshft - 1);

/* Note, lbmbits and lbcbits cannot be equal - not aligned */
/* cases lbmbits==0, lbcbits==0 lbmbits bigger, lbcbits bigger */
    lbmbits = (col+movecols)%f->entbyte; /* bits in last output byte */
    if(lbmbits==0)
    {
/* Paste Case 4 ---- */
        for (r = 0; r < nor; r++)
        {
            mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
            for (s = 1; s < inbyt; s++)
                mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if(lbcbits==0)
    {
        if(wlong==1)
        {
/* Paste Case 5 ---- */
            for (r = 0; r < nor; r++)
            {
                mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
                for (s = 1; s < inbyt; s++)
                    mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
                mp[inbyt]=ges[cbp[inbyt-1]] + gls[fes[mp[inbyt]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Paste Case 6 ---- */
        for (r = 0; r < nor; r++)
        {
            mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
            for (s = 1; s < inbyt; s++)
                mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
            mp[inbyt] = ges[cbp[inbyt-1]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if(lbmbits<lbcbits)
    {
        if(wlong==1)
        {
/* Paste Case 7 ---- */
            ml=&f->late8[0]  + f->dbytevals*(lbmbits-1);
            me=&(f->early8[0]) + f->dbytevals*(lbmbits-1);
            for (r = 0; r < nor; r++)
            {
                mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
                for (s = 1; s < inbyt; s++)
                    mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
                mp[inbyt] = ges[cbp[inbyt-1]] + ml[me[mp[inbyt]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
        if(wlong==2)
        {
/* Paste Case 8 ---- */
            for (r = 0; r < nor; r++)
            {
                mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
                for (s = 1; s < inbyt; s++)
                    mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
                mp[inbyt] = ges[cbp[inbyt-1]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Paste Case 9 ---- */
        ce=&(f->early8[0]) + f->dbytevals*(f->entbyte-lbmbits-1);
        cl=&f->late8[0]  + f->dbytevals*(f->entbyte-lbcbits-1);
        for (r = 0; r < nor; r++)
        {
            mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
            for (s = 1; s < inbyt; s++)
                mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
            mp[inbyt] = ce[cl[cbp[inbyt-1]]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if(wlong==1)
    {
        if(inbyt==1)
        {
/* Paste Case 10 ---- */
            ml=&f->late8[0]  + f->dbytevals*(lbmbits-1);
            me=&(f->early8[0]) + f->dbytevals*(lbmbits-1);
            for (r = 0; r < nor; r++)
            {
                mp[0] = ges[fls[mp[0]]] + gls[cbp[0]]
                      + ml[me[mp[0]]];
                cbp += cbs->nob;
                mp += ms->nob;
            }
            return;
        }
/* Paste Case 11 ---- */
        ml=&f->late8[0]  + f->dbytevals*(lbmbits-1);
        me=&(f->early8[0]) + f->dbytevals*(lbmbits-1);
        for (r = 0; r < nor; r++)
        {
            mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
            for (s = 1; s + 1 < inbyt; s++)
                mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
            mp[inbyt-1] = ges[cbp[inbyt-2]] + gls[cbp[inbyt-1]]
                  + ml[me[mp[inbyt-1]]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    if(wlong==2)
    {
/* Paste Case 12 ---- */
        for (r = 0; r < nor; r++)
        {
            mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
            for (s = 1; s < inbyt; s++)
                mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
            cbp += cbs->nob;
            mp += ms->nob;
        }
        return;
    }
    ce=&(f->early8[0]) + f->dbytevals*(f->entbyte-lbmbits-1);
    cl=&f->late8[0]  + f->dbytevals*(f->entbyte-lbcbits-1);
    if(inbyt==1)
    {
/* Paste Case 13  */
         for (r = 0; r < nor; r++)
        {
               mp[0]=ges[fls[mp[0]]]+ce[cl[cbp[0]]];
               cbp += cbs->nob;
               mp += ms->nob;
        }
        return;
    }
/* Paste Case 14 ---- */
    for (r = 0; r < nor; r++)
    {
        mp[0]=ges[fls[mp[0]]]+gls[cbp[0]];
        for (s = 1; s + 1 < inbyt; s++)
            mp[s] = ges[cbp[s-1]] + gls[cbp[s]];
        mp[inbyt-1] = ges[cbp[inbyt-2]] +ce[cl[cbp[inbyt-1]]];
        cbp += cbs->nob;
        mp += ms->nob;
    }
    return;
}

/* Point into d format row nor rows later */
Dfmt * DPAdv(const DSPACE * ds, uint64_t nor, const Dfmt * d)
{
    return (Dfmt *) (((char *) d)+nor*ds->nob);
}

/* Point into d format row 1 row later */
Dfmt * DPInc(const DSPACE * ds, const Dfmt * d)
{
    return (Dfmt *) (((char *) d)+ds->nob);
}

void TAdd(const DSPACE * ds, uint64_t nor, const Dfmt *a, uint64_t astride,
                  const Dfmt *b, uint64_t bstride, Dfmt *c, uint64_t cstride)
{
    const FIELD * f;
    int addtyp,ast,bst,cst;
    uint64_t *a64, *b64, *c64, *ap64, *bp64, *cp64;
    uint32_t *a32, *b32, *c32, *ap32, *bp32, *cp32;
    uint16_t *a16, *b16, *c16, *ap16, *bp16, *cp16;
    const uint8_t *a8,*b8,*ap8,*bp8;
    uint8_t *c8,*cp8;
    uint64_t x64,y64,z64;
    uint32_t x32;
    uint64_t xb1,xb2,xb3,yb1,yb2,yb3,zb1,zb2,zb3;
    uint64_t r,col,rw;

    f=ds->f;
    if(ds->ground==0)
        addtyp=f->addtyp;
    else     
        addtyp=f->paddtyp;
    switch ( addtyp )
    {

// Characteristic 2

      case 1:
        a8=a;
        b8=b;
        c8=c;
        if( (f->mact[0]=='j')||(f->mact[0]>='l') ) // AVX2?
        {
            for(r=0;r<nor;r++)
            {
                pc1xorj(c8,a8,b8,ds->nob);  // AVX2
                a8+=astride;
                b8+=bstride;
                c8+=cstride;
            }
            return;
        }
        for(r=0;r<nor;r++)
        {
            pc1xora(c8,a8,b8,ds->nob);  // SSE
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;

// 8-bit cases

      case 2:
        a8=a;
        b8=b;
        c8=c;
        for(r=0;r<nor;r++)
        {
            pcbif(c8,a8,b8,ds->nob,f->add8);
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;
      case 3:
        a8=a;
        b8=b;
        c8=c;
        y64 = ds->nob;
        x64 = y64>>2;    // words
        y64 = y64&3;     // remaining bytes
        for(r=0;r<nor;r++)
        {
            ap8=a8;
            bp8=b8;
            cp8=c8;
            for(rw=0;rw<x64;rw++)
            {
                x32=*ap8;
                *cp8    =f->red16[x32 + (*bp8)];
                x32=*(ap8+1);
                *(cp8+1)=f->red16[x32 + (*(bp8+1))];
                x32=*(ap8+2);
                *(cp8+2)=f->red16[x32 + (*(bp8+2))];
                x32=*(ap8+3);
                *(cp8+3)=f->red16[x32 + (*(bp8+3))];
                ap8+=4;
                bp8+=4;
                cp8+=4;
            }
            for(rw=0;rw<y64;rw++)
            {
                x32=*(ap8++);
                *(cp8++) = f->red16[x32+*(bp8++)];
            }
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;

// 16-bit cases

      case 4:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++);
                y64=*(bp16++);
                z64=x64+y64;
                *(cp16++) = f->red16[z64];
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;

        }
        return;
      case 5:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
      *(cp16++)=f->sqid16[f->spac16[*(ap16++)]+f->spac16[*(bp16++)]];
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;
      case 6:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++);
                y64=*(bp16++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->red16[xb1+yb1];
                zb2=f->red16[xb2+yb2];
                *(cp16++) = zb1*f->digit+zb2;
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;
      case 12:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++);
                y64=*(bp16++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->sqid16[f->spac16[xb1]+f->spac16[yb1]];
                zb2=f->sqid16[f->spac16[xb2]+f->spac16[yb2]];
                *(cp16++) = zb1*f->digit+zb2;
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;

// 32-bit cases

      case 7:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                z64=x64+y64;
/* next line is supposed to compile to cmov */
                if(z64 >= f->charc) z64-=f->charc;
                *(cp32++) = z64;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 8:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
                *(cp32++)=qadd(f,*(ap32++),*(bp32++));
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 11:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->sqid16[f->spac16[xb1]+f->spac16[yb1]];
                zb2=f->sqid16[f->spac16[xb2]+f->spac16[yb2]];
                *(cp32++) = zb1*f->digit+zb2;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 13:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->red16[xb1+yb1];
                zb2=f->red16[xb2+yb2];
                *(cp32++) = zb1*f->digit+zb2;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 14:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb2=(x64*f->bar41)>>41;
                xb3=x64-(xb2*f->digit);
                xb1=(xb2*f->bar41)>>41;
                xb2=xb2-(xb1*f->digit);
                yb2=(y64*f->bar41)>>41;
                yb3=y64-(yb2*f->digit);
                yb1=(yb2*f->bar41)>>41;
                yb2=yb2-(yb1*f->digit);
                zb1=f->red16[xb1+yb1];
                zb2=f->red16[xb2+yb2];
                zb3=f->red16[xb3+yb3];
                *(cp32++) = zb1*f->digit2+zb2*f->digit+zb3;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;

// 64-bit cases

      case 9:
        a64 = (uint64_t *) a;
        b64 = (uint64_t *) b;
        c64 = (uint64_t *) c;
        ast=astride/8;
        bst=bstride/8;
        cst=cstride/8;
        for(r=0;r<nor;r++)
        {
            ap64=a64;
            bp64=b64;
            cp64=c64;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap64++);
                y64=*(bp64++);
                z64=x64+y64;
                if( (z64<x64) || (z64>=f->charc) ) z64-=f->charc;
                *(cp64++) = z64;
            }
            a64+=ast;
            b64+=bst;
            c64+=cst;
        }
        return;
      case 10:
        a64 = (uint64_t *) a;
        b64 = (uint64_t *) b;
        c64 = (uint64_t *) c;
        ast=astride/8;
        bst=bstride/8;
        cst=cstride/8;
        for(r=0;r<nor;r++)
        {
            ap64=a64;
            bp64=b64;
            cp64=c64;
            for(col=0;col<ds->noc;col++)
                *(cp64++) = qadd(f,*(ap64++),*(bp64++));
            a64+=ast;
            b64+=bst;
            c64+=cst;
        }
        return;
      default:
        printf("Internal error in TAdd - type not set\n");
        exit(41);
    }
}

/*   d = d1 + d2    */
void DAdd(const DSPACE * ds, uint64_t nor, 
                   const Dfmt * d1, const Dfmt * d2, Dfmt * d)
{
        TAdd(ds,nor,d1,ds->nob,d2,ds->nob,d,ds->nob);
}

void TSub(const DSPACE * ds, uint64_t nor, const Dfmt *a, uint64_t astride,
                  const Dfmt *b, uint64_t bstride, Dfmt *c, uint64_t cstride)
{
    const FIELD * f;
    int addtyp,ast,bst,cst;
    uint64_t *a64, *b64, *c64, *ap64, *bp64, *cp64;
    uint32_t *a32, *b32, *c32, *ap32, *bp32, *cp32;
    uint16_t *a16, *b16, *c16, *ap16, *bp16, *cp16;
    const uint8_t *a8,*b8,*ap8,*bp8;
    uint8_t *c8,*cp8;
    uint64_t x64,y64,z64;
    uint32_t x32;
    uint64_t xb1,xb2,xb3,yb1,yb2,yb3,zb1,zb2,zb3;
    uint64_t r,col,rw;

    f=ds->f;
    if(ds->ground==0)
        addtyp=f->addtyp;
    else     
        addtyp=f->paddtyp;
    switch ( addtyp )
    {

// Characteristic 2

      case 1:
        a8=a;
        b8=b;
        c8=c;
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            for(r=0;r<nor;r++)
            {
                pc1xorj(c8,a8,b8,ds->nob);  // AVX2
                a8+=astride;
                b8+=bstride;
                c8+=cstride;
            }
            return;
        }
        for(r=0;r<nor;r++)
        {
            pc1xora(c8,a8,b8,ds->nob);  // SSE
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;

// 8-bit cases

      case 2:
        a8=a;
        b8=b;
        c8=c;
        for(r=0;r<nor;r++)
        {
            pcbif(c8,a8,b8,ds->nob,f->sub8);
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;
      case 3:
        a8=a;
        b8=b;
        c8=c;
        y64 = ds->nob;
        x64 = y64>>2;    // words
        y64 = y64&3;     // remaining bytes
        for(r=0;r<nor;r++)
        {
            ap8=a8;
            bp8=b8;
            cp8=c8;
            for(rw=0;rw<x64;rw++)
            {
                x32=*ap8;
                *cp8    =f->red16[f->p32 + x32 - (*bp8)];
                x32=*(ap8+1);
                *(cp8+1)=f->red16[f->p32 + x32 - (*(bp8+1))];
                x32=*(ap8+2);
                *(cp8+2)=f->red16[f->p32 + x32 - (*(bp8+2))];
                x32=*(ap8+3);
                *(cp8+3)=f->red16[f->p32 + x32 - (*(bp8+3))];
                ap8+=4;
                bp8+=4;
                cp8+=4;
            }
            for(rw=0;rw<y64;rw++)
            {
                x32=*(ap8++);
                *(cp8++) = f->red16[f->p32 + x32 - *(bp8++)];
            }
            a8+=astride;
            b8+=bstride;
            c8+=cstride;
        }
        return;

// 16-bit cases

      case 4:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++)+f->p32;
                y64=*(bp16++);
                z64=x64-y64;
                *(cp16++) = f->red16[z64];
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;

        }
        return;
      case 5:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
                *(cp16++)=f->sqid16[f->spaczero+
                f->spac16[*(ap16++)]-f->spac16[*(bp16++)]];
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;
      case 6:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++);
                y64=*(bp16++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->red16[f->p32+xb1-yb1];
                zb2=f->red16[f->p32+xb2-yb2];
                *(cp16++) = zb1*f->digit+zb2;
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;
      case 12:
        a16 = (uint16_t *) a;
        b16 = (uint16_t *) b;
        c16 = (uint16_t *) c;
        ast=astride/2;
        bst=bstride/2;
        cst=cstride/2;
        for(r=0;r<nor;r++)
        {
            ap16=a16;
            bp16=b16;
            cp16=c16;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap16++);
                y64=*(bp16++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->sqid16[f->spaczero+f->spac16[xb1]-f->spac16[yb1]];
                zb2=f->sqid16[f->spaczero+f->spac16[xb2]-f->spac16[yb2]];
                *(cp16++) = zb1*f->digit+zb2;
            }
            a16+=ast;
            b16+=bst;
            c16+=cst;
        }
        return;

// 32-bit cases

      case 7:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++)+f->p32;
                y64=*(bp32++);
                z64=x64-y64;
/* next line is supposed to compile to cmov */
                if(z64 >= f->charc) z64-=f->charc;
                *(cp32++) = z64;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 8:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
                *(cp32++)=qsub(f,*(ap32++),*(bp32++));
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 11:;
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->sqid16[f->spaczero+f->spac16[xb1]-f->spac16[yb1]];
                zb2=f->sqid16[f->spaczero+f->spac16[xb2]-f->spac16[yb2]];
                *(cp32++) = zb1*f->digit+zb2;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 13:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb1=(x64*f->bar41)>>41;
                xb2=x64-(xb1*f->digit);
                yb1=(y64*f->bar41)>>41;
                yb2=y64-(yb1*f->digit); 
                zb1=f->red16[f->p32+xb1-yb1];
                zb2=f->red16[f->p32+xb2-yb2];
                *(cp32++) = zb1*f->digit+zb2;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;
      case 14:
        a32 = (uint32_t *) a;
        b32 = (uint32_t *) b;
        c32 = (uint32_t *) c;
        ast=astride/4;
        bst=bstride/4;
        cst=cstride/4;
        for(r=0;r<nor;r++)
        {
            ap32=a32;
            bp32=b32;
            cp32=c32;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap32++);
                y64=*(bp32++);
                xb2=(x64*f->bar41)>>41;
                xb3=x64-(xb2*f->digit);
                xb1=(xb2*f->bar41)>>41;
                xb2=xb2-(xb1*f->digit);
                yb2=(y64*f->bar41)>>41;
                yb3=y64-(yb2*f->digit);
                yb1=(yb2*f->bar41)>>41;
                yb2=yb2-(yb1*f->digit);
                zb1=f->red16[f->p32+xb1-yb1];
                zb2=f->red16[f->p32+xb2-yb2];
                zb3=f->red16[f->p32+xb3-yb3];
                *(cp32++) = zb1*f->digit2+zb2*f->digit+zb3;
            }
            a32+=ast;
            b32+=bst;
            c32+=cst;
        }
        return;

// 64-bit cases

      case 9:
        a64 = (uint64_t *) a;
        b64 = (uint64_t *) b;
        c64 = (uint64_t *) c;
        ast=astride/8;
        bst=bstride/8;
        cst=cstride/8;
        for(r=0;r<nor;r++)
        {
            ap64=a64;
            bp64=b64;
            cp64=c64;
            for(col=0;col<ds->noc;col++)
            {
                x64=*(ap64++);
                y64=*(bp64++);
                z64=x64-y64;
                if(z64>x64) z64+=f->charc;
                *(cp64++) = z64;
            }
            a64+=ast;
            b64+=bst;
            c64+=cst;
        }
        return;
      case 10:
        a64 = (uint64_t *) a;
        b64 = (uint64_t *) b;
        c64 = (uint64_t *) c;
        ast=astride/8;
        bst=bstride/8;
        cst=cstride/8;
        for(r=0;r<nor;r++)
        {
            ap64=a64;
            bp64=b64;
            cp64=c64;
            for(col=0;col<ds->noc;col++)
                *(cp64++) = qsub(f,*(ap64++),*(bp64++));
            a64+=ast;
            b64+=bst;
            c64+=cst;
        }
        return;
      default:
        printf("Internal error in TSub - type not set\n");
        exit(41);
    }
}

/*   d = d1 - d2    */
void DSub(const DSPACE * ds, uint64_t nor, 
                   const Dfmt * d1, const Dfmt * d2, Dfmt * d)
{
        TSub(ds,nor,d1,ds->nob,d2,ds->nob,d,ds->nob);
}

/*d2 += scalar*d1 */
void DSMad(const DSPACE * ds, FELT scalar, uint64_t nor, const Dfmt * d1, Dfmt * d2)
{
    const FIELD * f;
    unsigned int i;
    int madtyp;
    const uint8_t *pmul8;
    uint8_t  *p8,*q8;
    uint16_t *p16,*q16;
    uint32_t *p32,*q32;
    uint64_t *p64,*q64;
    uint32_t x32,y32,a32,z32;
    uint64_t x64,a64,b64;;
    f=ds->f;
    if(ds->ground==0) madtyp=f->madtyp;
           else       madtyp=f->pmadtyp;
    switch ( madtyp )
    {
      case 1:
        if(scalar==0) return;
        if(scalar==1)
        {
            DAdd(ds,nor,d1,d2,d2);
            return;
        }
        pmul8=&(f->mul8[scalar*256]);
        p8=(uint8_t *)d1;
        q8=(uint8_t *)d2;
        pcxunf(q8,p8,ds->nob*nor,pmul8);
        return;
      case 2:
        if(scalar==0) return;
        if(scalar==1)
        {
            DAdd(ds,nor,d1,d2,d2);
            return;
        }
        if((scalar+1)==f->charc)
        {
            DSub(ds,nor,d2,d1,d2);
            return;
        }
        pmul8=&(f->mul8[scalar*256]);
        p8=(uint8_t *)d1;
        q8=(uint8_t *)d2;
        pcbunf(q8,p8,ds->nob*nor,pmul8,f->add8);
        return;
      case 3:
printf("Currently not used\n");
exit(1);
// used to be 1-byte primes, but pcbunf (case 2) is faster
      case 4:
        p16=(uint16_t *)d1;
        q16=(uint16_t *)d2;
        a64=scalar;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=*(p16++);
            b64=*q16;
            x64=x64*a64+b64;
            b64=(x64*f->bar48)>>48;
            x64-=(b64*f->charc);
            *(q16++)=x64&65535;
        }
        return;
      case 5:
        if(scalar==0) return;
        p16 = (uint16_t *) d1;
        q16 = (uint16_t *) d2;
        a32=f->log16[scalar];
        z32=2*f->qminus1;
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*(p16++);
            if(x32==0) {
              q16++;
              continue;
            }
            y32=*q16;
            x32=f->log16[x32]+a32;
            if(y32==0)
            {
                *(q16++)=f->alog16[x32];
                continue;
            }
            y32=f->log16[y32];
            y32=f->zech16[z32+y32-x32];
            if(y32==f->qminus1)
            {
                *(q16++)=0;
                continue;
            }
            *(q16++)=f->alog16[x32+y32];
        }
        return;
      case 6:
        if(scalar==0) return;
        p16 = (uint16_t *) d1;
        q16 = (uint16_t *) d2;
        a32=f->log16[scalar];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*(p16++);
            if(x32==0) {
              q16++;
              continue;
            }
            x32=f->log16[x32]+a32;
            y32=*q16;
            *(q16++)=y32^f->alog16[x32];
        }
        return;
      case 7:
        if(scalar==0) return;
        p16 = (uint16_t *) d1;
        q16 = (uint16_t *) d2;
        a32=f->log16[scalar];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*(p16++);
            y32=*q16;
            if(x32==0) {
              q16++;
              continue;
            }
            x32=f->log16[x32]+a32;
            if(x32>=f->qminus1) x32-=f->qminus1;
            if(y32==0)
            {
                *(q16++)=f->alog16[x32];
                continue;
            }
            y32=f->log16[y32];
            y32=f->qminus1+y32-x32;
            if(y32>=f->qminus1) y32-=f->qminus1;
            y32=f->zech16[y32];
            if(y32==f->qminus1)
            {
                *(q16++)=0;
                continue;
            }
            x32=x32+y32;
            if(x32>=f->qminus1)x32-=f->qminus1;
            *(q16++)=f->alog16[x32];
        }
        return;
      case 8:
        if(scalar==0) return;;
        p16 = (uint16_t *) d1;
        q16 = (uint16_t *) d2;
        a32=f->log16[scalar];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*(p16++);
            if(x32==0) {
              q16++;
              continue;
            }
            x32=f->log16[x32]+a32;
            if(x32>=f->qminus1) x32-=f->qminus1;
            y32=*q16;
            *(q16++)=y32^f->alog16[x32];
        }
        return;
      case 9:
        p32=(uint32_t *)d1;
        q32=(uint32_t *)d2;
        a64=scalar;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=(*(p32++))*a64 + *q32;
            *(q32++)=x64%f->p32;
        }
        return;
      case 10:
        p32=(uint32_t *)d1;
        q32=(uint32_t *)d2;
        a64=scalar;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=qmul(f,*(p32++),a64);
            x64=qadd(f,x64,*q32);
            *(q32++)=x64;
        }
        return;
      case 11:
        p32=(uint32_t *)d1;
        q32=(uint32_t *)d2;
        if(f->mact[0]>='g')
        {
            pccl32(f->clpm,(scalar<<f->clpm[2]),ds->noc*nor,p32,q32);
            return;
        }
// should be case 14 really
        a64=scalar;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=qmul(f,*(p32++),a64)^*q32;
            *(q32++)=x64;
        }
        return;
      case 12:
        p64=(uint64_t *)d1;
        q64=(uint64_t *)d2;
        a64=scalar;
        if(f->pow!=1)
        {
            for(i=0;i<ds->noc*nor;i++)
            {
                x64=qmul(f,*(p64++),a64);
                x64=qadd(f,x64,*q64);
                *(q64++)=x64;
            }
            return;
        }
        for(i=0;i<ds->noc*nor;i++)
        {
            *q64=pcpmad(f->charc,*(p64++),a64,*q64);
            q64++;
        }
        return;
      case 13:
        p64=(uint64_t *)d1;
        q64=(uint64_t *)d2;
        if(f->mact[0]>='g')
        {
            pccl64(f->clpm,(scalar<<f->clpm[2]),ds->noc*nor,p64,q64);
            return;
        }
// should be case 15
        a64=scalar;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=qmul(f,*(p64++),a64)^*q64;
            *(q64++)=x64;
        }
        return;
      default:
        printf("Internal error in DSMad - type not set\n");
        exit(46);
    }
}

/* Scale d by a */
void DSMul(const DSPACE * ds, FELT a, uint64_t nor, Dfmt * d)
{
    const FIELD * f;
    unsigned int i;
    int multyp;
    const uint8_t *pmul8;
    uint8_t  *p8;
    uint16_t *p16;
    uint32_t *p32;
    uint64_t *p64;
    uint32_t a32,x32;
    uint64_t a64,x64,y64;
    f=ds->f;
    if(ds->ground==0) multyp=f->multyp;
            else      multyp=f->pmultyp;
    switch ( multyp )
    {
      case 1:
        if(a==0)
        {
            memset(d,0,ds->nob*nor);
            return;
        }
        if(a==1) return;
        pmul8=&(f->mul8[a*256]);
        p8=(uint8_t *)d;
        pcunf(p8,ds->nob*nor,pmul8);
        return;
      case 2:
        p16=(uint16_t *)d;
        a64=a;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=*p16;
            x64=x64*a64;
            y64=(x64*f->bar48)>>48;
            x64-=(y64*f->charc);
            *(p16++)=x64&65535;
        }
        return;
      case 3:
        if(a==0)
        {
            memset(d,0,ds->nob*nor);
            return;
        }
        p16 = (uint16_t *) d;
        a32=f->log16[a];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*p16;
            if(x32==0) {
              p16++;
              continue;
            }
            x32=f->log16[x32]+a32;
            *(p16++)=f->alog16[x32];
        }
        return;
      case 4:
        if(a==0)
        {
            memset(d,0,ds->nob*nor);
            return;
        }
        p16 = (uint16_t *) d;
        a32=f->log16[a];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=*p16;
            if(x32==0) {
              p16++;
              continue;
            }
            x32=f->log16[x32]+a32;
            if(x32>=f->qminus1) x32-=f->qminus1;
            *(p16++)=f->alog16[x32];
        }
        return;
      case 5:
        p32=(uint32_t *)d;
        a64=a;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=(*p32)*a64;
            *(p32++)=x64%f->p32;
        }
        return;
      case 6:
        p32=(uint32_t *)d;
        a64=a;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=qmul(f,*(p32),a64);
            *(p32++)=x64;
        }
        return;
      case 7:
        p64=(uint64_t *)d;
        a64=a;
        for(i=0;i<ds->noc*nor;i++)
        {
            x64=qmul(f,*(p64),a64);
            *(p64++)=x64;
        }
        return;
      case 8:
        p32=(uint32_t *)d;
        x64=a<<f->clpm[2];
        for(i=0;i<ds->noc*nor;i++)
        {
            x32=0;
            pccl32(f->clpm,x64,1,p32,&x32);
            *p32=x32;
            p32++;
        }
        return;
      case 9:
        p64=(uint64_t *)d;
        x64=a<<f->clpm[2];
        for(i=0;i<ds->noc*nor;i++)
        {
            y64=0;
            pccl64(f->clpm,x64,1,p64,&y64);
            *p64=y64;
            p64++;
        }
        return;
      default:
        printf("Internal error in DSMul - type not set\n");
        exit(47);
    }
}


void PExtract(const DSPACE * ds, const Dfmt *mq, Dfmt *mp,
              uint64_t nor, uint64_t psiz)
{
    DSPACE dsp;
    Dfmt *ptp,*ptp1;
    const Dfmt *ptq, *ptq1;;
    const FIELD * f;
    FELT fq,fp,fr;
    uint64_t i,j,k,col,dfmt,colstodo,mply;
    uint64_t eltsleft;
    uint64_t *lfz;
    uint16_t * lfx;
    uint16_t wk1;
    uint32_t * lfy1,*lfy2;
    uint32_t wk2;
    int bymem,byused,bytesout;
    uint16_t iv[EXTIVCNT];
    uint16_t *ivp,*ivp1;
    uint64_t leftiniv,dofromiv,ptodo,pthistime;

    f=ds->f;
    PSSet(f,ds->noc,&dsp);
    switch (f->pextype)
    {
      case 1:   // GF4
        lfx=(uint16_t *) ((uint8_t *)f + f->Tlfx);
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            for(j=1;j<ds->nob;j+=2)
            {
                wk1=lfx[*ptq]+lfx[*(ptq+1)]*16;
                *ptp=wk1&255;
                *(ptp+psiz)=(wk1>>8)&255;
                ptp++;
                ptq+=2;
            }
            if( ((ds->nob)&1)!=0)
            {
                wk1=lfx[*ptq];
                *ptp=wk1&255;
                *(ptp+psiz)=(wk1>>8)&255;
            }
        }
        return;
      case 2:    // GF8
        lfy1=(uint32_t *) ((uint8_t *)f + f->Tlfx);
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            for(j=ds->noc;j>=8;j-=8)
            {
                wk2=lfy1[*ptq]         +
                    lfy1[*(ptq+1)]*4   +
                    lfy1[*(ptq+2)]*16  +
                    lfy1[*(ptq+3)]*64;
                *ptp=wk2&255;
                *(ptp+psiz)=(wk2>>8)&255;
                *(ptp+2*psiz)=(wk2>>16)&255;
                ptp+=1;
                ptq+=4;
            }
            if(j==0) continue;
            wk2=lfy1[*ptq];
            if(j>2) wk2+= lfy1[*(ptq+1)]*4;
            if(j>4) wk2+= lfy1[*(ptq+2)]*16;
            if(j>6) wk2+= lfy1[*(ptq+3)]*64;
            *ptp=wk2&255;
            *(ptp+psiz)=(wk2>>8)&255;
            *(ptp+2*psiz)=(wk2>>16)&255;
        }
        return;
      case 3:    // GF9
        lfy1=(uint32_t *) ((uint8_t *)f + f->Tlfx);
        lfy2=lfy1+81;
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            for(j=ds->noc;j>=10;j-=10)
            {
                wk2=lfy1[*ptq]         +
                    lfy1[*(ptq+1)]*9   +
                    lfy2[*(ptq+2)]     +
                    lfy1[*(ptq+3)]*768 +
                    lfy1[*(ptq+4)]*6912;
                *ptp=wk2&255;
                *(ptp+1)=(wk2>>8)&255;
                *(ptp+psiz)=(wk2>>16)&255;
                *(ptp+psiz+1)=(wk2>>24)&255;
                ptp+=2;
                ptq+=5;
            }
// following branches should be correctly predicted, since
// ds->noc (and hence j) will usually be the same every time
            if(j==0) continue;
            wk2 =lfy1[*ptq];
            if(j>=3) wk2+=lfy1[*(ptq+1)]*9;
            if(j>=5) wk2+=lfy2[*(ptq+2)];
            if(j>=7) wk2+=lfy1[*(ptq+3)]*768;
            if(j>=9) wk2+=lfy1[*(ptq+4)]*6912;
            *ptp=wk2&255;
            *(ptp+psiz)=(wk2>>16)&255;
            if(j>=6)
            {
                *(ptp+1)=(wk2>>8)&255;
                *(ptp+psiz+1)=(wk2>>24)&255;
            }
        }
        return;
      case 4:    // GF16
        lfy1=(uint32_t *) ((uint8_t *)f + f->Tlfx);
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            for(j=ds->noc;j>=8;j-=8)
            {
                wk2=lfy1[*ptq]         +
                    lfy1[*(ptq+1)]*4   +
                    lfy1[*(ptq+2)]*16  +
                    lfy1[*(ptq+3)]*64;
                *ptp=wk2&255;
                *(ptp+psiz)=(wk2>>8)&255;
                *(ptp+2*psiz)=(wk2>>16)&255;
                *(ptp+3*psiz)=(wk2>>24)&255;
                ptp+=1;
                ptq+=4;
            }
            if(j==0) continue;
            wk2=lfy1[*ptq];
            if(j>2) wk2+= lfy1[*(ptq+1)]*4;
            if(j>4) wk2+= lfy1[*(ptq+2)]*16;
            if(j>6) wk2+= lfy1[*(ptq+3)]*64;
            *ptp=wk2&255;
            *(ptp+psiz)=(wk2>>8)&255;
            *(ptp+2*psiz)=(wk2>>16)&255;
            *(ptp+3*psiz)=(wk2>>24)&255;
        }
        return;

      case 5:    // default - ulitmately to be obsolete
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            for(k=0;k<f->pow;k++)
                memset(ptp+k*psiz,0,dsp.nob);
            for(j=0;j<ds->noc;j++)
            {
                fq=DUnpak(ds,j,ptq);
                for(k=0;k<f->pow-1;k++)
                {
                    fr=fq/f->charc;
                    fp=fq-fr*f->charc;
                    DPak(&dsp,j,ptp+k*psiz,fp);
                    fq=fr;
                }
                DPak(&dsp,j,ptp+k*psiz,fq);
            }
        }
        return;
      case 6:    // primes 17-2^64, simple Barrett
        pcbarrett(&(f->barpar[0]), mq, mp, nor*ds->noc, psiz);
        return;
      case 7:    // all one-byte fields, including char 2.  lookup.
        lfz=(uint64_t *) ((uint8_t *)f + f->Tlfx);
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            col=0;
            while(col<ds->noc)
            {
                dfmt=0;
                mply=1;
                colstodo=f->pentbyte;
                if(col+f->pentbyte>ds->noc) colstodo=ds->noc-col;
                for(j=0;j<colstodo;j++)
                {
                    dfmt+=mply*lfz[*(ptq++)];
                    mply=mply*f->charc;
                }
                ptp1=ptp;
                for(j=0;j<f->pow;j++)
                {
                    *ptp1 = dfmt&255;
                    dfmt=dfmt>>8;
                    ptp1+=psiz;
                }
                ptp++;
                col+=f->pentbyte;
            }
        }
        return;
      case 8:    // char 2 of two or more bytes
        lfz=(uint64_t *) ((uint8_t *)f + f->Tlfx);
        bymem=8;
        if(f->paktyp==1) bymem=4;
        if(f->paktyp==2) bymem=2;
        byused=(f->pow+7)>>3;
        for(i=0;i<nor;i++)
        {
            ptq=mq+i*ds->nob;
            ptp=mp+i*dsp.nob;
            col=0;
            while(col<ds->noc)
            {
                ptp1=ptp;
                for(k=0;k<byused;k++)
                {
                    ptq1=ptq+k;
                    dfmt=0;
                    mply=1;
                    colstodo=f->pentbyte;
                    if(col+f->pentbyte>ds->noc) colstodo=ds->noc-col;
                    for(j=0;j<colstodo;j++)
                    {
                        dfmt+=mply*lfz[*(ptq1)];
                        ptq1+=bymem;
                        mply=mply*f->charc;
                    }
                    bytesout=f->pow-8*k;
                    if(bytesout>8) bytesout=8;
                    for(j=0;j<bytesout;j++)
                    {
                        *ptp1 = dfmt&255;
                        dfmt=dfmt>>8;
                        ptp1+=psiz;
                    }
                }
                col+=f->pentbyte;
                ptp++;
                ptq+=bymem*f->pentbyte;
            }
        }
        return;
      case 9:
        lfz=(uint64_t *) ((uint8_t *)f + f->Tlfx);
        eltsleft=nor*ds->noc;
        ptq=mq;     // Dfmt* so 8 bits
        ptp=mp;
        while(eltsleft!=0)
        {
            colstodo=eltsleft;
            if(colstodo>f->atatime) colstodo=f->atatime;
//  {{ 1 {{   aim now is to do colstodo inputs
            pcbarrett(&(f->barpar[0]), ptq, (Dfmt *)&(iv[0]),
                      colstodo, 2*colstodo);
//  -- 2 --   so now we want to convert colstodo inputs from iv to ptp
            ivp=(uint16_t *) &(iv[0]);

            leftiniv=colstodo;
            while(leftiniv>0)
            {
                dofromiv=leftiniv;
                if(dofromiv>f->pentbyte) dofromiv=f->pentbyte;
//  {{ 3 {{  aim now is to do dofromiv inputs from ivp to ptp1
//           but now dofrominv is at most f->pentbyte
                ptodo=f->pow;
                ivp1=ivp;
                ptp1=ptp;
                while(ptodo>0)
                {
                    pthistime=ptodo;
                    if(pthistime>f->spaclev) pthistime=f->spaclev;
//  {{ 4 {{  aim now do pthistime outputs from ivp1 to ptp1
                    dfmt=0;
                    mply=1;
                    for(j=0;j<dofromiv;j++)
                    {
                        dfmt+=mply*lfz[*ivp1];
                        mply=mply*f->charc;
                        ivp1++;
                    }
                    for(j=0;j<pthistime;j++)
                    {
                        *ptp1 = dfmt&255;
                        dfmt>>=8;
                        ptp1+=psiz;
                    }
//  }} 4 }}
                    ptodo-=pthistime;
                    ivp1+=colstodo-dofromiv;
                }
//  }} 3 }}  done them
                leftiniv-=dofromiv;
                ivp+=dofromiv;
                ptp++;
            }
//  }} 1 }}   done them.
            ptq+=colstodo*f->bytesper;
            eltsleft-=colstodo;
        }
        return;
      case 10:    // two byte fields needing no Barrett
        lfz=(uint64_t *) ((uint8_t *)f + f->Tlfx);
        for(i=0;i<nor;i++)
        {
            ivp=(uint16_t *) (mq+i*ds->nob);
            ptp=mp+i*dsp.nob;
            col=0;
            while(col<ds->noc)
            {
                dfmt=0;
                mply=1;
                colstodo=f->pentbyte;
                if(col+f->pentbyte>ds->noc) colstodo=ds->noc-col;
                for(j=0;j<colstodo;j++)
                {
                    dfmt+=mply*lfz[*(ivp++)];
                    mply=mply*f->charc;
                }
                ptp1=ptp;
                for(j=0;j<f->pow;j++)
                {
                    *ptp1 = dfmt&255;
                    dfmt=dfmt>>8;
                    ptp1+=psiz;
                }
                ptp++;
                col+=f->pentbyte;
            }
        }
        return;
      default:
        printf("Unknown pextype %d\n",(int)f->pextype);
        exit(82); 
    }
}
#ifdef NEVER
            ptp1=ptp;
            fullblocks=colstodo/f->pentbyte;
            for(i=0;i<fullblocks;i++)
            {
                dfmt=0;
                mply=1;
                for(j=0;j<f->pentbyte;j++)
                {
                    dfmt+=mply*lfz[*(iv[i*f->pentbyte+j*colstodo])];
                    mply=mply*f->charc;
                }
                for(j=0;j<f->spaclev;j++)
                {
                    *ptp1 = dfmt&255;
                    dfmt=dfmt>>8;
                    ptp1+=psiz;
                }
            }
            lastblock=colstodo-f->pentbyte*fullblocks;
            if(lastblock!=0)
            {
                dfmt=0;
                mply=1;
                for(j=0;j<lastblock;j++)
                {
                    dfmt+=mply*lfz[*(iv[i*f->pentbyte+j*colstodo])];
                    mply=mply*f->charc;
                }
                for(j=0;j<f->pow;j++)
                {
                    *ptp1 = dfmt&255;
                    dfmt=dfmt>>8;
                    ptp1+=psiz;
                }
            }

        }
        return;
////
    dfmt=0;
    mply=1;
    intsin=colsleft;
    if(intsin>f->pentbyte) intsin=f->pentbyte;
    for(j=0;j<intsin;j++)
    {
        dfmt+=mply*lfz[*(pti++)];
        mply=mply*f->charc;
    }
////
    bytesout=f->pow;
    if(bytesout>f->spaclev) bytesout=f->spaclev;
    for(j=0;j<bytesout;j++)
    {
        *ptp1 = dfmt&255;
        dfmt>>=8;
        ptp1+=psiz;
    }
////
#endif

void PAssemble(const DSPACE * ds, const Dfmt *mp, Dfmt *mq,
              uint64_t nor, uint64_t psiz)
{
    DSPACE dsp;
    const Dfmt *ptp;
    Dfmt *ptq;
    const FIELD * f;
    uint16_t * lfa;
    uint16_t wk1;
    uint32_t *lfb1, *lfb2;
    uint32_t wk2;
    uint64_t wk2a,wk2b,wk2aa,pmult;
    uint64_t i,j,k,m;
    uint64_t lefttodo;
    uint64_t * lfx;
    FELT fq,fp;
    f=ds->f;
    PSSet(f,ds->noc,&dsp);
    memset(mq,0,ds->nob*nor);
    lfa=NULL;  lfb1=NULL; lfb2=NULL; lfx=NULL;  // Avoid compilation warnings
    if( f->pastype==1) lfa=(uint16_t *) ((uint8_t *)f + f->Tlfa);
    if( f->pastype==2 ) lfb1=(uint32_t *) ((uint8_t *)f + f->Tlfa);
    if( f->pastype==3 )
    {
        lfb1=(uint32_t *) ((uint8_t *)f + f->Tlfa);
        lfb2=lfb1+243;
    }
    if( f->pastype==4 ) lfb1=(uint32_t *) ((uint8_t *)f + f->Tlfa);
    if(f->pastype==6) lfx=(uint64_t *) ((uint8_t *)f + f->Tlfx);
    pmult=256;
    if(f->pastype==7)
    {
        lfx=(uint64_t *) ((uint8_t *)f + f->Tlfx);
        if(f->charc==3) pmult=243;
        if(f->charc==5) pmult=125;
        if(f->charc==7) pmult=49;
        if(f->charc==11) pmult=121;
        if(f->charc==13) pmult=169;
    }
    for(i=0;i<nor;i++)
    {
        lefttodo=ds->noc;
        ptq=mq+i*ds->nob;
        ptp=mp+i*dsp.nob;

//  Do some fastpath on big chunks

        switch(f->pastype)
        {
          case 1:
            while(lefttodo>=8)
            {
                wk1=lfa[*ptp]+lfa[*(ptp+psiz)]*2;
                *ptq=wk1&255;
                *(ptq+1)=(wk1>>8)&255;
                ptp++;
                ptq+=2;
                lefttodo-=8;
            }
            break;
          case 2:
            while(lefttodo>=8)
            {
                wk2=lfb1[*ptp]+lfb1[*(ptp+psiz)]*2
                    +lfb1[*(ptp+2*psiz)]*4;
                *ptq=wk2&255;
                *(ptq+1)=(wk2>>8)&255;
                *(ptq+2)=(wk2>>16)&255;
                *(ptq+3)=(wk2>>24)&255;
                ptp++;
                ptq+=4;
                lefttodo-=8;
            }
            break;
          case 3:
            while(lefttodo>=10)
            {
                wk2a=lfb1[*ptp]+lfb1[*(ptp+psiz)]*3;
                wk2b=lfb2[*(ptp+1)]+lfb2[*(ptp+psiz+1)]*3;
                wk2a+=(wk2b<<16);
                *ptq=wk2a&255;
                *(ptq+1)=(wk2a>>8)&255;
                *(ptq+2)=(wk2a>>16)&255;
                *(ptq+3)=(wk2a>>24)&255;
                *(ptq+4)=(wk2a>>32)&255;
                ptp+=2;
                ptq+=5;
                lefttodo-=10;
            }
            break;
          case 4:
            while(lefttodo>=8)
            {
                wk2=lfb1[*ptp]+lfb1[*(ptp+psiz)]*2
                    +lfb1[*(ptp+2*psiz)]*4+lfb1[*(ptp+3*psiz)]*8;
                *ptq=wk2&255;
                *(ptq+1)=(wk2>>8)&255;
                *(ptq+2)=(wk2>>16)&255;
                *(ptq+3)=(wk2>>24)&255;
                ptp++;
                ptq+=4;
                lefttodo-=8;
            }
            break;
          case 5:
            break;
          case 6:
            while(lefttodo>=f->pentbyte)
            {
                wk2a=0;
                wk2b=1;
                for(j=0;j<f->pow;j++)
                {
                    wk2a+=wk2b*lfx[*(ptp+j*psiz)];
                    wk2b*=f->charc;
                }
                for(j=0;j<f->pentbyte;j++)
                {
                    *(ptq++)=wk2a&255;
                    wk2a=wk2a>>8;
                }
                ptp++;
                lefttodo-=f->pentbyte;
            }
            break;
          case 7:
            while(lefttodo>=f->pentbyte)
            {
                wk2a=0;
                wk2aa=0;
                wk2b=1;
                for(j=0;j<f->pentbyte;j++)
                {
                    wk2a+=wk2b*lfx[*(ptp+j*psiz)];
                    wk2b*=f->charc;
                }
                wk2b=1;
                for(j=f->pentbyte;j<f->pow;j++)
                {
                    wk2aa+=wk2b*lfx[*(ptp+j*psiz)];
                    wk2b*=f->charc;
                }
                for(j=0;j<f->pentbyte;j++)
                {
                    *((uint16_t *)ptq)=(wk2aa&255)*pmult+(wk2a&255);
                    ptq+=2;
                    wk2a=wk2a>>8;
                    wk2aa=wk2aa>>8;
                }
                ptp++;
                lefttodo-=f->pentbyte;
            }
            break;
          default:
printf("pastype is %d\n",f->pastype);
            break;
        }
//  Do the remaining columns by steam
        for(j=0;j<lefttodo;j++)
        {
            fq=0;
            m=1;
            for(k=0;k<f->pow;k++)
            {
                fp=DUnpak(&dsp,j,ptp+k*psiz);
                fq=fq + m*fp;
                m=m*f->charc;
            }
            DPak(ds,j,ptq,fq);
        }
    }
    return;
} 

void DCpy(const DSPACE * ds, const Dfmt * d1, uint64_t nor, Dfmt * d2)
{
    memcpy(d2,d1,ds->nob*nor);
}

/*    end of field.c       */
