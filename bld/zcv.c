/*
      zcv.c     MTX64 convert text to internal format
      =====     R. A. Parker   14.02.2012 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtax.h"
int hadcr;
FIELD * f;
FELT dig[11];
 
    /******  subroutine to get an integer like FORTRAN does  */
static uint64 getin(uint64 a)
{
    int c;
    uint32 i,j;
    j=0;
 
    if(hadcr == 1) return j;
    for(i=0;i<a;i++)
    {
	c = getchar();
	if(c == '\n')
	{
	    hadcr = 1;
	    return j;
	}
	if(c < '0') c = '0';
	if(c > '9') c = '0';
	j = 10*j + (c-'0');
    }
    return j;
}
 
/* subroutine to get an integer and reduce mod p */
static FELT getz(void)
{
    FELT res,res1;
    int c, flag;
    flag=1;
    res=0;
    c=getchar();
    while ((c==' ')||(c=='\n')) c=getchar();
    if(c=='-') 
    {
        flag=2;
        c=getchar();
    }
    while ((c!=' ')&&(c!='\n'))
    {
        if((c<'0')||(c>'9'))
        {
            printf("Illegal character in input\n");
            exit(7);
        }
        res1 = FieldMul(f,dig[10],res);
        res = FieldAdd(f,dig[c-'0'],res1);
        c=getchar();
    }
    if (flag==2) 
        return FieldNeg(f,res);
    else
        return res;
}
static void nextline(void)
{
    if(hadcr == 1)
    {
	hadcr=0;
	return;
    }
    while (getchar() != '\n');
}
 
int main(int argc,  char **argv)
{
    FILE *f1;
    size_t lenf;
    uint64 fdef,nor,noc,mode;
    int res;
    DSPACE ds;
    FELT fel;

    uint64 i,j,k;
    Dfmt v1;
 
	/******  First check the number of input arguments  */
    if (argc != 2)
    {
        printf("usage zcv <m1> < <textfile>\n");
        exit(14);
    }
    hadcr = 0;
    mode = getin(2);
	    /****** mmpppppprrrrrrcccccc          */
	    /****** 01000002000012000014          */
	    /****** 01 is mode                    */
	    /****** 000002 is the field parameter */
	    /****** 000012  number of rows        */
	    /****** 000014  number of columns     */
    fdef = getin(6);
    nor = getin(6);
    noc = getin(6);
 
	    /******  open the output file  */
 
    f1 = WrHdr(argv[1],fdef,nor,noc);
 
	    /******  initialize the arithmetic  */
	    /******  tell it what field to use  */
    lenf=LenField(fdef);
    f = malloc(lenf);
    if(f==NULL)
    {
        printf("Can't malloc field structure/n");
        exit(8);
    }
    res=FieldSet(fdef,f);
    if(res==99) printf("99\n");
    dig[0]=0;
    dig[1]=1;
    for(i=2;i<=10;i++)
        dig[i]=FieldAdd(f,dig[1],dig[i-1]);
	    /****** and how long the rows are  */
    DSSet(f,noc,&ds);
 
	    /******  check that a row fits in memory  */
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        printf("Can't malloc a single vector/n");
        exit(9);
    }
 
	    /******  for each row of the matrix  */
    for(i=0;i<nor;i++)
    {
 
		/******  start off with the row = 0  */
        memset(v1,0,ds.nob);
 
        switch(mode)
        {
          case 1:
		/******  then for each entry in the row  */
	    for(j=0;j<noc;j++)
	    {
		if(j%80 == 0) nextline();
		k=getin(1);
 
		    /******  put the entry in place  */
		fel=k;
                DPak(&ds,j,v1,fel);
	    }
	    break;
 
          case 2:
 
		/******  then put the 1 into the correct place  */
            nextline();
	    k=getin(6);
	    DPak(&ds,k-1,v1,1);
	    break;

          case 3:
		/******  then for each entry in the row  */
	    for(j=0;j<noc;j++)
	    {
		if(j%25 == 0) nextline();
		k=getin(3);
 
		    /******  put the entry in place  */
		fel=k;
                DPak(&ds,j,v1,fel);
	    }
	    break;
 
          case 4:
		/******  then for each entry in the row  */
	    for(j=0;j<noc;j++)
	    {
		if(j%5 == 0) nextline();
		k=getin(8);
 
		    /******  put the entry in place  */
		fel=k;
                DPak(&ds,j,v1,fel);
	    }
	    break;
 
          case 5:
		/******  then for each entry in the row  */
	    for(j=0;j<noc;j++)
	    {
		k=getz();
 
		    /******  put the entry in place  */
		fel=k;
                DPak(&ds,j,v1,fel);
	    }
	    break;
 
          default:
          {
            printf("mode unknown\n");
            exit(9);
          }
        }
 
		/******  write a row of the matrix  */
        WrMatrix(f1,&ds,1,v1);
    }
    Close(f1);
    return 0;
}      /******  end of zcv.c    ******/
