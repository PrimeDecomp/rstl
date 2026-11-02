#ifndef _CVECTOR2I
#define _CVECTOR2I

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/RstlVersions.h"
#include "types.h"
#include "rstl/construct.hpp"

class CVector2i {
public:
  CVector2i() : mX(0), mY(0) {}
  CVector2i(int x, int y = 0);
#if RSTL_PRIME1
  CVector2i(CInputStream& in) {
    mX = in.ReadLong();
    mY = in.ReadLong();
  }
#endif

  int GetX() const { return mX; }
  void SetX(RSTL_PRIME1_CONST int x) { mX = x; }
  int GetY() const { return mY; }
  void SetY(RSTL_PRIME1_CONST int y) { mY = y; }

  int& operator[](int idx) { return *(&mX + idx); }
  const int& operator[](int idx) const {
#if RSTL_PRIME1
    return reinterpret_cast< const int* >(this)[idx];
#else
    return *(&mX + idx);
#endif
  }

  static CVector2i Lerp(const CVector2i& a, const CVector2i& b, float t) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float inv = 1.f - t;
    int x = static_cast< int >(a.GetX() * inv + b.GetX() * t);
    int y = static_cast< int >(a.GetY() * inv + b.GetY() * t);
    return CVector2i(x, y);
#else
    const float inv = 1.f - t;
    return CVector2i(static_cast< int >(a.GetX() * inv + b.GetX() * t),
                     static_cast< int >(a.GetY() * inv + b.GetY() * t));
#endif
  }

private:
  int mX;
  int mY;
};

CVector2i operator+(const CVector2i& lhs, const CVector2i& rhs);
CVector2i operator-(const CVector2i& lhs, const CVector2i& rhs);
bool operator==(const CVector2i& lhs, const CVector2i& rhs);
bool operator!=(const CVector2i& lhs, const CVector2i& rhs);
CVector2i operator*(const CVector2i& lhs, int rhs);
CVector2i operator/(const CVector2i& lhs, int rhs);

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector2i)
} // namespace rstl
#endif

#endif // _CVECTOR2I
