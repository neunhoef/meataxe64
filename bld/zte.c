/*
      zte.c     MTX64 tensor product
      =====     R. A. Parker   14.02.2012 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
 
int main(int argc,  char **argv)
{
    FILE *f1,*f2,*f3;
    FIELD * f;
    FELT e1,e2,e3;
    int res;
    size_t lenf;
    uint64 fdef1,nor1,noc1,fdef2,nor2,noc2,nor3,noc3;
    uint64 i1,i2,j1,j2;
    DSPACE ds1,ds2,ds3;
    Dfmt v1,m2,v3;

 
	/******  First check the number of input arguments  */
    if (argc != 4)
    {
        printf("usage zte m1 m2 ans\n");
        exit(21);
    }
 
    f1 = RdHdr(argv[1],&fdef1,&nor1,&noc1);
    f2 = RdHdr(argv[2],&fdef2,&nor2,&noc2);
    if(fdef1!=fdef2)
    {
        printf("Matrices incompatible\n");
        exit(22);
    }
    lenf=LenField(fdef1);
    f = malloc(lenf);
    if(f==NULL)
    {
        printf("Can't malloc field structure/n");
        exit(22);
    }
    res=FieldSet(fdef1,f);
    if(res==99) printf("99\n");
    nor3=nor1*nor2;
    noc3=noc1*noc2;
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    DSSet(f,noc3,&ds3);
    f3 = WrHdr(argv[3],fdef1,nor3,noc3);

 
	    /******  check that a row fits in memory  */
    v1=malloc(ds1.nob);
    m2=malloc(ds2.nob*nor2);
    v3=malloc(ds3.nob);
    if( (v1==NULL) || (m2==NULL) || (v3==NULL) )
    {
        printf("Can't malloc the matrix space/n");
        exit(23);
    }

/* first read in matrix 2  */
    RdMatrix(f2,&ds2,nor2,m2);
    Close(f2);

/* for each row of matrix 1 */
    for(i1=0;i1<nor1;i1++)
    {
	RdMatrix(f1,&ds1,1,v1);

/* for each row of matrix 2 */
        for(i2=0;i2<nor2;i2++)
        {
            memset(v3,0,ds3.nob);
/* for each column of matrix 1 */
            for(j1=0;j1<noc1;j1++)
            {
                e1=DUnpak(&ds1,j1,v1);
/* for each column of matrix 2 */
                for(j2=0;j2<noc2;j2++)
                {
                    e2=DUnpak(&ds2,j2,m2+ds2.nob*i2);
                    e3=FieldMul(f,e1,e2);
                    DPak(&ds3,j1*noc2+j2,v3,e3);
                }
            }
            WrMatrix(f3,&ds3,1,v3);
        }
    }
    Close(f1);
    Close(f3);
    return 0;
}      /******  end of zad.c    ******/
