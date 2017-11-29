/*
         slab.h  -   Slab Routines Header
         ======      R. A. Parker 12.5.2016
*/

extern void FieldSet (uint64_t fdef, FIELD * f);
extern int  FieldSet1(uint64_t fdef, FIELD * f, int flags);

extern void SLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
                  Dfmt * c, uint64_t nora, uint64_t noca, uint64_t nocb);

extern void SLTra(const FIELD *f, const Dfmt *a, Dfmt *b,
                  uint64_t nora, uint64_t noca);

uint64_t SLEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor);
uint64_t SLEchS(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor);
uint64_t SLSize (FIELD * f, uint64_t nor, uint64_t noc);
uint64_t SLSizeM(FIELD * f, uint64_t nor, uint64_t noc);
uint64_t SLSizeC(FIELD * f, uint64_t nor, uint64_t noc);
uint64_t SLSizeR(FIELD * f, uint64_t nor, uint64_t noc);

/* internal interface - called from linf */

extern void PLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
                  Dfmt * c, uint64_t nora, uint64_t noca, uint64_t nocb);

/* end of slab.h  */
