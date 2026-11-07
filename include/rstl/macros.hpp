#ifndef _RSTL_MACROS
#define _RSTL_MACROS

// Compiler helpers RSTL and Kyoto use. A game or SDK header may define the same macros; these only
// fill in the ones it leaves out.

#define ALIGN_UP(x, a) (((x) + (a - 1)) & ~(a - 1))

#ifndef AUTO
#if (defined(__cplusplus) && __cplusplus >= 201103L) || defined(__clang__)
// Use C++11 auto keyword
#define AUTO(name, val) auto name = val
#define AUTO_REF(name, val) auto& name = val
#define AUTO_CONST_REF(name, val) const auto& name = val
#else
// Use __typeof__ extension
#define AUTO(name, val) __typeof__(val) name = val
#define AUTO_REF(name, val) __typeof__(val)& name = val
#define AUTO_CONST_REF(name, val) const __typeof__(val)& name = val
#endif
#endif

#ifndef ATTRIBUTE_ALIGN
#if defined(__MWERKS__) || defined(__GNUC__)
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#elif defined(_MSC_VER)
#define ATTRIBUTE_ALIGN(num) __declspec(align(num))
#else
#error unknown compiler
#endif
#endif

#ifndef ATTRIBUTE_ALIGN_DECL
// Older CodeWarrior versions require the alignment attribute after the declarator.
#if defined(__MWERKS__)
#define ATTRIBUTE_ALIGN_DECL(num, decl) decl ATTRIBUTE_ALIGN(num)
#else
#define ATTRIBUTE_ALIGN_DECL(num, decl) ATTRIBUTE_ALIGN(num) decl
#endif
#endif

#endif // _RSTL_MACROS
