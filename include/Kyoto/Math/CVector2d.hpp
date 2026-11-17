#ifndef _CVECTOR2D
#define _CVECTOR2D

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "rstl/construct.hpp"

// Guessed name
class CVector2d {
public:
  CVector2d(double x, double y);

  double Magnitude() const;
  double MagSquared() const;

  double GetX() const { return mX; }
  double GetY() const { return mY; }

  // Guessed name
  static double Cross(const CVector2d& a, const CVector2d& b);

private:
  double mX;
  double mY;
};
CHECK_SIZEOF(CVector2d, 0x10)

CVector2d operator-(const CVector2d& lhs, const CVector2d& rhs);

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector2d)
}

#endif // _CVECTOR2D
