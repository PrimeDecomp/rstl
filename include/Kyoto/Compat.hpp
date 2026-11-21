#ifndef _COMPAT_H_
#define _COMPAT_H_

#include "Kyoto/Platform.h"

#if IS_TARGET_PC
#define MEMORY_SYS_ARGUMENT_DECL
#define MEMORY_SYS_ARGUMENT_VAL
#else
#define MEMORY_SYS_ARGUMENT_DECL , CMemorySys& memorySys
#define MEMORY_SYS_ARGUMENT_VAL  , memorySys
#endif

#endif // _COMPAT_H_
