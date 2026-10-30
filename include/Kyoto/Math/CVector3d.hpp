#ifndef _CVECTOR3D
#define _CVECTOR3D
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CVector3f.hpp"

#if !RSTL_PRIME1
class CMotionState;
#endif

class CVector3d {
public:
  CVector3d(double x, double y, double z);
  CVector3d(const CVector3f& other);
  double Magnitude() const;
  double MagSquared() const;

  CVector3d AsNormalized() const;
  CVector3f AsCVector3f() const;
#if !RSTL_PRIME1
  static const CVector3d& Zero() { return sZeroVector; }
#endif

  double GetX() const { return mX; }
  double GetY() const { return mY; }
  double GetZ() const { return mZ; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  double& operator[](int i) { return (&mX)[i]; }
  const double& operator[](const int i) const {
    const double* data = &mX;
    return data[i];
  }
#else
  double& operator[](int index) { return (&mX)[index]; }
  const double& operator[](int index) const { return (&mX)[index]; }
#endif

  static double Dot(const CVector3d& a, const CVector3d& b);
  static CVector3d Cross(const CVector3d& a, const CVector3d& b);
#if RSTL_PRIME1
  bool IsNonZero() const { return GetX() != 0.0 || GetY() != 0.0 || GetZ() != 0.0; }
#endif

private:
  double mX;
  double mY;
  double mZ;
#if !RSTL_PRIME1
  static CVector3d sZeroVector;
  static CVector3d sUpVector;
  static CVector3d sDownVector;
  static CVector3d sLeftVector;
  static CVector3d sRightVector;
  static CVector3d sForwardVector;
  static CVector3d sBackVector;
#endif

};

CVector3d operator+(const CVector3d& lhs, const CVector3d& rhs);
CVector3d operator-(const CVector3d& lhs, const CVector3d& rhs);
#if !RSTL_PRIME1
CVector3d operator-(const CVector3d& other);
#endif

CVector3d operator*(double lhs, const CVector3d& rhs);

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector3d)
} // namespace rstl
#endif

#endif // _CVECTOR3D
