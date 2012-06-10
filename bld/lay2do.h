/*   lay2do.h - Just do it interface  */
/*   R. A. Parker  14.02.2012         */

#define CVA 1
#define CVB 2
#define MUL 3
#define CVD 4

void lay2do(int prog, int * inp, int * out);

void pgcva(int f, int d, int af);
void pgcvb(int f, int d, int bf);
void pgmul(int f, int s, int a, int b, int c);
void pgcvd(int f, int c, int d);

/* end of lay2do.h  */
