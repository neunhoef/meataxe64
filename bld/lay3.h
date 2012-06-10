/*
         lay3.h   -   Layer 3 header - multi-thread scheduler
         =======      R. A. Parker    15.02.2012
*/

#define MOJMAX 4000
#define JOBMAX 2000
#define THREADS 4
#define DIE -7

/*  comment or uncomment this for scheduler debug data */
/* #define DEBUG 1 */

char * cachemalloc(size_t bytes);
void cachefree(char * ptr);

/* first the three that all can use  */

char * lay3ptr(int moj);
char * lay3mem(int moj, size_t bytes);
void   lay3release(int moj);

/* now the five that only the layer 4 can call  */

int  lay3initialize(void);
int  lay3newmoj(void);
void lay3deref(int moj);
void lay3waitmoj(int moj);
void lay3submit(int proggy, int * inputs, int * outputs);
void lay3shutdown(void);

/* end of lay3.h  */
