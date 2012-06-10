/*
         lay3s.c   -   Layer 3 header - multi-thread scheduler
                          (single threaded version)            
         =======       R. A. Parker     14.02.2012
*/

#include <stdio.h>
#include <stdlib.h>
#include "mtax.h"
#include "lay2do.h"
#include "lay3.h"

char * cachemalloc(size_t len)
{
    char * x, *y;
    char **z;
    y=malloc(len+8192);
    x=y+8;
    while( (((long int) x)&0x1fff)!=0) x+=8;
    z=(char **)x;
    *(z-1)=y;
    return x;
}

void cachefree(char * x)
{
    char **z;
    z=(char **)x;
    free(z[-1]);
}

/* ========== moj data ================= */
        /* moj status table */

/* status in the moj table */
/* F  free - no-one is using this moj    */
/* A  awaiting output.  No data yet      */
/* O  Open as output.  Being populated   */
/* R  released.  Can be used as input    */ 
char lay3mojstat[MOJMAX];
/* reference count - free when zero      */
int lay3mojref[MOJMAX];
char * lay3mojmem[MOJMAX];
char * lay3mojfree[MOJMAX];

/* first the three that all can use  */
char * lay3ptr(int moj)
{
    return lay3mojmem[moj];
}

char * lay3mem(int moj, size_t bytes)
{
    char * ptr;
    if(lay3mojstat[moj]!='A')
    {
        printf("Memory allocation requested for %d but status is %c\n",
                             moj, lay3mojstat[moj]);
        exit(2);
    }
    ptr=malloc(bytes+256);
    if(ptr==NULL)
    {
        printf("memory failure for moj %d %ld\n",moj,bytes);
        exit(1);
    }
    lay3mojfree[moj]=ptr;
    while( (((unsigned long long) ptr)%CACHELINE)!=0) ptr+=8;
    lay3mojmem[moj]=ptr;
    lay3mojstat[moj]='O';
    return ptr;
}

void lay3release(int moj)
{
    if(lay3mojstat[moj]=='O')
    {
        lay3mojstat[moj]='R';
        return;
    }
    lay3mojref[moj]--;
    if(lay3mojref[moj]==0)
    {
        free(lay3mojfree[moj]);
        lay3mojstat[moj]='F';
    }
    return;
}

/* now the five that only the layer 4 can call  */

int lay3initialize(void)
{
    int i;
    for(i=1;i<MOJMAX;i++)
        lay3mojstat[i]='F';    /* all moj are free  */
    return 1;
}

int  lay3newmoj(void)
{
    int i;
    for(i=1;i<MOJMAX;i++)
        if(lay3mojstat[i]=='F') break;
    if(i<MOJMAX)
    {
        lay3mojstat[i]='A';
        lay3mojref[i]=1;
        lay3mojmem[i]=NULL;
        return i;
    }
    printf("Run out of moj\n");
    exit(4);

}

void lay3deref(int moj)
{
    lay3mojref[moj]--;
    if(lay3mojref[moj]==0)
    {
        free(lay3mojfree[moj]);
        lay3mojstat[moj]='F';
    }
    return;
}

void lay3waitmoj(int moj)
{
    if(lay3mojstat[moj]!='R')
    {
        printf("Wait requested for %d but status is %c\n",
                             moj, lay3mojstat[moj]);
        exit(4);
    }
    return;
}

void lay3submit(int proggy, int * inputs, int * outputs)
{
    int i;
    i=0;
    while(inputs[i]!=0)
    {
        lay3mojref[inputs[i]]++;
        i++;
    }

    i=0;
    lay2do(proggy,inputs,outputs);
}

/* end of lay3s.c  */
