#ifndef _CBASICS
#define _CBASICS

#include "types.h"

#include "rstl/RstlVersions.h"

namespace CBasics {
bool Init();
char* Stringize(const char* fmt, ...);
// Guessed names. Cache-block (dcbz) accelerated fill/copy; G2ME01 0x8028BD3C/0x8028BE1C, next to
// CBasics::Init in Prime Trilogy.
void ZeroMemory(void* dst, uint size);
void CopyMemory(void* dst, const void* src, uint size);

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
#endif

#if RSTL_KYOTO_CONST_SCALAR_RETURNS
// Convert between serialized big-endian values and host byte order.
inline const int SwapBytes(int x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const uint SwapBytes(const uint x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const float SwapBytes(float x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const short SwapBytes(const short x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const ushort SwapBytes(const ushort x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

inline const u64 SwapBytes(u64 x) {
#if TARGET_BIG_ENDIAN
  return x;
#else
  return detail::SwapBytes(x);
#endif
}

#else
// Convert between serialized big-endian values and host byte order. The big-endian bodies keep
// their original shape; changing them alters code generation in console callers.
inline uint SwapBytes(uint x) {
#if TARGET_LITTLE_ENDIAN
  x = detail::SwapBytes(x);
#endif
  return x;
}
#if RSTL_KYOTO_PROTO_SOURCE_FORMS
inline int SwapBytes(int x) {
#else
inline const int SwapBytes(int x) {
#endif
#if TARGET_LITTLE_ENDIAN
  x = detail::SwapBytes(x);
#endif
  return x;
}
inline short SwapBytes(short x) {
#if TARGET_LITTLE_ENDIAN
  x = detail::SwapBytes(x);
#endif
  return x;
}
inline ushort SwapBytes(ushort x) {
#if TARGET_LITTLE_ENDIAN
  x = detail::SwapBytes(x);
#endif
  return x;
}
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
inline u64 SwapBytes(u64 x) {
#if TARGET_LITTLE_ENDIAN
  x = detail::SwapBytes(x);
#endif
  return x;
}

#endif

} // namespace CBasics

// Big-endian targets read the bare operand: even an inline SwapBytes call changes code generation.
#if TARGET_LITTLE_ENDIAN
#define BE_SWAP(x) CBasics::SwapBytes(x)
#else
#define BE_SWAP(x) (x)
#endif

#endif // _CBASICS
