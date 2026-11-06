#ifndef _PLATFORM_H
#define _PLATFORM_H

#if RSTL_PRIME1
#define IF_RSTL_PRIME1(...) __VA_ARGS__
#define IF_NOT_RSTL_PRIME1(...) 
#define IF_RSTL_PRIME1_ELSE(A, B) A
#else
#define IF_RSTL_PRIME1(...)
#define IF_NOT_RSTL_PRIME1(...) __VA_ARGS__
#define IF_RSTL_PRIME1_ELSE(A, B) B
#endif

#if defined(TARGET_PC)

#define IS_TARGET_PC 1
#define IF_TARGET_PC_OR(A, B) (A)
#define IF_TARGET_PC(...) __VA_ARGS__
#define IF_NOT_TARGET_PC(...)

#else

#define IS_TARGET_PC 0
#define IF_TARGET_PC_OR(A, B) (B)
#define IF_TARGET_PC(...)
#define IF_NOT_TARGET_PC(...) __VA_ARGS__

#endif

#endif // _PLATFORM_H
