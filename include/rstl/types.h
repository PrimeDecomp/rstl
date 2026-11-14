#ifndef _TYPES
#define _TYPES

// TODO: correct place
#define RS_ASSERT(cond, msg) ((void)(0))

#ifdef __cplusplus
extern "C" {
#endif

#include <dolphin/types.h>

// Dolphin u32 is unsigned long
typedef unsigned int uint;
typedef signed short sshort;
typedef unsigned short ushort;
typedef signed char schar;
typedef unsigned char uchar;

// Pointer to unknown, to be determined at a later date.
typedef void* unkptr;

#ifdef __cplusplus
}
#endif

#endif // _TYPES
