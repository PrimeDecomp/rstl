#ifndef _CVECTOR2I
#define _CVECTOR2I

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/RstlVersions.h"
#include "types.h"
#include "rstl/construct.hpp"

class CVector2i {
public:
  CVector2i() : mX(0), mY(0) {}
#if RSTL_PRIME1
  CVector2i(int x, int y = 0);
  CVector2i(CInputStream& in) {
    mX = in.ReadLong();
    mY = in.ReadLong();
  }
#else
  CVector2i(int, int);
#endif

  int GetX() const { return mX; }
#if RSTL_PRIME1
  void SetX(const int x) { mX = x; }
#endif
  int GetY() const { return mY; }
#if RSTL_PRIME1
  void SetY(const int y) { mY = y; }
#else
  void SetX(int x) { mX = x; }
  void SetY(int y) { mY = y; }
#endif

  int& operator[](int idx) { return *(&mX + idx); }
#if RSTL_PRIME1
  const int& operator[](int idx) const { return reinterpret_cast< const int* >(this)[idx]; }
#else
  const int& operator[](int idx) const { return *(&mX + idx); }
#endif

#if RSTL_PRIME1
  static CVector2i Lerp(const CVector2i& a, const CVector2i& b, float v) {
    float inv = 1.f - v;
    int x = static_cast< int >(a.GetX() * inv + b.GetX() * v);
    int y = static_cast< int >(a.GetY() * inv + b.GetY() * v);
    return CVector2i(x, y);
  }
#else
  static CVector2i Lerp(const CVector2i& a, const CVector2i& b, float t) {
    const float inv = 1.f - t;
    return CVector2i(static_cast< int >(a.GetX() * inv + b.GetX() * t),
                     static_cast< int >(a.GetY() * inv + b.GetY() * t));
  }
#endif

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
