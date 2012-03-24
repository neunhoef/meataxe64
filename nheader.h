/*
 * $Id: nheader.h,v 1.1 2012/03/24 13:32:21 jon Exp $
 *
 * Internal header for meataxe 64
 *
 */

#ifndef included__nheader
#define included__nheader

typedef struct nheader_struct nheader;

extern u64 nheader_get_prime(const nheader *);
extern u64 nheader_get_raw_prime(const nheader *);
extern void nheader_set_prime(nheader *, u64);
extern void nheader_set_raw_prime(nheader *, u64);
extern u64 nheader_get_nob(const nheader *);
extern void nheader_set_nob(nheader *, u64);
extern u64 nheader_get_nod(const nheader *);
extern void nheader_set_nod(nheader *, u64);
extern u64 nheader_get_nor(const nheader *);
extern void nheader_set_nor(nheader *, u64);
extern u64 nheader_get_noc(const nheader *);
extern void nheader_set_noc(nheader *, u64);
extern u64 nheader_compute_len(u64 nob, u64 noc);
extern u64 nheader_get_len(const nheader *);
extern u64 nheader_get_u64_len(const nheader *);
extern u64 nheader_get_u64_len(const nheader *);
extern void nheader_set_len(nheader *);
extern int nheader_alloc(nheader **);
extern void nheader_free(const nheader *);
extern nheader *nheader_create(u64 prime, u64 nob,
                               u64 nod, u64 noc,
                               u64 nor);
extern int nheader_check(const nheader *h); /* Check for correct size setting */

#endif
