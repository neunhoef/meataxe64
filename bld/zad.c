/*
      zad.c     MTX64 matrix addition
      =====     R. A. Parker  14.02.2012 
*/
#include <stdio.h>
#include <stdlib.h>
#include "mtax.h"
 
int main(int argc,  char **argv)
{
    FILE *f1,*f2,*f3;
    FIELD * f;
    int res;
    size_t lenf;
    uint64 fdef,nor,noc,fdef1,nor1,noc1;
    DSPACE ds;
    Dfmt v1,v2;
    uint64 i;

 
	/******  First check the number of input arguments  */
    if (argc != 4)
    {
        printf("usage zad m1 m2 ans\n");
        exit(21);
    }
 
    f1 = RdHdr(argv[1],&fdef,&nor,&noc);
    f2 = RdHdr(argv[2],&fdef1,&nor1,&noc1);
    if( (fdef!=fdef1) || (nor!=nor1) || (noc!=noc1) )
    {
        printf("Matrices incompatible\n");
        exit(22);
    }
    lenf=LenField(fdef);
    f = malloc(lenf);
    if(f==NULL)
    {
        printf("Can't malloc field structure/n");
        exit(22);
    }
    res=FieldSet(fdef,f);
    if(res==99) printf("99\n");
    f3 = WrHdr(argv[3],fdef,nor,noc);
    DSSet(f,noc,&ds);
 
	    /******  check that a row fits in memory  */
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    if( (v1==NULL) || (v2==NULL) )
    {
        printf("Can't malloc the two single vectors/n");
        exit(23);
    }
 
	    /******  for each row of the matrix  */
    for(i=0;i<nor;i++)
    {
	RdMatrix(f1,&ds,1,v1);
	RdMatrix(f2,&ds,1,v2);
        DAdd(&ds,v1,v2);
        WrMatrix(f3,&ds,1,v2);
    }
    Close(f1);
    Close(f2);
    Close(f3);
    return 0;
}      /******  end of zad.c    ******/
