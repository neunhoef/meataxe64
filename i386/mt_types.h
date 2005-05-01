/*
 * $Id: mt_types.h,v 1.1 2005/05/01 10:10:30 jon Exp $
 *
 * Fundamental types for meataxe
 *
 */

#ifndef included__mt_types
#define included__mt_types

typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;

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
