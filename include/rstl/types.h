#ifndef _TYPES
#define _TYPES

// TODO: correct place
#define RS_ASSERT(cond, msg) ((void)(0))

#ifdef __cplusplus
extern "C" {
#endif

// stddef.h first: its NULL (0L) is what dolphin/types.h's nullptr falls back to; with NULL as plain 0,
// vector<T*>(n, nullptr) picks the iterator-range constructor.
#include <stddef.h>
#include <dolphin/types.h>

// Dolphin u32 is unsigned long
typedef unsigned int uint;
typedef signed short sshort;
typedef unsigned short ushort;
typedef signed char schar;
typedef unsigned char uchar;

// Used by Retro asserts strings. Avoid using otherwise.
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned char uint8;

// Pointer to unknown, to be determined at a later date.
typedef void* unkptr;

#ifdef __cplusplus
}
#endif

#endif // _TYPES
