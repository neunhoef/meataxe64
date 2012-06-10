/*   lay2do.c - Just do it interface  */
/*   R. A. Parker   14.02.2012        */

#include <stdio.h>
#include <stdlib.h>
#include "mtax.h"
#include "lay3.h"
#include "lay2do.h"

void lay2do(int proggy, int * moji, int * mojo)
{
    if(proggy==CVA) 
    {
        pgcva(moji[0], moji[1], mojo[0]);
        return;
    }
    if(proggy==CVB) 
    {
        pgcvb(moji[0], moji[1], mojo[0]);
        return;
    }
    if(proggy==MUL) 
    {
        pgmul(moji[0], moji[1], moji[2], moji[3], mojo[0]);
        return;
    }
    if(proggy==CVD) 
    {
        pgcvd(moji[0], moji[1], mojo[0]);
        return;
    }
    printf("Unknown proggy number %d<n",proggy);
    exit(1);
    
}
/* end of lay2do.c  */
