/*
 * $Id: header.h,v 1.8 2002/03/10 22:45:28 jon Exp $
 *
 * Internal header for meataxe
 *
 */

#ifndef included__header
#define included__header

typedef struct header_struct header;

extern unsigned int header_get_prime(const header *);
extern void header_set_prime(header *, unsigned int);
extern unsigned int header_get_nob(const header *);
extern void header_set_nob(header *, unsigned int);
extern unsigned int header_get_nod(const header *);
extern void header_set_nod(header *, unsigned int);
extern unsigned int header_get_nor(const header *);
extern void header_set_nor(header *, unsigned int);
extern unsigned int header_get_noc(const header *);
extern void header_set_noc(header *, unsigned int);
extern unsigned int header_get_len(const header *);
extern void header_set_len(header *);
extern unsigned int header_get_blen(const header *);
extern void header_set_blen(header *);
extern unsigned int header_get_eperb(const header *);
extern void header_set_eperb(header *);
extern int header_alloc(header **);
extern void header_free(const header *);
extern header *header_create(unsigned int prime, unsigned int nob,
                             unsigned int nod, unsigned int noc,
                             unsigned int nor);
extern int header_compare(const header *, const header *);

#endif
