/*
 * $Id: header.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Internal header for meataxe
 *
 */

#ifndef included__header
#define included__header

typedef struct header_struct *header;

extern unsigned int header_get_prime(header);
extern void header_set_prime(header, unsigned int);
extern unsigned int header_get_nob(header);
extern void header_set_nob(header, unsigned int);
extern unsigned int header_get_nod(header);
extern void header_set_nod(header, unsigned int);
extern unsigned int header_get_nor(header);
extern void header_set_nor(header, unsigned int);
extern unsigned int header_get_noc(header);
extern void header_set_noc(header, unsigned int);
extern header header_alloc(void);
extern void header_free(header);
extern header header_create(unsigned int prime, unsigned int nob,
                            unsigned int nod, unsigned int noc,
                            unsigned int nor);

#endif
