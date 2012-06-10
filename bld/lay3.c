/*
         lay3.c   -    Layer 3 multi-thread scheduler                         
         =======       R. A. Parker    15.02.2012
*/
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include "mtax.h"
#include "lay2do.h"
#include "lay3.h"


/* ======== first the multithread data ========*/
/* first the main mutex - "dodgy".  This lock  */
/* is obtained before accessing any of the     */
/* shared data, which follws in in the source  */

pthread_mutex_t dodgy=PTHREAD_MUTEX_INITIALIZER;

char schedstat;

/* the thread structures themselves      */
pthread_t mythread[THREADS];

/* this just tells a thread its number   */
int parms[THREADS];

         /* thread status table  */
/*             B busy                    */
/*             I idle                    */
char tstat[THREADS];

/* condition variable that the workers   */
/* wait on when they are idle            */
pthread_cond_t wakeworker[THREADS];

/* condition variable that the scheduler */
/* waits on when it is idle              */
pthread_cond_t kick=PTHREAD_COND_INITIALIZER;

/* condition variable that main          */
/* waits on for a moj to be ready        */
pthread_cond_t mojwt=PTHREAD_COND_INITIALIZER;

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
/* F  free - no-one is using this moj    */
/* A  awaiting output.  No data yet      */
/* O  Open as output.  Being populated   */
/* R  released.  Can be used as input    */ 
char lay3mojstat[MOJMAX];

/* reference count - free when zero      */
int lay3mojref[MOJMAX];

/* pointer to the actual memory of moj   */
char * lay3mojmem[MOJMAX];

/* pointer to the memory to free of moj   */
char * lay3mojfree[MOJMAX];

int waitmoj;

/* ==========job data ================== */

/* job status F(ree) W(aiting) R(unning) */
char jstat[JOBMAX];

int jprog[JOBMAX];

int jinp[JOBMAX][10];

int joup[JOBMAX][10];

/* ======== scheduler routine ========== */
static void * scheduler(void * junk)
{ 
    int jx,wx,i;
#ifdef DEBUG
printf("Scheduler started\n");
#endif
 (void)junk;
    pthread_mutex_lock(&dodgy);
/* any initialization goes here  */
    schedstat='R';
    while(1)
    {
        pthread_cond_wait(&kick,&dodgy);

/* check to see if main is waiting for      */
/* a moj that is now ready                  */
        if(waitmoj!=0)
        {
            if(lay3mojstat[waitmoj]=='R')
            {
                waitmoj=0;
                pthread_cond_signal(&mojwt);
            }
        }

/* run through the jobs and the workers     */
/* to start lots of things                  */

        while(1)
        {
/* find a waiting job */
            jx=0;
            while((jx+1)<JOBMAX)
            {
                jx++;
                if(jstat[jx]!='W') continue;
/* check all the inputs are ready  */
                i=0;
                while(jinp[jx][i]!=0)
                {
                    if(lay3mojstat[jinp[jx][i]]!='R') break;
                    i++;
                }
                if(jinp[jx][i]==0) break;
            }
            if((jx+1)>=JOBMAX) break;
/* find an idle thread  */
            wx=0;
            while((wx+1)<THREADS)
            {
                wx++;
                if(tstat[wx]=='I') break;
            }
            if((wx+1)>=THREADS) break;
/* OK.  give that thread that job!  */

#ifdef DEBUG
printf("Sched giving thread %d job %d which is a %d\n",wx,jx,jprog[jx]);
#endif
            tstat[wx]='B';
            parms[wx]=jx;
            jstat[jx]='R';
            pthread_cond_signal(wakeworker+wx);
        }
    }
    return NULL;
}

/* ========== worker routine =========== */
static void * worker(void * parms)
{ 
    int * mynostar;
    int myno;
    int proggy;
    int inputs[10];
    int outputs[10];
    int i,ix;
    mynostar=(int *)parms;
    myno=mynostar[0];
#ifdef DEBUG
printf("Worker %d started\n",myno);
#endif
    while(1)
    {
        pthread_mutex_lock(&dodgy);
        tstat[myno]='I';
        pthread_cond_signal(&kick);
        pthread_cond_wait(wakeworker+myno,&dodgy);
        if(tstat[myno]!='B')
        {
            pthread_mutex_unlock(&dodgy);
            continue;
        }
        ix=mynostar[0];
        proggy=jprog[ix];
        for(i=0;i<10;i++) inputs[i] =jinp[ix][i];
        for(i=0;i<10;i++) outputs[i]=joup[ix][i];
        pthread_mutex_unlock(&dodgy);
        if(proggy==DIE) return mynostar;
#ifdef DEBUG
printf("Worker %d starting job %d which is a %d\n",myno,ix,jprog[ix]);
#endif
        lay2do(proggy,inputs,outputs);
#ifdef DEBUG
printf("Worker %d completed job %d which is a %d\n",myno,ix,jprog[ix]);
#endif
        pthread_mutex_lock(&dodgy);
        jstat[ix]='F';
        pthread_mutex_unlock(&dodgy);;
    }
    return mynostar;
}


/* ====================Initialize ================== */
/* the initialize function.  This is called by the   */
/* program to start up all the threads and get going */
int lay3initialize(void)
{
    int i;
    cpu_set_t cpuset;

    for(i=0;i<THREADS;i++) pthread_cond_init(wakeworker+i,NULL);


/* to start with all the moj objects are free  */
    for(i=1;i<MOJMAX;i++)
        lay3mojstat[i]='F';
/* all the threads start off busy initializing */    
    for(i=1;i<THREADS;i++) tstat[i]='B';

/* also need to empty the list of jobs to do   */
    for(i=1;i<JOBMAX;i++)
        jstat[i]='F';
    

/* get the "dodgy" lock so that nothing does   */
/* any damage while we are starting up         */
    pthread_mutex_lock(&dodgy);
             
/* start the scheduler thread                  */
    pthread_create(mythread+0,NULL, scheduler, parms);
/* start all the worker threads                */
    for(i=1;i<THREADS;i++)
    {
        parms[i]=i;
        pthread_create(mythread+i,NULL, worker, parms+i);
    }
    schedstat='I'; /* scheduler status "initializing" */
    for(i=1;i<THREADS;i++)
    {
        CPU_ZERO(&cpuset);
        CPU_SET(i,&cpuset);
        pthread_setaffinity_np(mythread[i],sizeof(cpu_set_t),&cpuset);
    }
    CPU_ZERO(&cpuset);
    CPU_SET(16,&cpuset);
    pthread_setaffinity_np(mythread[0],sizeof(cpu_set_t),&cpuset);

/* all is ready - now let's go                 */
    pthread_mutex_unlock(&dodgy);

    return 1;
}

/* first the three that all can use */
char * lay3ptr(int moj)
{
    char * retptr;
    pthread_mutex_lock(&dodgy);
    retptr = lay3mojmem[moj];
    pthread_mutex_unlock(&dodgy);
    return retptr;
}

char * lay3mem(int moj, size_t bytes)
{
    char * ptr;
    pthread_mutex_lock(&dodgy);
    if(lay3mojstat[moj]!='A')
    {
        printf("Memory allocation requested for %d but status is %c\n",
                             moj, lay3mojstat[moj]);
        pthread_mutex_unlock(&dodgy);
        exit(2);
    }
    ptr=malloc(bytes+CACHELINE);
    if(ptr==NULL)
    {
      printf("memory failure for moj %d %ld\n",moj,(long int)bytes);
        pthread_mutex_unlock(&dodgy);
        exit(1);
    }
    lay3mojfree[moj]=ptr;
    while((((uintptr_t)ptr)%CACHELINE)!=0) ptr+=8;
    lay3mojmem[moj]=ptr;
    lay3mojstat[moj]='O';
    pthread_mutex_unlock(&dodgy);
    return ptr;
}

void lay3release(int moj)
{
    pthread_mutex_lock(&dodgy);
    if(lay3mojstat[moj]=='O')
    {
        lay3mojstat[moj]='R';
        pthread_cond_signal(&kick);
        pthread_mutex_unlock(&dodgy);
        return;
    }
    lay3mojref[moj]--;
    if(lay3mojref[moj]==0)
    {
        free(lay3mojfree[moj]);
        lay3mojstat[moj]='F';
    }
    pthread_mutex_unlock(&dodgy);
    return;
}

int  lay3newmoj(void)
{
    int i;
    pthread_mutex_lock(&dodgy);
    for(i=1;i<MOJMAX;i++)
        if(lay3mojstat[i]=='F') break;
    if(i<MOJMAX)
    {
        lay3mojstat[i]='A';
        lay3mojref[i]=1;
        lay3mojmem[i]=NULL;
        pthread_mutex_unlock(&dodgy);
        return i;
    }
    printf("Run out of moj\n");
    pthread_mutex_unlock(&dodgy);
    exit(4);

}

void lay3deref(int moj)
{
    pthread_mutex_lock(&dodgy);
    lay3mojref[moj]--;
    if(lay3mojref[moj]==0)
    {
        free(lay3mojfree[moj]);
        lay3mojstat[moj]='F';
    }
    pthread_mutex_unlock(&dodgy);
    return;
}

void lay3waitmoj(int moj)
{
    pthread_mutex_lock(&dodgy);
    waitmoj=moj;
    pthread_cond_signal(&kick);
    while(waitmoj!=0)
        pthread_cond_wait(&mojwt,&dodgy);
    pthread_mutex_unlock(&dodgy);
    return;
}

void lay3submit(int proggy, int * inputs, int * outputs)
{
    int i,ix;

    pthread_mutex_lock(&dodgy);
    i=0;
    while(inputs[i]!=0)
    {
        lay3mojref[inputs[i]]++;
        i++;
    }
    for(ix=1;ix<JOBMAX;ix++)
        if(jstat[ix]=='F') break;
    if(ix==JOBMAX)
    {
        printf("Run out of job space\n");
        pthread_mutex_unlock(&dodgy);
        exit(4);
    }
#ifdef DEBUG
printf("Submitted job %d which is a %d\n",ix,proggy);
#endif
    jstat[ix]='W';
    jprog[ix]=proggy;
    i=0;
    while(inputs[i]!=0)
    {
        jinp[ix][i]=inputs[i];
        i++;
    }
    jinp[ix][i]=0;
    i=0;
    while(outputs[i]!=0)
    {
        joup[ix][i]=outputs[i];
        i++;
    }
    joup[ix][i]=0;
    while(schedstat=='I')
    {
        pthread_mutex_unlock(&dodgy);
        pthread_mutex_lock(&dodgy);
    }
    pthread_cond_signal(&kick);
    pthread_mutex_unlock(&dodgy);
}

void lay3shutdown(void)
{
    return;
}

/* end of lay3s.c  */

