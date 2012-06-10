/*   bkmad.c  - MTAX-64 High Performance Meataxe mod 2 only  */
/*   Aiming towards version 1.00                             */
/*   R. A. Parker 29.02.2012                                 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"

void BrickMad(FIELD * f, Afmt a, Brick bk, Cfmt c)
{
    uint8 * ap;
    uint64 *cp, *b0, *bv1, *bv2, *bv3, *bv4, *realc;
    uint64 *bv5, *bv6, *bv7, *bv8;
    uint64 * b;
    uint64 btype, cs, s8, gs, wpercaul;
    unsigned int i, j;
    int ax,ss;
    int ds1,ds2,ds3,ds4,ds5,ds6,ds7,ds8;

    (void)f;
    b = (uint64 *) bk;
    btype=b[0];
    cs=b[1];     /*  3=64byte (to Westmere) 4=128byte (from Sandy Bridge) */
    wpercaul=1<<cs;    /* words per cauldron */
    s8=b[2];     /*  slices divided by 8 */
    gs=b[3];     /*  grease level */
    ss=cs+gs;    /*  shift for words per slice */
    ds1=1<<ss;
    ds2=2<<ss;
    ds3=3<<ss;
    ds4=4<<ss;
    ds5=5<<ss;
    ds6=6<<ss;
    ds7=7<<ss;
    ds8=8<<ss;
    switch(btype)
    {
      case 0:
        return;
      case 1:
        ap=(uint8 *) a;        /* a is program counter for Afmt */
        cp=(uint64 *) c;       /* c is pointer to Cfmt */
        ax=*(ap++);            /* get the first skip/terminate byte */
        while(ax!=255)         /* if terminate, that's the lot */
        {
          b0=b+32;            /* start at the beginning of Brick data */
          cp+=(ax<<cs);       /* skip rows as instructed */
          for(i=0;i<s8;i++)   /* process all lots of 8 slices */
            {
                realc=cp;
                ax=*(ap++);    /* byte of displacement */
                bv1=b0+(ax<<cs);  /* indirect to grease row */
                ax=*(ap++);          /* and again */
                bv2=b0+ds1+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv3=b0+ds2+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv4=b0+ds3+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv5=b0+ds4+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv6=b0+ds5+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv7=b0+ds6+(ax<<cs);
                ax=*(ap++);          /* and again */
                bv8=b0+ds7+(ax<<cs);
                b0+=ds8;           /* ready for next 8 slices */
                for(j=0;j<wpercaul;j++)    /* this is the real work! */
                    *(realc++) ^= 
       *(bv1++)^*(bv2++)^*(bv3++)^*(bv4++)^*(bv5++)^*(bv6++)^*(bv7++)^*(bv8++); 
            }
          ax=*(ap++);        /* get next skip/terminate byte */
            cp+=(1<<cs);
        }
        return;
      default:
        printf("Brick type %llu not known to this module\n",btype);
        exit(17);
    }
}

/* end of hpmi.c  */
