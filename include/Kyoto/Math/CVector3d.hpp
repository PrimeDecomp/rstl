#ifndef _CVECTOR3D
#define _CVECTOR3D
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CVector3f.hpp"

class CMotionState;

class CVector3d {
public:
  CVector3d(double x, double y, double z);
  CVector3d(const CVector3f& other);
  double Magnitude() const;
  double MagSquared() const;
  CVector3d& operator*=(double scale);
  CVector3d& operator+=(const CVector3d& other);

  CVector3d AsNormalized() const;
  CVector3f AsCVector3f() const;
#if !RSTL_PRIME1
  static const CVector3d& Zero() { return sZeroVector; }
#endif

  double GetX() const { return mX; }
  double GetY() const { return mY; }
  double GetZ() const { return mZ; }
  double& operator[](int i) { return (&mX)[i]; }
  const double& operator[](RSTL_PRIME1_CONST int i) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const double* data = &mX;
    return data[i];
#else
    return (&mX)[i];
#endif
  }

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
CVector3d operator-(const CVector3d& other);

CVector3d operator*(double lhs, const CVector3d& rhs);
// Declaring these changes overload resolution in Prime (CVector3f converts to CVector3d), so they
// stay guarded.
#if RSTL_KYOTO_VECTOR3D_COMPOUND
CVector3d operator*(const CVector3d& lhs, double rhs);
bool operator==(const CVector3d& lhs, const CVector3d& rhs);
#endif

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector3d)
} // namespace rstl
#endif

#endif // _CVECTOR3D
