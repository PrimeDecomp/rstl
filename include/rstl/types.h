#ifndef _TYPES
#define _TYPES

// TODO: correct place
#define RS_ASSERT(cond, msg) ((void)(0))

#include "rstl/RstlVersions.h"

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: NULL is being defined as 0 or 0L depending on the inclusion order, and different games are breaking in different ways.
// Corruption proto wants 0L, but prime/echoes wants 0 due to reserved_vector constructor.
#if RSTL_CORRUPTION_STANDALONE
#include <stddef.h>
#endif
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
