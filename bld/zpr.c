/*
      zpr.c     MTX64 convert internal format to text
      =====     R. A. Parker 14.02.2012
*/
#include <stdio.h>
#include <stdlib.h>
#include "mtax.h"
 
int main(int argc,  char **argv)
{
    FILE *f1;
    FIELD * f;
    Dfmt v1;
    DSPACE ds;
    uint64 fdef,nor,noc;
    FELT fel;
    int res;
    size_t lenf;
    uint64 i,j;

    if (argc != 2) 
    {
        printf("usage zpr <m1>\n");
        exit(14);
    }
    f1 = RdHdr(argv[1],&fdef,&nor,&noc);
    printf(" 1%6lld%6lld%6lld",fdef,nor,noc);
    lenf=LenField(fdef);
    f = malloc(lenf);
    if(f==NULL)
    {
        printf("Can't malloc field structure/n");
        exit(15);
    }
    res=FieldSet(fdef,f);
    if(res==99)printf("99/n");
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        printf("Can't malloc a single vector/n");
        exit(19);
    }

    for(i=0;i<nor;i++)
    {
	RdMatrix(f1,&ds,1,v1);
	for(j=0;j<noc;j++)
	{
	    if(j%80 == 0) printf("\n");
	    fel=DUnpak(&ds,j,v1);
	    printf("%lld",fel);
	}
    }
    printf("\n");
    Close(f1);
    return 0;
}
/*  end of zpr.c    */
