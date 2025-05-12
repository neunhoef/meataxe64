/*
         tfarm3.c   -   Replacement thread farm using stdatomic
         ========       R. A. Parker     9.10.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>

#include "tfarm.h"
#include "tuning.h"
#define SCALE MAXCHOP*MAXCHOP*MAXCHOP

/* Notes TfPause has been replaced by nanosleep
 * Unlike TfPause, itsdurection is not processor dependent
 * Ie slow processors will sleep no longer than fast ones
 * This doesn't seem to be to be a problem.
 * TfAppend has a lock in it, but is only used before the threads are started
 * Hence I believe the lock is redundant
 */

/* type definition completions */

typedef struct mojstruct
{
    void * mem;      // Memory pointer (NULL if none allocated) 
    rdlstruct * RDL; // closable chain of RDLs for this moj
    atomic_int refc; // Read reference counter
    int  junk;       // to make sizeof(mojstruct) divisible by 8
} mojstruct;

struct jobinc
{
    MOJ parm[MAXPARAMS];    // new parameters
    int  proggyno;   // proggy to be executed
    int  priority;   // priority (lower runs earlier)
    atomic_int ref;  // how many things it is waiting for 
    int nparm;       // number of parameters
};

/* Global Variables */

/*
 * Some of these are counting semaphores, implemented using stdatomic
 * However, TfAdd and its replacement atomic_fetch_add have slightly
 * different semantics. TfAdd returns the value newly written,
 * whereas atomic_fetch_add returns the value previously held.
 * In the cases where the return value is used we have to adjust it
 */

/* closejobs is incremented by submit and decremented when a */
/* job completes, allowing TfWaitEnd to know when all the    */
/* submitted jobs have completed. TFUpJobs and TFDownJobs    */
/* allows proggies (e.g. M3Read, M3Write) to manipulate it.  */

atomic_int closejobs;

/* stopfree is incremented by TFStopMOJFree and decremented  */
/* by TFStartMOJFree, and if non-zero, TFRelease blocks      */
/* This allows a string of submissions to proceed without    */
/* a release discarding a MOJ when an unsubmitted job will   */
/* read it                                                   */
 
atomic_int stopfree;

/* Functions replacing the x86 assembler versions */

static int my_atomic_fetch_add(atomic_int *arg, int val)
{
  atomic_int i = atomic_fetch_add(arg, val);
  return i + val;
}

static void wait(uint64_t ns)
{
  struct timespec ts;
  ns *= 10; /* The original claims to pause about 10 times arg ns */
  ts.tv_sec = ns / 1000000000;
  ts.tv_nsec = ns % 1000000000;
  nanosleep(&ts, NULL);
}

/*
 * This is used before the threads are started, so doesn't require locks
 * The lists are in the form of count (element 0) and members (1 -> )
 */
static void append(uint64_t *list, uint64_t new)
{
  uint64_t old = *list + 1;
  list[old] = new;
  *list = old;
}

void TFStopMOJFree(void)
{
  my_atomic_fetch_add(&stopfree, 1);
}

extern void TFStartMOJFree(void)
{
  my_atomic_fetch_add(&stopfree, -1);
}

/* nothreads is the (constant) number of started threads     */
/* It is set by TFInit and used by TFClose                   */

unsigned int nothreads;

/* There is a single "global" mutex called "dodgy" that can  */
/* be used to prevent data races etc., and this is used      */
/* as a last resort, or where performance is not important   */

pthread_mutex_t dodgy=PTHREAD_MUTEX_INITIALIZER;

static void Lock(void)
{
  pthread_mutex_lock(&dodgy);
}

static void Unlock(void)
{
    pthread_mutex_unlock(&dodgy);
}

/* Similarly there is a single "global" condition variable    */
/* that can be used (called holding with "dodgy") to wait for */
/* things to happen.  Again a last resort, or where           */
/* performance is not a major issue.                          */

pthread_cond_t maincond=PTHREAD_COND_INITIALIZER;

static void LcMainPause(void)
{
    pthread_cond_wait(&maincond,&dodgy);
}

static void LcMainKick(void)
{
    pthread_cond_broadcast(&maincond);
}

void TFUpJobs(void)
{
  my_atomic_fetch_add(&closejobs, 1);
}

void TFDownJobs(void)
{
    int i = my_atomic_fetch_add(&closejobs, -1);

    if(i==0)
    {
        Lock();
        LcMainKick();
        Unlock();
    }
}

#define TFMSIZE 10
uint64_t * TFM;
#define TFMFRE 0
#define TFMJOB 1
#define TFMRDL 2

#define FRESIZE 2000

/* ========== thread data ================= */

typedef struct
{
    int threadno;
    jobstruct * JOB;
}  parmstruct;
int * tfthread;
parmstruct * tfparms;

int firstfreethread;

atomic_int runjobs;

void TFWaitEnd(void)
{
    Lock();
    for (;;) {  // wait until all submitted jobs completed
        if(closejobs==0) break;
        LcMainPause();
    }
    Unlock();
    for (;;) {  // wait until all threads got lock (at least) to suspend
        if( (runjobs+nothreads)==0) break;
        wait(3000);
    }
}

//  This should be inlined once it is under regression

static void *AlignTalloc(size_t len)
{
    unsigned long *x,y;
    x=malloc(len+128);
    if(x==NULL) return NULL;
    y=1;
    x++;
    while( (((long int) x)&0x3f)!=0) 
    {
        x++;
        y++;
    }
    *(x-1)=y;
    return (void *) x;
}

static void AlignTree(void *z)
{
    unsigned long y,*x;
    x=(unsigned long *) z;
    y=(unsigned long) *(x-1);
    x -= y;
    free(x);
}


/* ========== moj data ===== */


MOJ tfmoj;

unsigned int nfmoj;
unsigned int nmojes;

/* ========== job data and methods ======= */


static jobstruct *NewJob(void) 
{
    return (jobstruct *) TfLinkOut(TFM+TFMJOB);
}

static void FreeJob(jobstruct *JOB)
{
    TfLinkIn(TFM+TFMJOB,(uint64_t *) JOB);
}

static void JobPop(jobstruct *JOB, int proggy, int priority)
{
    JOB->proggyno = proggy;
    JOB->priority = priority;
    atomic_init(&JOB->ref, 1);
    JOB->nparm=0;
}

/* ========== rdl data and methods ======= */

static rdlstruct *LcNewRdl(void) 
{
     return (rdlstruct *) TfLinkOut(TFM+TFMRDL);
}

static void LcFreeRdl(rdlstruct *RDL)
{
    TfLinkIn(TFM+TFMRDL,(uint64_t *) RDL);
}

static void TfUpJobRef(jobstruct *JOB)
{
    my_atomic_fetch_add(&(JOB->ref),1);   // maybe lock add someday
}

static void UpMojRef(MOJ mj)
{
    my_atomic_fetch_add(&(mj->refc),1);
}

static void LCBindRdl(jobstruct *job, MOJ moj)
{
    rdlstruct * RDL;
    int x;
    UpMojRef(moj);
    RDL=LcNewRdl();
    x=TfLinkIn((uint64_t *)(&(moj->RDL)),(uint64_t *)RDL);
    if(x==2) return;
    RDL->JOB=job;
    TfUpJobRef(job);
}

#if 0 /* Unused */
static void LcBindRdl(jobstruct * JOB, int moj)
{
    rdlstruct * RDL;
    MOJ mj;
    int x;
    mj=tfmoj+moj;
    UpMojRef(mj);
    RDL=LcNewRdl();
    x=TfLinkIn((uint64_t *)(&(mj->RDL)),(uint64_t *)RDL);
    if(x==2) return;
    RDL->JOB=JOB;
    TfUpJobRef(JOB);
}
#endif

/* ==========  jobs ready to run  ======= */

jobstruct ** RUNJOB;
long  jobsready;

static void LcStartJob(jobstruct *JOB, int thread);

static int TfUnQThread(jobstruct *JOBNO)
{
    int thread;
    if(firstfreethread!=-1)
    {
        thread=firstfreethread;
        firstfreethread=tfthread[thread];
        LcStartJob(JOBNO,thread);
        Unlock();
        return thread;
    }
    fprintf(stderr,"different sort of failure\n");
    exit(17);
}

static void TfQJob(jobstruct *JOBNO)
{
    int k,kp;
    jobstruct * t;
    RUNJOB[jobsready++]=JOBNO;
// swim
    k=jobsready-1;
    while(k>0)
    {
        kp=(k-1)/2;
        if(RUNJOB[k]->priority
         >=RUNJOB[kp]->priority) break;
        t=RUNJOB[k];
        RUNJOB[k]=RUNJOB[kp];
        RUNJOB[kp]=t;
        k=kp;
    }
}

static void TfPutJob(jobstruct *JOBNO)
{

    int i;
    Lock();
    i = my_atomic_fetch_add(&runjobs, 1);
    if(i<=0)
    {
        TfUnQThread(JOBNO);
        return;
    }
    if(firstfreethread!=-1) fprintf(stderr,"Yet another sort\n");
    TfQJob(JOBNO);
    Unlock();
    return;
}

// Rename as TfUnQJob once locking sorted

static jobstruct *LcNextRun(void) 
{
    int k,kc1,kc2;
    jobstruct * rj, *t;
    rj=RUNJOB[0];
    RUNJOB[0]=RUNJOB[--jobsready];
// sink
    k=0;
    for (;;) {
        kc1=2*k+1;
        kc2=2*k+2;
        if(kc1>=jobsready) break;
        if(kc2<jobsready)
        {
            if(RUNJOB[kc1]->priority
             >=RUNJOB[kc2]->priority)  kc1=kc2;
        }
        if(RUNJOB[k  ]->priority
         <=RUNJOB[kc1]->priority)  break;
        t=RUNJOB[k];
        RUNJOB[k]=RUNJOB[kc1];
        RUNJOB[kc1]=t;
        k=kc1;
    }
    return rj;
}

static void TfJobRefDown(jobstruct *JOB) 
{
    int x;
    x = my_atomic_fetch_add(&(JOB->ref),-1);
    if(x!=0) return;
    TfPutJob(JOB);
}

static void LcFreeThread(int thread) 
{
    tfthread[thread]=firstfreethread;
    firstfreethread=thread;
}

/* ====****====**** pthread stuff *****====*****/

/* the thread structures themselves      */
pthread_t * mythread;

/* condition variable that the workers   */
/* wait on when they are idle            */
pthread_cond_t * wakeworker;

static void LcWorkerPause(int thread)
{
    pthread_cond_wait(wakeworker+thread,&dodgy);
}

static void LcWorkerKick(int thread)
{
    pthread_cond_signal(wakeworker+thread);
}

static void LcStartJob(jobstruct * JOB, int thread)
{
    tfparms[thread].JOB=JOB;
    LcWorkerKick(thread); 
}

/* ====== Now the code for the routines  =======*/

void tfdo(int proggyno, MOJ * p);

static jobstruct *LcQThread(parmstruct *pp)
{
    for (;;) {
        pp->JOB=NULL;
        while(pp->JOB==NULL)
        {
            LcFreeThread(pp->threadno);
            LcWorkerPause(pp->threadno);
        }
        return pp->JOB;
    } 
}

static jobstruct *GetJob(parmstruct *pp)  // may be 2
{
    int i;
    jobstruct * JOB;
    Lock();
    i = my_atomic_fetch_add(&runjobs, -1);
    if(i>=0)
    {
        JOB=LcNextRun();
        Unlock();
        return JOB;
    }
    JOB=LcQThread(pp);
    Unlock();
    return JOB;
}

/*  The worker thread  */
static void *worker(void *p)
{
    int nparm;
    jobstruct * JOB;
    parmstruct * pp;
    int proggyno;
    MOJ parms[10];
    int i;
    pp = (parmstruct *) p;

    for (;;) {
        JOB=GetJob(pp);
        if(((uint64_t)JOB)==2) break;
        proggyno=JOB->proggyno;
        nparm=JOB->nparm;
        for(i=0;i<nparm;i++) parms[i]=JOB->parm[i];
        FreeJob(JOB);
        tfdo(proggyno,parms);
        for(i=0;i<nparm;i++) TFRelease(parms[i]);
        TFDownJobs();
    }
    return NULL;
}

void * TFPointer(MOJ mj)
{
    return mj->mem;   // read only at this point
}

void * TFAllocate(MOJ mj, size_t bytes)
{
    void * ptr;
    ptr = AlignTalloc(bytes);
    if(ptr==NULL)
    {
        printf("memory failure for moj of %ld\n",bytes);
        exit(1);
    }
    mj->mem=ptr;   // private at this point
    return ptr;
}

void TFSetPtr(MOJ mj, void * ptr)
{
    mj->mem=ptr;   // private at this point
}

void TFRelease(MOJ mj)
{
    int i;

    for (;;) {
        if(stopfree==0) break;    // atomic fetch
        wait(100);
    }
    i = my_atomic_fetch_add(&(mj->refc),-1);
    if(i>0) return;
    if(i<0)
    {
        printf("Refcount went negative!\n");
        return;
    }
    if(mj->refc!=0) return;
    if(mj->mem!=NULL) AlignTree(mj->mem);
    mj->mem=NULL;
    mj->RDL=NULL;   // it is dead now  ?? delete this line ?
//  ought to return the MOJ to the list
}

MOJ TFNewMOJ(void)
{
    MOJ newone;
    Lock();
    if((nfmoj+1)>=nmojes)
    {
        printf("Run out of MOJs\n");
        exit(1);
    }
    newone = tfmoj+nfmoj;
    nfmoj++;
    Unlock();
    newone->mem=NULL;
    newone->RDL=NULL;
    atomic_init(&newone->refc, 0);
    return newone;
}

void TFStable (MOJ mj)
{
    rdlstruct * RDL;
    Lock();
    for (;;) {
        RDL=(rdlstruct *) TfLinkClose((uint64_t *)&(mj->RDL));
        if(RDL==NULL) break;
        Unlock();
        TfJobRefDown(RDL->JOB);
        Lock();
        LcFreeRdl(RDL);
    }
    LcMainKick();        // it may be waiting for this moj
    Unlock();
    return;
}


void TFQuickReady(MOJ moj)
{
    TfLinkClose((uint64_t *)(&(moj->RDL)));
}

void TFGetReadRef(MOJ moj)
{
    UpMojRef(moj);
}

// now the calls that only the upper level can use

void TFClose(void)
{
    uint64_t * FRE;
    uint64_t nfree,i;
    uint64_t fakejob;
    int thread;
    fakejob=2;                  // instruction to shut down thread
    for(i=0;i<nothreads;i++)
    {
        Lock();
        thread=TfUnQThread((jobstruct *)fakejob);
        pthread_join(*(mythread+thread),NULL);
    }
    FRE=(uint64_t *)(*TFM);
    nfree=*FRE;
    for(i=1;i<=nfree;i++) AlignTree((void *)FRE[i]);
    free(FRE);
    free(TFM);
}

void TFInit(unsigned int threads)
{
  unsigned int i;
  unsigned int jobx,rdlx;
  uint64_t * FRE;
  jobstruct * tfjob;
  rdlstruct * tfrdl;
  MOJ mj;
  jobx=SCALE*12;
  nmojes=jobx*2;
  atomic_init(&closejobs, 0);
  TFM=malloc(TFMSIZE*sizeof(uint64_t));
  FRE=malloc(FRESIZE*sizeof(uint64_t));
  *(TFM+TFMFRE)=(uint64_t)FRE;
  *FRE=0;      // none yet to free
  // following is temporary fix for zpe needs.
  // redesign it better for V2
  rdlx=3*nmojes;
  tfjob=AlignTalloc(jobx*sizeof(jobstruct));
  append(FRE,(uint64_t)tfjob);
  *(TFM+TFMJOB)=0;     // freechain of jobs empty
  for(i=0;i<jobx;i++) TfLinkIn(TFM+TFMJOB,(uint64_t *) (tfjob+i));
  tfrdl=AlignTalloc(rdlx*sizeof(rdlstruct));
  append(FRE,(uint64_t)tfrdl);
  *(TFM+TFMRDL)=0;     // freechain of jobs empty
  for(i=0;i<rdlx;i++) TfLinkIn(TFM+TFMRDL,(uint64_t *) (tfrdl+i));
  RUNJOB=AlignTalloc(jobx*sizeof(jobstruct *));
  append(FRE,(uint64_t)RUNJOB);
  nfmoj=0;
  atomic_init(&runjobs, 0);
  atomic_init(&stopfree, 0);

  /* Initialize the moj data  */
  tfmoj=AlignTalloc(nmojes*sizeof(mojstruct));
  append(FRE,(uint64_t)tfmoj);
  for(i=0;i<nmojes;i++)
    {
      mj=tfmoj+i;
      mj->mem=NULL;
      atomic_init(&mj->refc, 0);     // This is private, so OK.
      mj->RDL=NULL;
    }
  /* Initialize the thread data  */
  firstfreethread=-1;  // no free threads yet
  tfthread=AlignTalloc(threads*sizeof(int));
  append(FRE,(uint64_t)tfthread);
  tfparms =AlignTalloc(threads*sizeof(parmstruct));
  append(FRE,(uint64_t)tfparms);
  for(i=0;i<threads;i++) tfparms[i].JOB=(jobstruct *)3;  // so close works
  nothreads=threads;
  mythread=AlignTalloc(threads*sizeof(pthread_t));
  append(FRE,(uint64_t)mythread);
  wakeworker=AlignTalloc(threads*sizeof(pthread_cond_t));
  append(FRE,(uint64_t)wakeworker);

  /* initialize the condition variables for the workers */
  for(i=0;i<threads;i++) pthread_cond_init(wakeworker+i,NULL);
  jobsready=0;    // no jobs ready to run
  /* get the lock so that nothing does   */
  /* any damage while we are starting up         */
  Lock();
  /* start all the worker threads                */
  for(i=0;i<nothreads;i++)
    {
      tfparms[i].threadno=i;
      pthread_create(mythread+i,NULL, worker, tfparms+i);
    }
  /* all is ready - now let's go                 */
  Unlock();
  return;
}

void TFWait(MOJ moj)
{
    Lock();
    for (;;) {
        if(((uint64_t)(moj->RDL))==2)
        {
            Unlock();
            return;
        }
        LcMainPause();    
    }
}

void TFSubmit(int priority, int proggyno, ...)
{
    va_list va;
    MOJ moj;
    jobstruct * JOB;
    MOJ * mojadd;
    va_start(va,proggyno);
    JOB=NewJob();
    JobPop(JOB, proggyno, priority);
    for (;;) {
        moj=va_arg(va,MOJ);
        if((long)moj==-1) break;
        if((long)moj==-2)
        {
            mojadd=va_arg(va,MOJ*);
            moj=TFNewMOJ();
            *mojadd=moj;              // writing
            UpMojRef(moj);    // for the proggy
        }
        else
        {
            Lock();
            LCBindRdl(JOB,moj);
            Unlock();
        }
        JOB->parm[JOB->nparm++]=moj;    // new parameters
    }
    va_end(va);
    TFUpJobs();
    TfJobRefDown(JOB);
}

/*
 * Looks like the compare and exchange is done by
 *     int expected = -1;
 *     if (atomic_compare_exchange_strong(ptr, &expected, expected)) {
 *       Spin Lock
 *       while (-1 == expected) {
 *         C pause
 *         atomic_compare_exchange_strong(ptr, &expected, expected);
 *       }
 *     }
 *     Critical section
 *     Fiddle about using atomic_store
 */
int TfLinkIn(uint64_t *chain, uint64_t *ours)
{
  uint64_t state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
  for (;;) {
    if (1 == state) {
      /* Someone else has the lock */
      for (;;) {
        state = atomic_load((atomic_uint_least64_t *)chain);
        if (1 != state) {
          break;
        }
        wait(10); /* Short pause */
      }
      /* We can have the lock, so try again */
      state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
    }
    if (1 != state) {
      /* We got it */
      break;
    }
  }
  /* We have the lock */
  if (2 == state) {
    /* List closed */
    atomic_store((atomic_uint_least64_t *)chain, 2); /* Unlock and we're done */
    return 2;
  } else {
    /* List not closed */
    atomic_store((atomic_uint_least64_t *)ours, state); /* chain from ours onwards */
    atomic_store((atomic_uint_least64_t *)chain, (uint64_t)ours); /* unlock and chain ours in */
    return 0;
  }
}

uint64_t *TfLinkOut(uint64_t *chain)
{
  uint64_t state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
  for (;;) {
    if (1 == state) {
      /* Someone else has the lock */
      for (;;) {
        state = atomic_load((atomic_uint_least64_t *)chain);
        if (1 != state) {
          break;
        }
        wait(10); /* Short pause */
      }
      /* We can have the lock, so try again */
      state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
    }
    if (1 != state) {
      /* We got it */
      break;
    }
  }
  /* We have the lock */
  if (0 == state || 2 == state) {
    /* Empty  or closed */
    atomic_store((atomic_uint_least64_t *)chain, state); /* Unlock as empty or closed and we're done */
    return (uint64_t *)state;
  } else {
    /* List not closed or empty */
    uint64_t *ours = (uint64_t *)state;
    uint64_t new = *ours; /* Follow the link */
    atomic_store((atomic_uint_least64_t *)chain, new); /* unlock and chain next in */
    return ours;
  }
}

uint64_t *TfLinkClose(uint64_t *chain)
{
  uint64_t state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
  for (;;) {
    if (1 == state) {
      /* Someone else has the lock */
      for (;;) {
        state = atomic_load((atomic_uint_least64_t *)chain);
        if (1 != state) {
          break;
        }
        wait(10); /* Short pause */
      }
      /* We can have the lock, so try again */
      state = atomic_exchange((atomic_uint_least64_t *)chain, 1);
    }
    if (1 != state) {
      /* We got it */
      break;
    }
  }
  /* We have the lock */
  if (0 == state) {
    /* Empty, we need to close it */
    atomic_store((atomic_uint_least64_t *)chain, 2);
    return NULL;
  } else if (2 == state) {
    /* closed already */
    atomic_store((atomic_uint_least64_t *)chain, state); /* Close it and unlock */
    return (uint64_t *)state;
  } else {
    /* List not closed or empty */
    uint64_t *ours = (uint64_t *)state;
    uint64_t new = *ours; /* Follow the link */
    atomic_store((atomic_uint_least64_t *)chain, new); /* unlock and chain next in */
    return ours;
  }
}

