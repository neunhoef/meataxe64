/*
 * $Id: header.h,v 1.11 2021/08/02 18:19:40 jon Exp $
 *
 * Internal header for meataxe
 *
 */

#ifndef included__header
#define included__header

typedef struct header_struct header;

extern u32 header_get_prime(const header *);
extern u32 header_get_raw_prime(const header *);
extern void header_set_prime(header *, u32);
extern void header_set_raw_prime(header *, u32);
extern u32 header_get_nob(const header *);
extern void header_set_nob(header *, u32);
extern u32 header_get_nod(const header *);
extern void header_set_nod(header *, u32);
extern u32 header_get_nor(const header *);
extern void header_set_nor(header *, u32);
extern u32 header_get_noc(const header *);
extern void header_set_noc(header *, u32);
extern u32 compute_len(u32 nob, u32 noc);
extern u32 padded_cols(u32 cols, u32 nob);
extern u32 header_get_len(const header *);
extern u32 header_get_u32_len(const header *);
extern u32 header_get_u64_len(const header *);
extern void header_set_len(header *);
extern u32 header_get_blen(const header *);
extern void header_set_blen(header *);
extern u32 header_get_eperb(const header *);
extern void header_set_eperb(header *);
extern int header_alloc(header **);
extern void header_free(const header *);
extern header *header_create(u32 prime, u32 nob,
                             u32 nod, u32 noc,
                             u32 nor);
extern int header_check(const header *h); /* Check for correct size setting */

#endif
