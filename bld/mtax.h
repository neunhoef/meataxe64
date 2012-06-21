/*
  mtax.h   -   Meataxe-64 General Header
  ======       R. A. Parker     14.02.2012
*/

#define NOT_USED(_x) (void)(_x)

#define CACHELINE 128

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long long uint64;

typedef uint64 FELT;
typedef char *Afmt;
typedef char *Bfmt;
typedef char *Cfmt;
typedef char *Dfmt;
typedef char *Brick;

typedef uint64 FIELD;

#define FDEF     0
#define CHARC    1
#define POW      2
#define CONP     3
#define ALC     16
#define CAUL    17
#define BCAUL   18
#define FGC     19
#define FSL     20

typedef struct
{
  FIELD *field;     /* the field in use */
  uint64 noc;       /* Dimension of the space. */
  uint64 nob;       /* Number of bytes for a row of Dfmt in memory. */
  uint64 paktyp;    /* how FIELD elements are packed in Dfmt. */
} DSPACE;

/* now the function prototypes  First the simple field stuff */

size_t LenField(uint64 fdef);
int  FieldSet(uint64 fdef, FIELD *f);
FELT FieldAdd(FIELD *f, FELT a, FELT b);
FELT FieldNeg(FIELD *f, FELT a);
FELT FieldSub(FIELD *f, FELT a, FELT b);
FELT FieldMul(FIELD *f, FELT a, FELT b);
FELT FieldInv(FIELD *f, FELT a);
FELT FieldDiv(FIELD *f, FELT a, FELT b);

/* The D-format things in field.c  */

void DSSet(FIELD *f, uint64 noc, DSPACE *ds);

FILE *RdHdr(const char *fname, uint64 *fdef, uint64 *nor, uint64 *noc);
FILE *WrHdr(const char *fname, uint64 fdef, uint64 nor, uint64 noc);
void RdMatrix(FILE *f, DSPACE *ds, uint64 nor, Dfmt d);
void WrMatrix(FILE *f, DSPACE *ds, uint64 nor, Dfmt d);
void Close(FILE *f);

FELT DUnpak(DSPACE *ds, uint64 col, Dfmt d);
void DPak(DSPACE *ds, uint64 col, Dfmt d, FELT f);
void DAdd(DSPACE *ds, Dfmt d1, Dfmt d2);
void DSMul(DSPACE *ds, FELT f, Dfmt d);
void DMove(DSPACE *ds, Dfmt d1, Dfmt d2);
void DCut(DSPACE *ds1, uint64 nor, uint64 col,
          Dfmt d1, DSPACE *ds2, Dfmt d2);
void DPaste(DSPACE *ds1, Dfmt d1, uint64 nor, uint64 col,
            DSPACE *ds2, Dfmt d2);

/* the HPMI routines  */

size_t LenHpmi(uint64 fdef, uint64 rowa, uint64 cola, uint64 colb);
int HpmiSet(uint64 fdef, uint64 rowa, uint64 cola, 
            uint64 colb, FIELD *f);
size_t LenBrick(FIELD *f);
void BrickMad(FIELD *f, Afmt a, Brick bk, Cfmt c);
void BrickPop(FIELD *f, Bfmt b, uint64 cauldron, 
              uint64 bstride, Brick bk);
size_t LenA(FIELD *f, uint64 nor);
size_t DtoA(FIELD *f, DSPACE *ds, Dfmt d, uint64 nor,
            uint64 alcove, Afmt a);
size_t LenB(FIELD *f, uint64 nor, uint64 noc);
void DtoB(FIELD *f, DSPACE *ds, Dfmt d, uint64 nor, Bfmt b);
void CtoD(FIELD *f, DSPACE *ds, Cfmt c, uint64 nor, Dfmt d);
void ZerC(FIELD *f, Cfmt c, uint64 cauldrons, uint64 nor);

/* end of mtax.h  */
