#ifndef _CBASICS
#define _CBASICS

#include "types.h"

#include "rstl/RstlVersions.h"

#include <stdarg.h>

namespace CBasics {
bool Init();
char* Stringize(const char* fmt, va_list args);
char* Stringize(const char* fmt, ...);
// Guessed names. Cache-block (dcbz) accelerated fill/copy; G2ME01 0x8028BD3C/0x8028BE1C, next to
// CBasics::Init in Prime Trilogy.
void ZeroMemory(void* dst, uint size);
void CopyMemory(void* dst, const void* src, uint size);

// Convert between serialized big-endian values and host byte order.
#if TARGET_LITTLE_ENDIAN
namespace detail {
template < typename T >
inline T SwapBytes(T value) {
  uchar* bytes = reinterpret_cast< uchar* >(&value);
  for (uint i = 0; i < sizeof(T) / 2; ++i) {
    const uchar byte = bytes[i];
    bytes[i] = bytes[sizeof(T) - 1 - i];
    bytes[sizeof(T) - 1 - i] = byte;
  }
  return value;
}
} // namespace detail
#define CBASICS_SWAPPED(x) detail::SwapBytes(x)
#else
#define CBASICS_SWAPPED(x) (x)
#endif

// Where the declarations say const changes code generation. Prime returns every scalar const and
// also takes uint, short and ushort as const parameters; Echoes returns only int const; the
// prototype says const nowhere.
#if RSTL_KYOTO_CONST_SCALAR_RETURNS
#define CBASICS_PRIME_CONST const
#else
#define CBASICS_PRIME_CONST
#endif
#if RSTL_KYOTO_PROTO_SOURCE_FORMS
#define CBASICS_INT_CONST
#else
#define CBASICS_INT_CONST const
#endif
#define CBASICS_SWAP_BYTES(returnConst, T, parameterConst)                                         \
  inline returnConst T SwapBytes(parameterConst T x) { return CBASICS_SWAPPED(x); }

CBASICS_SWAP_BYTES(CBASICS_INT_CONST, int, )
CBASICS_SWAP_BYTES(CBASICS_PRIME_CONST, uint, CBASICS_PRIME_CONST)
CBASICS_SWAP_BYTES(CBASICS_PRIME_CONST, short, CBASICS_PRIME_CONST)
CBASICS_SWAP_BYTES(CBASICS_PRIME_CONST, ushort, CBASICS_PRIME_CONST)
CBASICS_SWAP_BYTES(CBASICS_PRIME_CONST, u64, )
#if RSTL_KYOTO_CONST_SCALAR_RETURNS
CBASICS_SWAP_BYTES(const, float, )
#else
// Echoes and the prototype swap a float through its bits. Even on big-endian targets the body
// keeps that shape: a plain return changes code generation in console callers.
inline float SwapBytes(float x) {
  union {
    float f;
    uint u;
  };
  f = x;
#if TARGET_LITTLE_ENDIAN
  u = SwapBytes(u);
#endif
  return f;
}
#endif

#undef CBASICS_SWAP_BYTES
#undef CBASICS_INT_CONST
#undef CBASICS_PRIME_CONST
#undef CBASICS_SWAPPED

} // namespace CBasics

// Big-endian targets read the bare operand: even an inline SwapBytes call changes code generation.
#if TARGET_LITTLE_ENDIAN
#define BE_SWAP(x) CBasics::SwapBytes(x)
#else
#define BE_SWAP(x) (x)
#endif

#endif // _CBASICS
