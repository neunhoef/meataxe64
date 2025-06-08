/*
 * $Id: mt_types.h,v 1.1 2005/10/28 20:56:57 jon Exp $
 *
 * Fundamental types for meataxe for ia64
 *
 */

#ifndef included__mt_types
#define included__mt_types

typedef unsigned short u32; /* Check, might be int */
typedef short s32; /* Check, might be int */
typedef unsigned int u64; /* Check, might be long */
typedef int s64; /* Check, might be long */

#define PRIME_MASK 0x3fffffff
#define PRIME_BITS 0xc0000000

#ifdef EIGHT_BYTE_WORD
typedef u64 word;
#define PRIME_BIT 0x40000000
#else
typedef u32 word;
#define PRIME_BIT 0
#endif

#define U32PRIME_BIT 0
#define U64PRIME_BIT 0x40000000

#endif
