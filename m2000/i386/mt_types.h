/*
 * $Id: mt_types.h,v 1.6 2014/06/11 07:56:19 jon Exp $
 *
 * Fundamental types for meataxe for x86
 *
 */

#ifndef included__mt_types
#define included__mt_types

/* Format types */

#define U32_F "u"
#define S32_F "d"
#define SIZE_F "u"

typedef unsigned int u32;
typedef int s32;
#if defined(WIN32)
typedef unsigned long u64;
typedef long s64;
#define U64_F "lu"
#define S64_F "ld"
#else
typedef unsigned long long u64;
typedef long long s64;
#define U64_F "llu"
#define S64_F "lld"
#endif

#define PRIME_MASK 0x3fffffff
#define PRIME_BITS 0xc0000000

#ifdef EIGHT_BYTE_WORD
typedef u64 word;
#define W_F U64_F
#define PRIME_BIT 0x40000000
#else
typedef u32 word;
#define W_F U32_F
#define PRIME_BIT 0
#endif

#define U32PRIME_BIT 0
#define U64PRIME_BIT 0x40000000

/* The maximum we could try to allocate */
#define SIZE_T_MAX ULONG_MAX

#endif
