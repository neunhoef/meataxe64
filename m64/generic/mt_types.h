/*
 * $Id: mt_types.h,v 1.5 2014/06/11 07:56:19 jon Exp $
 *
 * Fundamental types for meataxe for em64t
 *
 */

#ifndef included__mt_types
#define included__mt_types

#include <stdint.h>

/* Format types */

#define U32_F "u"
#define S32_F "d"
#define U64_F "lu"
#define S64_F "ld"
#define SIZE_F "lu"

typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

#define PRIME_MASK 0x3fffffff
#define PRIME_BITS 0xc0000000

typedef u64 word;
#define W_F U64_F
#define PRIME_BIT 0x40000000

#endif
