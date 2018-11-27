/*    Meataxe-64    pcrit.h     */
/*    ==========    ========     */

/*    R. A. Parker      2.11.2017 */

extern void __attribute__((sysv_abi)) mactype(char * mact);
extern void hpmiset(FIELD * f);
extern uint64_t pcstride(uint64_t s);
extern uint64_t __attribute__((sysv_abi)) pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
extern void __attribute__((sysv_abi)) pcaxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void __attribute__((sysv_abi)) pcjxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void __attribute__((sysv_abi)) pcbif(Dfmt * d, const Dfmt * s1, const Dfmt * s2,
                   uint64_t nob, const uint8_t * t2);
extern void __attribute__((sysv_abi)) pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2);
extern void __attribute__((sysv_abi)) pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1);
extern void __attribute__((sysv_abi)) pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1);

void __attribute__((sysv_abi)) pcab2(const uint8_t *a, uint8_t * bv, uint8_t *c);
void __attribute__((sysv_abi)) pcjb2(const uint8_t *a, uint8_t * bv, uint8_t *c);
void __attribute__((sysv_abi)) pcad3(const uint8_t *a, const uint8_t *b, uint8_t * c);
void __attribute__((sysv_abi)) pcab3(const uint8_t *a, uint8_t * bv, uint8_t *c);
void __attribute__((sysv_abi)) pcjb3(const uint8_t *a, uint8_t * bv, uint8_t *c);
void __attribute__((sysv_abi)) pcaas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void __attribute__((sysv_abi)) pcdas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void __attribute__((sysv_abi)) pcjas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void __attribute__((sysv_abi)) pcjat(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void __attribute__((sysv_abi)) pcdasc(const uint8_t *prog, uint8_t * bv, const uint64_t * parms);
void __attribute__((sysv_abi)) pccl32 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint32_t * d1, uint32_t * d2);
void __attribute__((sysv_abi)) pccl64 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint64_t * d1, uint64_t * d2);
// void pcbmdq(const uint8_t *a, uint8_t * bw, uint8_t *c,
//             const uint64_t * parms);

/* end of pcrit.h  */
